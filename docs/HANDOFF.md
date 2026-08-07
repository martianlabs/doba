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
                         percent_decode_in_place, etc.)
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
  intención de ciclo de vida del canal.

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

`transport/server/tcpip.h` selecciona `tcpip_windows.h` o `tcpip_linux.h` según
la plataforma. El backend Windows usa IOCP y el Linux usa EPOLL. Ambos
mantienen el orden de respuestas pipelined mediante identificadores de
respuesta monótonos.

Ambos backends consumen `serialization_result` de la misma forma: primero
vuelcan `prefix` una sola vez (marca `prefix_written`) y después drenan
`source` en trozos acotados por `kSendChunkSz`, deteniéndose cuando el buffer
de envío alcanza `kSendBufferMaxSz`. Un `failed()` del reader cierra el
contexto; una lectura de cero bytes retira la respuesta y avanza al siguiente
identificador esperado.

En Windows, cada request recibe un identificador de respuesta monótono. El
`on_send` entregado al protocolo conserva el contexto mediante `shared_ptr`,
encola la respuesta serializada con ese identificador y, cuando la completación
llega desde otro hilo, arma el siguiente envío. El contexto solo envía la
siguiente respuesta esperada, por lo que las respuestas de handlers asíncronos
se entregan en el orden de las requests pipelined.

En Linux, cada worker posee su instancia EPOLL, su listener configurado con
`SO_REUSEPORT` y todos los contextos aceptados por él. El camino síncrono
deserializa, ejecuta el handler y envía en ese mismo worker. Una respuesta
tardía conserva el contexto, se encola en su worker mediante `eventfd` y ese
worker realiza el envío y cualquier operación EPOLL. No hay mutex ni lookup
global en el despacho de eventos. Los contextos solo se destruyen desde su
worker propietario; las inscripciones retiradas se conservan hasta completar
el lote EPOLL que las referencia. Una respuesta tardía tras `stop()` descarta
la notificación si el worker ya no acepta notificaciones.

Cada `on_send` es de un solo uso: la primera llamada completa la respuesta y
las posteriores se descartan. Al recibir EOF, Linux deja de leer y drena las
respuestas ya completadas y las que estén pendientes; un fallo de handler o de
serialización cierra el contexto. `tcpip::stop()` debe invocarse desde fuera de
un worker del transporte.

No modificar la frontera protocolo/transporte para resolver una necesidad
exclusiva de HTTP. Si un cambio requiere semántica HTTP, debe vivir en la capa
HTTP o expresarse en el contrato genérico ya existente.

## Pendientes de compliance HTTP/1.1

Estado verificado por lectura del árbol, no por ejecución contra clientes
reales. Complejidad: B = baja (localizada), M = media (varios archivos o API
interna), A = alta (capa nueva, dependencia externa o cambio de contrato).

### Nivel 1 — Crítico: incumplimiento visible en cada respuesta

1. **Ausencia total de timeouts. (A — diferido al final, ver Secuencia
   recomendada)** Sin deadline de cabeceras, body, escritura ni idle de
   keep-alive en ninguno de los dos backends. Exposición directa a
   slowloris y prerequisito para emitir `408`, cuyo status-line ya existe
   pero nunca se usa.

~~2. Todo rechazo colapsa en `400`.~~ **Resuelto.** `rejection_reason`
   (`protocol/http/v11/rejection_reason.h`) sustituye el `verdict` binario como
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

3. **Sin TLS. (A — diferido al final, ver Secuencia recomendada)**
   detrás de un terminador TLS. Debe encajar sin romper la frontera
   protocolo/transporte.

4. **`100-continue`
   interpreta, pero no se emite `100 Continue` ni `417`. El obstáculo
   estructural es que `on_send` es de un solo uso. Sin ello, los clientes que
   envían `Expect` esperan su timeout en cada petición con cuerpo grande.

5. ~~**Una excepción~~ **[RESUELTO]** Se añadió `rejection_reason::kHandlerError`
   (mapea a 500 Internal Server Error en `server.h`) y ambos backends
   (`tcpip_windows.h`, `tcpip_linux.h`) ahora reutilizan el canal existente
   `enqueue_error_response`/`on_bad_request_` en el `catch` que envuelve la
   llamada a `on_request_`, en lugar de cerrar la conexión silenciosamente.
   No se añadió ningún delegate/miembro/setter nuevo: se reutilizó la
   infraestructura ya existente para errores de decodificación. Build
   verificado con `run_build`: exitoso.

6. ~~**`request` sin acceso~~ **[RESUELTO]** `get_header(std::string_view)` y
   `exist_header(...)` ahora comparan de forma case-insensitive (RFC 9110
   §5.1) usando `helpers::iequals`, corrigiendo un bug de compliance además
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

7. **Sin percent-decoding. (B) — Resuelto.**
   El path del request-target (`origin-form`/`absolute-form`) se decodifica
   ahora en `decoder.h`, inmediatamente después de validar `max_uri_length` y
   antes de montar la request, mediante `helpers::percent_decode_in_place`.
   Se implementó como un algoritmo de dos punteros (lectura/escritura)
   *in-place* sobre el propio sub-rango de `absolute_path_` dentro del buffer
   del decoder: como un triplete `%HH` decodificado nunca es más largo que su
   forma codificada, el resultado siempre cabe en el mismo rango sin
   necesidad de un buffer adicional ni de una segunda pasada — coste O(n),
   cero allocations, sin impacto apreciable en el caso común (paths sin `%`).
   Un byte NUL (`%00`) decodificado se rechaza como sintácticamente inválido
   (`rejection_reason::kSyntax`, mapeado a 400), al ser un vector clásico de
   inyección/bypass. `request::get_absolute_path()` expone directamente el
   path ya decodificado sin cambios de API ni de almacenamiento en
   `request.h`; `router.h` no requirió cambios, ya que consume ese mismo
   getter.

   Nota para un futuro punto (p. ej. el handler de ficheros estáticos, ítem
   12): un `%2F` decodificado se convierte en un `/` literal indistinguible
   de un separador de segmento real. Cualquier lógica que vuelva a segmentar
   el path por `/` después de este punto debe tener esto en cuenta (p. ej.
   para mitigar path-traversal), ya que la decodificación ocurre una única
   vez, antes del enrutado, sobre el path completo.

8. **Solo se acepta `HTTP/1.1`. (B) — Resuelto por decisión de alcance.**
    Doba es un framework HTTP/1.1-only por diseño (ver cabecera de este
    documento y comentarios de `decoder.h`/`server.h`); no se implementará
    semántica `HTTP/1.0` (framing sin chunked, cierre implícito por defecto,
    etc.), ya que eso ampliaría el alcance del proyecto en lugar de corregir
    un incumplimiento de compliance HTTP/1.1. El comportamiento actual ya es
    conforme: una versión bien formada pero distinta de `1.1` que sea
    numéricamente inferior (p. ej. `HTTP/1.0`, `HTTP/0.9`) devuelve `400`;
    una versión numéricamente superior (p. ej. `HTTP/2.0`) devuelve `505`
    (cubierto por el punto 2, ya resuelto). No se requiere ningún cambio de
    código para este punto.

### Nivel 3 — Medio: paridad funcional esperada

9. **Condicionales
    están modelados y validados sintácticamente, pero nadie los evalúa: no se
    genera `304`, `412`, `206` ni `416`. Requiere decidir cómo expone el
    handler sus validadores.

11. **`OPTIONS`
    `Allow`. El router ya sabe calcular los métodos aplicables (lo hace para el
    `405`); falta exponerlo.

13. **`channel_intent::kUpgrade`
    definido y los headers `Sec-WebSocket-*` modelados, pero ningún transporte
    lo maneja. Hay que definir quién posee el socket tras el `101`, cómo se
    drena el buffer ya acumulado y cómo se desactiva el pipelining, sin filtrar
    semántica HTTP al transporte.

14. **Trailers de salida.
    `response` no tiene API para emitirlos.

### Nivel 4 — Operabilidad y confianza

15. **Sin límites de conexión
    `connections_` es solo un contador observacional: nada lo consulta para
    dejar de aceptar.

## Backlog de producto / conveniencia — no es compliance HTTP/1.1

Estos ítems no corrigen un incumplimiento de RFC 9110/9112: son
funcionalidad de conveniencia que un framework web suele ofrecer, pero que
un usuario de Doba puede implementar por su cuenta con la API ya existente
sin que el servidor deje de ser conforme. Se numeran de forma independiente
para no mezclarlos con el backlog de compliance.

P1. **Compresión (antes ítem 10).** Negociación de contenido opcional
    (`Accept-Encoding`/`Content-Encoding`) y gestión de `Vary`. Introduce
    dependencia externa, lo que choca con el "cero dependencias" del README:
    es decisión de producto, no solo técnica.

P2. **Handler de ficheros estáticos (antes ítem 12). (M)** El streaming de
    salida ya funciona, así que la base está. Depende del punto 7
    (percent-decoding, ya resuelto) por seguridad e idealmente de
    `TransmitFile`/`sendfile`.

P3. **Logging de acceso (antes ítem 16).** No hay ningún punto de
    extensión para observabilidad.

P4. **Cadena de middleware (antes ítem 17).** No hay forma de componer lógica
    transversal sin duplicarla en cada handler. Requiere diseño cuidadoso para
    no contradecir el principio de "sin maquinaria de framework".

P5. **Parsing de formularios (antes ítem 18). (M)** Ni
    `x-www-form-urlencoded` ni `multipart/form-data`. Comparte primitiva con
    el punto 7.

P6. **Suite de conformidad (antes ítem 19).** No existe una batería propia
    de casos de protocolo (framing, smuggling, pipelining, límites); cada
    cambio en el decoder es una apuesta. Es infraestructura de QA, no un
    incumplimiento de compliance en sí.

### Documentación pendiente

- Documentar el contrato de ciclo de vida para usuarios que consuman
  directamente el transporte, fuera de `v11::server`.

### Secuencia recomendada

Tanda 1 (complejidad B, alto retorno): 5, 6, 8.
Tanda 2 (correctitud de framing): resuelta (ver punto 2, Nivel 1).
Tanda 3 (robustez, excluyendo timeouts): 15.
Tanda 4 (despliegue autónomo, excluyendo TLS): 4.
Tanda 5 (paridad funcional de compliance, depende de 7): 9, 14, 11, 13.
Tanda 6 (complejidad A, diferidos al final por decisión explícita): 1, 3.
Tanda 7 (backlog de producto/conveniencia, sin dependencias bloqueantes):
  P1, P2, P3, P4, P5, P6.

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
