# Doba — Documento de traspaso de contexto

Este documento describe el árbol de código actual. No es un historial de
auditorías ni una lista de cambios previstos.

## Proyecto y reglas de trabajo

Doba es un framework de servidor C++20 header-only. La frontera esencial es
genérica: el transporte no debe conocer tipos ni semántica de HTTP/1.1. El
protocolo comunica al transporte el resultado genérico de deserialización y la
intención del canal.

- Build: CMake 3.20 o superior, C++20 y presets `msvc-debug` / `msvc-release`.
- No realizar operaciones Git sobre este repositorio.
- Todo archivo C++ nuevo debe llevar la cabecera Apache/doba exacta de un
  archivo equivalente.
- `AGENTS.md` es la norma de trabajo vigente. Antes de modificar código,
  pruebas, configuración o documentación hay que presentar un plan concreto y
  esperar aprobación explícita.
- El árbol puede contener cambios locales del usuario. No sobrescribir ni
  revertir cambios ajenos.

### Codificación y formato del código fuente

Convención vigente en todo el repositorio, declarada en `.editorconfig` y
forzada en Git mediante `.gitattributes`:

- UTF-8 **sin** BOM.
- Finales de línea CRLF en el working tree.
- Newline final obligatorio.
- Contenido exclusivamente ASCII (0x00-0x7F).
- Límite de 80 columnas por línea.

Al no haber BOM, `CMakeLists.txt` pasa `/utf-8` a MSVC para fijar el juego de
caracteres de origen y de ejecución.

En los comentarios se usa `S` para las secciones RFC (por ejemplo
`RFC 9110 S7.6.1`) y `-` en lugar de raya. Cualquier sustitución dentro de un
comentario enmarcado debe conservar el ancho original para no desalinear la
`|` de cierre.

## Arquitectura actual

```text
include/
  common/
    byte_storage.h       almacenamiento temporal en memoria o fichero
    reader.h             lector move-only de byte_storage o span prestado
    writer.h             escritor move-only sobre byte_storage
    date_server.h        generación de Date
    hash_map.h           mapa hash usado por el dispatch y el router
  protocol/
    deserialization.h    deserialization_status, channel_intent y resultado
    serialization.h      serialization_result { prefix, source }
    http/
      common/            tipos y componentes reutilizables entre versiones
                         de HTTP (no especificos de HTTP/1.1)
        header.h         alias de header y header_view
        header_names.h   constantes de nombres de header
        helpers.h        utilidades de parsing compartidas (iequals,
                         percent_decode_validate / percent_decode_in_place,
                         etc.)
        method.h         tipo de metodo HTTP
        method_names.h   constantes de nombres de metodo
        query_parameter.h
                         tipo de parametro de query
        request_getter.h callback generico que termina de construir una
                         request a partir de un byte_storage opcional
        router.h         seleccion de rutas estaticas, parametrizadas y
                         wildcard (agnostico de la version HTTP)
        router_types.h   resultados de match y conversion de parametros
        router_handler_static.h
                         contrato del handler de ruta estatica
        router_handler_parametrized.h
                         handler tipado para parametros de path
        router_handler_parametrized_base.h
                         interfaz interna de handlers parametrizados
        status_codes.h   constantes de status code
        target.h         tipo de request-target
        headers/         checkers e interpretes de headers compartidos
                         entre versiones de HTTP (Accept, Cache-Control,
                         Content-Type, Cookie, ETag, Range, Sec-WebSocket-*,
                         Vary, WWW-Authenticate, etc.)
      v11/               todo lo estrictamente especifico de HTTP/1.1
        decoder.h        decoder incremental de peticiones HTTP/1.1
        request.h        objeto de request y materializacion diferida
        response.h       respuesta de buffer fijo
        server.h         composicion HTTP/1.1 de router y transporte
        connection.h     estado hop-by-hop derivado de la request
        context.h        contexto de decodificacion (connection, policies,
                         rejection_reason)
        limits.h         limites operacionales centralizados (buffers,
                         tamanos maximos, etc.)
        parsed_types.h   tipos intermedios de parsing (tokens, parametros)
        policies.h       politicas de decodificacion derivadas de limits
        rejection_reason.h
                         motivo de rechazo neutro que atraviesa el decoder
        reason_phrases.h reason-phrases de status line
        status_lines.h   status-lines completas (codigo + reason-phrase)
        verdict.h        veredicto binario de check/interpret
        body/
          (entrada: framing del body recibido)
          framer_raw.h     framing de un body delimitado por Content-Length
          framer_chunked.h validacion del wire chunked recibido
          framer_state.h   resultado de los body framers
          framer_error.h   errores de framing de entrada
          (entrada: lectura del body ya acumulado)
          reader.h         body::reader: selecciona reader_chunked/reader_raw
                           segun el encoding real de la request
          reader_chunked.h decodificacion del framing chunked al leer
          reader_raw.h     lectura directa de un body Content-Length
          reader_state.h   resultado de los body readers
          reader_error.h   errores de lectura de body
          (salida: body de la respuesta)
          writer.h         body::body_writer: fachada raw/chunked de salida
          writer_raw.h     codificacion de salida con Content-Length
          writer_chunked.h codificacion del wire chunked de salida
          writer_state.h   resultado de los body writers
          writer_error.h   errores de escritura de salida
        headers/           checkers e interpretes de headers y reglas
                           especificos de HTTP/1.1 (Host, Content-Length,
                           Transfer-Encoding, Connection, TE, Trailer,
                           Expect, Upgrade, Via, Forwarded, X-Forwarded-*)
          rules/           reglas transversales aplicadas tras el pase de
                           headers: directives, framing, policy, routing
  transport/server/
    tcpip.h              selector de plataforma
    tcpip_windows.h      backend IOCP
    tcpip_linux.h        backend epoll
```

## Contrato protocolo ↔ transporte

`protocol/deserialization.h` define:

- `deserialization_status`: `kSucceeded`, `kInvalidSource` y
  `kMoreBytesNeeded`.
- `channel_intent`: `kKeep`, `kClose` y `kUpgrade`.
- `deserialization_result<RQty>`, que entrega una `std::shared_ptr<RQty>` y la
  intención de ciclo de vida del canal. Incluye además `reason`, un `int`
  opaco con el motivo de rechazo, e `interim`, un `std::string_view` con
  bytes que el protocolo necesita en el cable antes de poder continuar. El
  transporte los escribe sin interpretarlos, reservándoles su propio
  `response_id`, y su almacenamiento pertenece al protocolo (hoy lo usa
  HTTP/1.1 para el `100 Continue`).

El transporte debe basarse únicamente en esos tipos genéricos. HTTP decide si
una conexión se mantiene o se cierra; el transporte no debe inspeccionar
headers, métodos ni status-lines.

`protocol/serialization.h` define `serialization_result`:

```cpp
struct serialization_result {
  std::string prefix;
  std::optional<common::reader> source;
};
```

`prefix` posee los bytes ya materializados. `source` permite que un protocolo
entregue una fuente genérica para el resto de bytes. La `response` HTTP rellena
`prefix` con la status-line, los headers y el cuerpo inline cuando lo hay, y
emplaza `source` con el `common::reader` liberado por su `body::body_writer`
cuando el cuerpo se entregó como writer. Ambos transportes consumen primero
`prefix` y después drenan `source`.

## HTTP/1.1: decodificación de requests

El punto de entrada actual es `decoder<RQty, RSty>` en
`protocol/http/v11/decoder.h`:

1. `accumulate(char*, size)` copia en su buffer interno hasta agotar
   `kDecodingBufferSize` (16384 bytes, alias privado del decoder que reutiliza
   `limits::kDecodingBufferSize`) y devuelve cuántos bytes admitió.
2. `deserialize()` llama a `parse_core()` mientras no haya body pendiente o a
   `parse_body()` cuando ya se ha elegido un body framer.
3. `parse_core()` procesa request-line y headers una vez. Usa
   `header_dispatchers_`, un `common::hash_map` con punteros a función.
4. Los headers modelados actualizan `context_`; al terminar los headers se
   aplican `framing`, `routing`, `directives` y `policy`.
5. Si hay body, el decoder abre un `common::writer` con `spill_threshold` de
   65535 bytes y selecciona `body::framer_chunked` cuando la conexión indica
   chunked, o `body::framer_raw(content_length)` en caso contrario.
6. Al completarse la request, `dispatch()` construye el
   `deserialization_result` y reinicia el estado del decoder.

Los headers y reglas viven bajo `protocol/http/v11/headers/`. El mapa de
dispatch actual contiene los headers comprobados y los dispatchers dedicados
para Host, Content-Length, Transfer-Encoding, Connection, TE, Trailer,
Expect, Upgrade, Max-Forwards, Via, Forwarded y los tres X-Forwarded.

### Body de entrada

Las clases de entrada son los `framer`, no los `writer` (estos últimos son de
salida). `body::framer_raw` delimita exactamente los bytes indicados por
Content-Length. `body::framer_chunked` valida el framing chunked y conserva
todos los bytes wire, incluidos tamaño de chunk, extensiones, trailers y
terminador. Ninguno decodifica el payload a una forma distinta: el decoder
vuelca lo aceptado a un `common::writer`.

`common::writer` entrega el resultado como `common::byte_storage`.
`request_getter<RQty>` es el callback que recibe opcionalmente ese storage y
termina de devolver una `std::shared_ptr<RQty>`. La implementación actual de
`request::from` recibe además si la request usa chunked encoding y su
Content-Length, crea un `common::reader` propietario del `byte_storage` y lo
envuelve en un `body::reader` (ver `protocol/http/v11/body/reader.h`), que
selecciona internamente `body::reader_chunked` o `body::reader_raw` según el
encoding real usado por la request. `request::get_body_reader()` expone ese
`body::reader` ya construido cuando hay body, de forma que el llamador no
necesita saber qué framing se usó para leer el payload decodificado.

`common::byte_storage` empieza en memoria y puede derramar a fichero cuando
`spill_threshold` es mayor que cero. `common::reader` es move-only y permite
leer el storage, obtener bytes individuales y vaciarlo a un `std::string` con
`read_all`. También ofrece `reader::borrowed(span)`: esa variante no posee los
bytes y exige que el almacenamiento del llamador sobreviva al reader y a todos
los objetos a los que se mueva.

### Body de salida

`body::body_writer` (`body/writer.h`) es la fachada del cuerpo de respuesta.
Se construye con `body_writer::raw(opts)` o `body_writer::chunked(opts)` y
mantiene internamente un `std::variant<writer_chunked, writer_raw>` sobre un
`common::writer`, por lo que también puede derramar a fichero.

`response::set_body(std::string_view)` copia el payload a la zona inline
cuando cabe en `kMaxBodySizeInMemory` y fija `Content-Length`; si no cabe,
crea internamente un writer raw y delega en él.
`response::set_body(body::body_writer&&)` adopta un writer ya construido por
el llamador. En ambos casos `apply_body_framing()` emite
`Transfer-Encoding: chunked` para writers chunked o un `Content-Length`
derivado de `bytes_written()` para los raw. `serialize()` consume el writer y
lo entrega como `serialization_result::source`.

## Objetos HTTP públicos

### `request`

`request` no es copiable ni movible. Su construcción pública se realiza a
través de `request::from(...)`, que devuelve `request_getter<request>`.

Expone método, forma del target, path, headers, parámetros de query, Host y
autoridad del request-target, además de `get_body_reader()`.

Los campos de metadatos de la implementación actual son principalmente
`std::string_view` y `header_view`; no documentar esos getters como copias
owned. Cualquier cambio en su propiedad o lifetime requiere una revisión
explícita de la interacción entre `decoder`, `request_getter` y el buffer del
decoder.

### `response`

`response` no es copiable ni movible y usa `char memory_[4096]`:

- status-line y headers ocupan la parte inicial;
- el body inline se reserva al final;
- `set_body(std::string_view)` copia el body a esa zona;
- `serialize()` devuelve `protocol::serialization_result` con el contenido
  inline en `prefix`.

La API incluye `add_header`, `set_header`, `has_header`, `get_header` por clave
o índice, `get_headers_length`, `remove_header` y los helpers de status-line.
Para el cuerpo existen las dos sobrecargas de `set_body` descritas en "Body de
salida": una inline y otra que adopta un `body::body_writer`.

### `server` y `router`

`server` está parametrizado por request, response, decoder, transporte y
router:

```cpp
server<RQty, RSty, DEty, TRty, ROty>
```

Sus valores por defecto son `request`, `response`, `decoder`,
`transport::server::tcpip` y `router`. `server` almacena
`ROty<RQty, RSty>` como dependencia interna: `add_route` pertenece al servidor
y `start` recibe el puerto.
Origin-form y absolute-form se enrutan; authority-form responde 501 y
asterisk-form responde 200.

El router registra rutas estáticas, parametrizadas y wildcard mediante
`add_route(method, path, lambda, policy)`, donde `policy` es opcional y por
defecto `execution_policy::kSynchronous`. El handler recibe
`std::shared_ptr<const RQty>` y `std::shared_ptr<RSty>` como sus dos primeros
argumentos. El patrón parametrizado usa segmentos `:nombre`, por ejemplo
`/users/:id`, y sus parámetros se declaran a continuación en la lambda. Se
convierten en orden y soportan `std::string`, `bool`, tipos integrales y tipos
de punto flotante.

`match` recibe también `on_send`, evalúa primero las rutas estáticas y después
las parametrizadas, y da prioridad a una coincidencia estática. Una ruta
síncrona ejecuta su handler y llama a `on_send` dentro del router. Una ruta
asíncrona se encola en el pool interno del router y llama a `on_send` al
completar. Devuelve `kMatched`, `kNotFound` o `kMethodNotAllowed`; `server`
traduce los dos últimos a 404 y 405, respectivamente. Al detenerse, `server`
drena el router antes de detener el transporte.

Los paths son sensibles a mayúsculas. Una ruta parametrizada con barra final
solo coincide con un path que también la tenga. Un patrón que termina en `/*`
coincide con cualquier path que empiece por su prefijo, incluida la porción
vacía: `/assets/*` coincide con `/assets/` y con `/assets/a/b`, pero no con
`/assets`. La ruta wildcard no entrega esa porción como argumento: el handler
puede consultarla en la request. La prioridad es estática, parametrizada y
wildcard. Cuando se devuelve 405, el router añade `Allow` con los métodos
aplicables, incluidos los wildcard. Una wildcard contiene un único `*` como
segmento final y su handler no admite parámetros tipados; cualquier otra
combinación con `*` se rechaza durante `add_route`.

## Transporte

Ambos backends estan implementados y operativos. `transport/server/tcpip.h`
selecciona `tcpip_windows.h` (IOCP) o `tcpip_linux.h` (EPOLL) en tiempo de
compilacion segun la plataforma; no hay backend de referencia ni fallback
sincrono. Los dos exponen exactamente la misma API publica al protocolo
(`start`/`stop`, callbacks de request, bad-request y ciclo de vida de canal) y
comparten el mismo modelo de ordenacion de respuestas pipelined mediante
identificadores monotonos.

Tras la unificacion de ambos transportes, la parte de gestion de respuestas es
estructuralmente identica en los dos backends:

- El estado por respuesta vive en un `response_data` con el mismo contenido
  (`response`, `prefix_written`, etc.).
- Las respuestas pendientes se guardan en un `std::deque<response_data>`
  (`responses_`) indexado por la distancia respecto a `expected_response_id_`,
  de modo que reservar es un `push_back` amortizado O(1) y completar es un
  acceso por indice O(1).
- Los limites de envio son las mismas constantes en ambos ficheros:
  `kSendBufferMaxSz` (65536) y `kSendChunkSz` (8192).
- Ambos ofrecen el mismo conjunto de operaciones sobre el contexto:
  `get_next_response_id`, encolado de respuesta normal, `enqueue_error_response`
  (usado tanto por errores de decodificacion como por excepciones de handler),
  `fail_response`, `close()` y `abort()`.

Ambos backends consumen `serialization_result` de la misma forma: primero
vuelcan `prefix` una sola vez (marca `prefix_written`) y despues drenan
`source` en trozos acotados por `kSendChunkSz`, deteniendose cuando el buffer
de envio alcanza `kSendBufferMaxSz`. Un `failed()` del reader cierra el
contexto; una lectura de cero bytes retira la respuesta y avanza al siguiente
identificador esperado.

La diferencia entre backends es unicamente el mecanismo de I/O y de propiedad
del contexto que impone cada plataforma:

En Windows, cada request recibe un identificador de respuesta monotono. El
`on_send` entregado al protocolo conserva el contexto mediante `shared_ptr`,
encola la respuesta serializada con ese identificador y, cuando la completacion
llega desde otro hilo del pool de IOCP, arma el siguiente envio. El contexto
solo envia la siguiente respuesta esperada, por lo que las respuestas de
handlers asincronos se entregan en el orden de las requests pipelined. El
estado compartido del contexto (cola de respuestas, buffer de salida y flags de
cierre) se protege con `sending_mutex_`, ya que las completaciones pueden
llegar en cualquier worker.

En Linux, cada worker posee su instancia EPOLL, su listener configurado con
`SO_REUSEPORT` y todos los contextos aceptados por el. El camino sincrono
deserializa, ejecuta el handler y envia en ese mismo worker. Una respuesta
tardia conserva el contexto, se encola en su worker mediante `eventfd` y ese
worker realiza el envio y cualquier operacion EPOLL. El camino sincrono no
toca la cola ni el mutex de notificaciones asincronas. EPOLL entrega
directamente el puntero al contexto, sin lookup por evento; el registro de
contextos solo se consulta al aceptar o retirar una conexion.

El worker conserva cada contexto mediante `shared_ptr` y `on_send` mantiene
esa misma propiedad mientras puede existir una respuesta tardia. Al cerrar un
socket, el contexto se enlaza en una lista de retirada del propio worker y no
se elimina del registro hasta terminar el lote devuelto por `epoll_wait`; asi
ningun evento ya entregado puede observar memoria liberada y no se necesita
una asignacion para diferir la destruccion. Una respuesta tardia tras `stop()`
encuentra el contexto cerrado y el worker deja de aceptar notificaciones.

Una posicion vacia en la cola bloquea unicamente las respuestas posteriores;
una respuesta duplicada, nula o tardia no puede completar de nuevo la misma
posicion.

Se distinguen dos formas de terminar, identicas en ambos backends:

- Cierre ordenado (`close`): se deja de leer, se drena todo lo que sea seguro
  enviar y el contexto se destruye cuando no quedan respuestas. Lo usan EOF,
  `channel_intent::kClose`, una respuesta nula y un fallo de serializacion.
- Aborto fatal (`abort`): el flujo de salida ya esta corrupto (fallo del
  socket, decoder o source), asi que no se envia nada mas y el contexto se
  retira de inmediato.

Un fallo sincrono del handler reutiliza el identificador ya reservado para
enviar el error, en lugar de reservar uno nuevo. `tcpip::stop()` debe
invocarse desde fuera de un worker del transporte. Los callbacks del
transporte no pueden cambiar mientras `start()` o `stop()` estan activos.

Lo que sigue pendiente en el transporte no es el backend en si, sino politicas
transversales aun no implementadas en ninguno de los dos: timeouts (item 1) y
limites de conexion efectivos (item 15).

No modificar la frontera protocolo/transporte para resolver una necesidad
exclusiva de HTTP. Si un cambio requiere semantica HTTP, debe vivir en la capa
HTTP o expresarse en el contrato generico ya existente.

## Pendientes de compliance HTTP/1.1

Estado verificado por lectura del árbol, no por ejecución contra clientes
reales. Complejidad: B = baja (localizada), M = media (varios archivos o API
interna), A = alta (capa nueva, dependencia externa o cambio de contrato).

Los items pendientes conservan identificadores estables `C1`-`C6`; los
niveles indican su criticidad. Los ya resueltos o descartados pierden su
numero y conservan una referencia `(antes item N)` para no romper trazas
anteriores.

### Nivel 1 — Crítico: incumplimiento visible en cada respuesta

C1. **Ausencia total de timeouts. (A - diferido al final, ver Secuencia
    recomendada)** Sin deadline de cabeceras, body, escritura ni idle de
    keep-alive en ninguno de los dos backends. Exposición directa a
    slowloris y prerequisito para emitir `408`, cuyo status-line ya existe
    pero nunca se usa.

**[RESUELTO] (antes ítem 2)** Todo rechazo colapsaba en `400`.
`rejection_reason` (`protocol/http/v11/rejection_reason.h`) sustituye el
`verdict` binario como
canal neutro de motivo de rechazo: cada `interpret()` de `headers/` y las
cuatro reglas de `rules/` pueden registrar en `context_.rejection_reason`
por qué rechazan, sin que `decoder.h` ni el transporte conozcan semántica
HTTP (el motivo viaja como `int` opaco en
`deserialization_result::reason`). `v11::server::set_on_bad_request`
traduce ese motivo a `400`, `413`, `414`, `431`, `501` o `505` según
corresponda. `414` y `431` se apoyan en dos límites operacionales nuevos,
`policies::max_uri_length` y `policies::max_header_section_size`
(ambos deshabilitados por defecto, valor `0`), cuyos valores sugeridos
viven en el repositorio centralizado `protocol/http/v11/limits.h` junto con
el resto de límites operacionales del árbol (`kDecodingBufferSize`,
`kMaxRequestHeadSize`, `kMaxResponseSizeInMemory`,
`kMaxResponseBodySizeInMemory`, `kMaxChunkedExtensionSize`,
`kMaxChunkedTrailerSize`, `kMaxQueryParameters`, etc.). Los alias locales
redundantes que algunas clases (`decoder.h`, `request.h`,
`body/framer_chunked.h`, `body/reader_chunked.h`, `response.h`)
declaraban simplemente para renombrar una constante de `limits.h` se
eliminaron; todos los puntos de uso referencian `limits::` directamente,
evitando una capa de indirección sin valor añadido. Build verificado con
`run_build`: exitoso.

### Nivel 2 — Alto: bloquea despliegue real o usabilidad básica

C6. **Trailers de entrada sin validacion completa. (M)**
    `body/framer_chunked.h` y `body/reader_chunked.h` delimitan la seccion de
    trailers y aplican su limite de tamano, pero no validan la sintaxis de cada
    `field-line`. Un trailer malformado puede atravesar el framing como si fuera
    valido; `framer_error::invalid_trailer` y `reader_error::invalid_trailer`
    existen, pero esa validacion aun no esta implementada.

**[RESUELTO] (antes ítem C2)** `100-continue` sin implementar. El header
`Expect` se interpretaba, pero no se emitía `100 Continue` ni `417`.
Resuelto en dos partes independientes:

- **`417 Expectation Failed`.** Se añadió
  `rejection_reason::kExpectationFailed` (al final del enum, para no alterar
  los valores numéricos que el transporte transporta en crudo) y
  `dispatch_expect` lo registra cuando `expect::interpret` rechaza una
  expectativa desconocida, siguiendo el patrón ya existente de
  `dispatch_upgrade`. `server.h` lo traduce a `417` reutilizando el helper
  `expectation_failed_417()`, que ya existía. Un fallo de `expect::check`
  sigue produciendo `400`, porque eso es sintaxis malformada.
- **`100 Continue`.** `v11::connection` gana el flag `expects_continue`, que
  activa `expect::interpret`. El obstáculo estructural citado antes
  (`on_send` es de un solo uso) se resolvió sin tocar `enqueue_response_`:
  el interim no comparte slot con la respuesta final, sino que pide **su
  propio `response_id`**, anterior al de la petición. El orden de salida ya
  lo garantiza el recorrido en orden estricto de `responses_`.

El contrato genérico gana un único campo, `deserialization_result::interim`,
un `std::string_view` de bytes opacos que el transporte escribe antes de
continuar; apunta a una constante `constexpr` del decoder, así que no hay
asignaciones ni gestión de ciclo de vida. El decoder lo rellena al crear el
body framer, rama que se ejecuta una sola vez por petición. Nótese que
`status_lines::k100` no sirve directamente: la macro `SL()` emite un solo
CRLF y un interim necesita además la línea vacía que cierra la sección de
headers. Ambos backends lo encolan en la rama `kMoreBytesNeeded` (unas cinco
líneas cada uno), sin delegates ni tipos nuevos. Se descartó por innecesario
un interruptor en `policies` y la optimización RFC de omitir el `100` cuando
el handler ya iría a responder `4xx`. Build verificado con `run_build`:
exitoso.

`interim` solo se emite junto a `kMoreBytesNeeded`, y así está documentado en
`deserialization.h`: son bytes para desbloquear a un emisor al que aún se le
deben datos, de modo que un mensaje ya completo nunca lleva uno. El decoder
omite deliberadamente el `100` si el cuerpo entero llegó en el mismo segmento
(cliente optimista que envía `Expect` y el cuerpo a la vez): la RFC lo
permite, y emitirlo entonces lo colocaría detrás de los datos que debía
preceder. Sin esa comprobación el interim se habría perdido en silencio,
porque los transportes solo lo leen en esa rama.

**Verificación pendiente:** el comportamiento sobre el cable no se ha
comprobado contra clientes reales (ver ítem P5, suite de pruebas). Queda sin
verificar el orden de salida del interim respecto a la respuesta final, el
caso de pipelining -donde el interim consume un `response_id` intermedio- y
todo el backend Linux, que no compila en el entorno MSVC de este árbol. Un
`curl -v -H "Expect: 100-continue" --data-binary @archivo` cubriría lo
primero.

**[RESUELTO] (antes ítem 5)** Una excepción del handler cerraba la conexión en
silencio. Se añadió `rejection_reason::kHandlerError`
(mapea a 500 Internal Server Error en `server.h`) y ambos backends
(`tcpip_windows.h`, `tcpip_linux.h`) ahora reutilizan el canal existente
`enqueue_error_response`/`on_bad_request_` en el `catch` que envuelve la
llamada a `on_request_`, en lugar de cerrar la conexión silenciosamente.
No se añadió ningún delegate/miembro/setter nuevo: se reutilizó la
infraestructura ya existente para errores de decodificación. Build
verificado con `run_build`: exitoso.

**[RESUELTO] (antes ítem 6)** `request` no daba acceso de conveniencia a
headers, query y cookies. `get_header(std::string_view)` y
`exist_header(...)` ahora comparan de forma case-insensitive (RFC 9110
S5.1) usando `helpers::iequals`, corrigiendo un bug de compliance además
de la carencia de conveniencia. Se añadieron `get_query_parameter(name)`
(`std::optional<query_parameter_view>`), `get_cookie(name)`
(`std::optional<std::string_view>`, parseo perezoso bajo demanda del
header `Cookie` crudo) y `get_cookies()` (todos los pares). No se
modificó el parseo/validación existente de `headers/cookie.h` ni la
representación interna de `request`. Build verificado con `run_build`:
exitoso.

Durante el análisis se detectaron dos conveniencias de API adicionales,
diferidas para un futuro punto (fuera de alcance de este ítem):

- `request` no expone el body ya leído como string/bytes de conveniencia
  (solo `get_body_reader()` de bajo nivel).
- No hay getters tipados para headers ya interpretados como `Content-Type`
  o `Accept` (existen los parsers en `headers/`, falta exponerlos en
  `request`).

**[RESUELTO] (antes ítem 7)** Sin percent-decoding. (B)
El path del request-target (`origin-form`/`absolute-form`) se decodifica
ahora sin romper la inmutabilidad del buffer del decoder, en dos fases:

- `decoder.h`, inmediatamente después de validar `max_uri_length` y antes
  de montar la request, llama a `helpers::percent_decode_validate`, una
  comprobación **de solo lectura** sobre `absolute_path_`. El decoder no
  escribe nunca en su propio buffer (`buffer_`), manteniendo la regla de
  que el decoder solo observa la fuente que recibe.
- `request.h`, en su constructor, ya realiza un `memcpy` del head a un
  buffer que la propia `request` posee. Sobre esa copia (y solo sobre
  ella) se aplica `helpers::percent_decode_in_place`, un algoritmo de dos
  punteros (lectura/escritura) *in-place* sobre el sub-rango de
  `abs_path_`: como un triplete `%HH` decodificado nunca es más largo que
  su forma codificada, el resultado siempre cabe en el mismo rango sin
  buffer adicional ni segunda pasada - coste O(n), cero allocations y con
  un fast-path que retorna de inmediato si el path no contiene ningún `%`
  (el caso dominante).

El reparto evita duplicar trabajo: `percent_decode_in_place` no vuelve a
comprobar bytes NUL, porque `percent_decode_validate` ya garantizó aguas
arriba que ningún triplete decodifica a `0x00`; por eso devuelve `void` y
documenta explícitamente esa precondición. Un `%00` decodificado se
rechaza en el decoder como sintácticamente inválido
(`rejection_reason::kSyntax`, mapeado a 400), al ser un vector clásico de
inyección/bypass. `request::get_absolute_path()` expone directamente el
path ya decodificado sin cambios de API; `router.h` no requirió cambios,
ya que consume ese mismo getter.

Nota para un futuro punto (p. ej. el handler de ficheros estáticos, ítem
P1): un `%2F` decodificado se convierte en un `/` literal indistinguible
de un separador de segmento real. Cualquier lógica que vuelva a segmentar
el path por `/` después de este punto debe tener esto en cuenta (p. ej.
para mitigar path-traversal), ya que la decodificación ocurre una única
vez, antes del enrutado, sobre el path completo.

**[RESUELTO] (antes ítem 8)** Solo se acepta `HTTP/1.1`. (B) Cerrado por
decisión de alcance.
Doba es un framework HTTP/1.1-only por diseño (ver cabecera de este
documento y comentarios de `decoder.h`/`server.h`); no se implementará
semántica `HTTP/1.0` (framing sin chunked, cierre implícito por defecto,
etc.), ya que eso ampliaría el alcance del proyecto en lugar de corregir
un incumplimiento de compliance HTTP/1.1. El comportamiento actual ya es
conforme: una versión bien formada pero distinta de `1.1` que sea
numéricamente inferior (p. ej. `HTTP/1.0`, `HTTP/0.9`) devuelve `400`;
una versión numéricamente superior (p. ej. `HTTP/2.0`) devuelve `505`
(cubierto por el antiguo ítem 2, ya resuelto). No se requiere ningún
cambio de código para este punto.

### Nivel 3 - Medio: paridad funcional esperada

C2. **Condicionales sin evaluar. (M/A)** Los headers condicionales
    están modelados y validados sintácticamente, pero nadie los evalúa: no se
    genera `304`, `412`, `206` ni `416`. Requiere decidir cómo expone el
    handler sus validadores.

C3. **Trailers de salida. (M)**
    `response` no tiene API para emitirlos.

C4. **`OPTIONS` sin respuesta automática. (B)** No se emite
    `Allow`. El router ya sabe calcular los métodos aplicables (lo hace para el
    `405`); falta exponerlo.

### Nivel 4 - Operabilidad y confianza

C5. **Sin límites de conexión efectivos. (B/M)**
    `connections_` es solo un contador observacional: nada lo consulta para
    dejar de aceptar.

## Backlog de producto / conveniencia — no es compliance HTTP/1.1

Estos ítems no corrigen un incumplimiento de RFC 9110/9112: son
funcionalidad de conveniencia que un framework web suele ofrecer, pero que
un usuario de Doba puede implementar por su cuenta con la API ya existente
sin que el servidor deje de ser conforme. Se numeran de forma independiente
para no mezclarlos con el backlog de compliance.

P1. **Handler de ficheros estáticos (antes ítem 12). (M)** El streaming de
    salida ya funciona, así que la base está. Depende del antiguo ítem 7
    (percent-decoding, ya resuelto) por seguridad e idealmente de
    `TransmitFile`/`sendfile`.

P2. **Logging de acceso (antes ítem 16).** No hay ningún punto de
    extensión para observabilidad.

P3. **Cadena de middleware (antes ítem 17).** No hay forma de componer lógica
    transversal sin duplicarla en cada handler. Requiere diseño cuidadoso para
    no contradecir el principio de "sin maquinaria de framework".

P4. **Parsing de formularios (antes ítem 18). (M)** Ni
    `x-www-form-urlencoded` ni `multipart/form-data`. Comparte primitiva con
    el percent-decoding del antiguo ítem 7.

P5. **Suite de conformidad (antes ítem 19).** No existe una batería propia
    de casos de protocolo (framing, smuggling, pipelining, límites); cada
    cambio en el decoder es una apuesta. Es infraestructura de QA, no un
    incumplimiento de compliance en sí.

## Fuera del alcance de la primera release

Decisiones de producto explícitas. Estos puntos **no se numeran** junto al
resto de ítems (ni de compliance ni de producto) porque no forman parte de
ningún lote planificado para la primera release de Doba. Se conservan aquí
como pendientes futuros, para que no se pierda el contexto ya analizado.

- **TLS (antes ítem 3). (A)** El despliegue previsto para la primera release
  es detrás de un terminador TLS (reverse proxy). Cuando se aborde, debe
  encajar sin romper la frontera protocolo/transporte.

- **Compresión / GZIP (antes ítem 10, luego P1).** Negociación de contenido
  opcional (`Accept-Encoding`/`Content-Encoding`) y gestión de `Vary`.
  Introduce dependencia externa, lo que choca con el "cero dependencias" del
  README.

- **WebSockets / manejo de `channel_intent::kUpgrade` (antes ítem 13). (A)**
  `channel_intent::kUpgrade` está definido y los headers `Sec-WebSocket-*`
  están modelados, pero ningún transporte lo maneja. Abordarlo exige definir
  quién posee el socket tras el `101`, cómo se drena el buffer ya acumulado y
  cómo se desactiva el pipelining, todo ello sin filtrar semántica HTTP al
  transporte. No es un incumplimiento de RFC 9110/9112: un servidor conforme
  puede rechazar toda petición de upgrade. Lo ya modelado se conserva tal
  cual, sin coste para la primera release.

### Documentación pendiente

- Documentar el contrato de ciclo de vida para usuarios que consuman
  directamente el transporte, fuera de `v11::server`.

### Secuencia recomendada

Las "tandas" son lotes de ejecución, no un ranking de criticidad: agrupan
ítems que conviene abordar juntos porque comparten dependencias o naturaleza.
Los números son los identificadores de los ítems de este documento.

| Tanda | Criterio de agrupación                                     | Ítems         | Estado     |
|-------|------------------------------------------------------------|---------------|------------|
| 1     | Complejidad B, alto retorno                                 | (5, 6, 8)     | Completada |
| 2     | Correctitud de framing                                      | (2)           | Completada |
| 3     | Robustez, excluyendo timeouts                               | C5, C6        | Pendiente  |
| 4     | Despliegue autónomo                                         | (C2)          | Completada |
| 5     | Paridad funcional de compliance (dependía de 7, ya resuelto)| C2, C3, C4    | Pendiente  |
| 6     | Complejidad A, diferidos al final por decisión explícita    | C1            | Pendiente  |
| 7     | Backlog de producto/conveniencia, sin dependencias bloqueantes | P1, P2, P3, P4, P5 | Pendiente |

**Lista completa de compliance pendiente**, en el orden de ejecución que
resulta de la tabla anterior:

1. Item C6 - Validacion de trailers de entrada (Nivel 2).
2. Ítem C5 — Sin límites de conexión efectivos (Nivel 4).
3. Ítem C2 — Evaluación de condicionales: `304`, `412`, `206`, `416`
   (Nivel 3).
4. Ítem C3 — Trailers de salida (Nivel 3).
5. Ítem C4 — `OPTIONS` / `Allow` (Nivel 3).
6. Ítem C1 — Ausencia total de timeouts (Nivel 1).

Ordenados por criticidad estricta (niveles de este documento) el orden sería
`C1` → `C6` → `C2, C3, C4` → `C5`. La discrepancia con la tabla es
intencionada: el ítem C1 es el más crítico, pero se difiere al final por su
complejidad A (toca ambos transportes y el ciclo de vida del canal).

TLS, compresión/GZIP y WebSockets no aparecen en ninguna tanda ni en la
numeración de ítems: ver "Fuera del alcance de la primera release".

## Estado de pruebas y documentación

El único subproyecto de prueba configurado es `examples/001-hello_world`, que
es un harness/ejemplo de servidor; no debe presentarse como una suite unitaria
exhaustiva sin revisar sus casos y aserciones concretas. No existe una batería
de conformidad de protocolo.

Los resultados de builds, auditorías y correcciones históricas no se incluyen
aquí: deben verificarse de nuevo contra el árbol y la toolchain disponibles
antes de afirmarlos.

## Al continuar

1. Leer `AGENTS.md`, este documento y los archivos directamente relacionados
   con la tarea.
2. Antes de editar, presentar un plan con causa, archivos, cambio, pruebas y
   riesgos; esperar aprobación explícita.
3. Preservar la API pública, el modelo de ownership y el estilo del módulo
   afectado salvo que el encargo aprobado requiera cambiarlos.
4. Mantener los cambios estrictamente locales. No añadir abstracciones,
   sobrecargas públicas, configuración, dependencias ni cambios de CMake sin
   que formen parte del plan aprobado.
5. Tras un cambio aprobado, comunicar exactamente qué se verificó y qué queda
   sin verificar.
