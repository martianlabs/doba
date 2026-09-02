# Doba — Documento de traspaso de contexto

Este documento describe el árbol de código actual, conserva el contexto de
decisiones ya resueltas y mantiene los backlogs técnicos pendientes. Las
afirmaciones sobre la implementación deben contrastarse con el código cuando
se retome un punto.

## Proyecto y reglas de trabajo

Doba es un framework de servidor C++20 header-only. La frontera esencial es
genérica: el transporte no debe conocer tipos ni semántica de HTTP/1.1. El
protocolo comunica al transporte el resultado genérico de deserialización y la
intención del canal.

- Build manual: CMake 3.20 o superior y C++20. Los presets `msvc-debug` /
  `msvc-release` requieren CMake 3.25 o superior.
- Todo archivo C++ nuevo debe llevar la cabecera Apache/doba exacta de un
  archivo equivalente.
- `AGENTS.md` es la norma de trabajo vigente. Antes de modificar código,
  pruebas, configuración o documentación hay que presentar un plan concreto y
  esperar aprobación explícita.
- Este documento aporta contexto técnico, pero no añade reglas de trabajo
  independientes ni sustituye a `AGENTS.md`.
- El árbol puede contener cambios locales del usuario. No sobrescribir ni
  revertir cambios ajenos.

### Codificación y formato del código fuente

La política vigente para los archivos fuente C++ está definida en `AGENTS.md`:

- UTF-8 **sin** BOM.
- Finales de línea CRLF en el working tree.
- Newline final obligatorio.
- Contenido exclusivamente ASCII (0x00-0x7F).
- Límite de 80 columnas por línea.
- Sin whitespace al final de línea.

`.editorconfig` aplica actualmente estas reglas a los archivos C++ bajo
`include/` y `examples/`; no cubre `tests/` ni la documentación.
`.gitattributes` normaliza a CRLF los archivos de texto en el working tree,
pero no fuerza por sí mismo el encoding, la ausencia de BOM, ASCII ni el límite
de columnas. Esas restricciones siguen siendo política obligatoria para C++
aunque una ruta no esté incluida todavía en `.editorconfig`.

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
`add_route(method, path, lambda)`. El handler recibe `const RQty&` y `RSty&`
como sus dos primeros argumentos. El patrón parametrizado usa segmentos
`:nombre`, por ejemplo
`/users/:id`, y sus parámetros se declaran a continuación en la lambda. Se
convierten en orden y soportan `std::string`, `bool`, tipos integrales y tipos
de punto flotante.

`match` evalúa primero las rutas estáticas, después las parametrizadas y por
último las wildcard. Devuelve el handler coincidente; `server` lo invoca
directamente dentro del callback del transporte y traduce la ausencia de ruta
o de método permitido a 404 y 405, respectivamente.

Los paths son sensibles a mayúsculas. Una ruta parametrizada con barra final
solo coincide con un path que también la tenga. Un patrón que termina en `/*`
coincide con cualquier path que empiece por su prefijo, incluida la porción
vacía: `/assets/*` coincide con `/assets/` y con `/assets/a/b`, pero no con
`/assets`. La ruta wildcard no entrega esa porción como argumento: el handler
puede consultarla en la request. La prioridad es estática, parametrizada y
wildcard. Ante un método no permitido, el router añade `Allow` con los métodos
aplicables, incluidos los wildcard, pero `server` lo descarta actualmente al
establecer el status `405` (C6). Una wildcard contiene un único `*` como
segmento final y su handler no admite parámetros tipados; cualquier otra
combinación con `*` se rechaza durante `add_route`.

## Transporte

Ambos backends estan implementados y operativos. `transport/server/tcpip.h`
selecciona `tcpip_windows.h` (IOCP) o `tcpip_linux.h` (EPOLL) en tiempo de
compilacion segun la plataforma; no hay backend de referencia ni fallback
sincrono. Los dos exponen exactamente la misma API publica al protocolo
(`start`/`stop`, callbacks de request, bad-request y ciclo de vida de canal).
Cada request deserializada ejecuta su handler de forma directa en el worker del
transporte. Cuando este retorna, la respuesta se serializa y se añade a la cola
FIFO del contexto, conservando el orden de las requests pipelined.

Ambos backends consumen `serialization_result` de la misma forma: primero
vuelcan `prefix` una sola vez (marca `prefix_written`) y despues drenan
`source` en trozos acotados por `kSendChunkSz`, deteniendose cuando el buffer
de envio alcanza `kSendBufferMaxSz`. Un `failed()` del reader cierra el
contexto; una lectura de cero bytes retira la respuesta.

En Windows, IOCP conserva el contexto mediante `shared_ptr` mientras haya
operaciones solapadas y protege el estado de envio con `sending_mutex_`. En
Linux, cada worker posee su instancia EPOLL, su listener configurado con
`SO_REUSEPORT` y los contextos que acepta; todas las operaciones EPOLL y de
socket de esos contextos se realizan en su propio worker.

Se distinguen dos formas de terminar, identicas en ambos backends:

- Cierre ordenado (`close`): se deja de leer, se drena todo lo que sea seguro
  enviar y el contexto se destruye cuando no quedan respuestas. Lo usan EOF,
  `channel_intent::kClose` y un fallo de serializacion.
- Aborto fatal (`abort`): el flujo de salida ya esta corrupto (fallo del
  socket, decoder o source), asi que no se envia nada mas y el contexto se
  retira de inmediato.

`tcpip::stop()` debe invocarse desde fuera de un worker del transporte. Los
callbacks del transporte no pueden cambiar mientras `start()` o `stop()` estan
activos.

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

### Nivel 1 — Crítico: incumplimiento visible en cada respuesta

C1. **Ausencia total de timeouts. (A - diferido al final, ver Secuencia
    recomendada)** Sin deadline de cabeceras, body, escritura ni idle de
    keep-alive en ninguno de los dos backends. Exposición directa a
    slowloris y prerequisito para emitir `408`, cuyo status-line ya existe
    pero nunca se usa.

    - **Que implica:** medir el progreso de cada conexion y aplicar limites
      independientes al head, body, escritura pendiente e inactividad entre
      requests. Al agotarse un plazo, el transporte debe dejar de leer y
      cerrar de forma ordenada o abortar segun el estado del envio.
    - **Frontera:** los relojes, la espera de I/O y el cierre pertenecen a cada
      backend; la politica debe exponerse de forma generica. `v11::server`
      decide el significado HTTP, incluido el `408` cuando aun sea posible
      responder, sin que IOCP ni epoll interpreten HTTP.
    - **Decision pendiente:** fijar los plazos iniciales, si son constantes o
      una politica configurable, y que ocurre cuando ya hay respuestas
      pipelined pendientes. La eleccion afecta API publica, consumo de timers y
      semantica de cierre, por lo que requiere un plan propio aprobado.
    - **Por que es necesario:** sin limites temporales una conexion puede
      retener socket, contexto y buffers enviando bytes lentamente o dejando
      bloqueada una escritura. Es el mayor riesgo operativo y puede retener
      recursos sin limite.
    - **Evidencia de cierre:** clientes locales que fragmenten el head y el
      body, no lean una respuesta, mantengan un keep-alive idle y usen
      pipelining. Los mismos casos deben comprobar plazo, bytes finales,
      cierre y callbacks en Windows y Linux.

### Nivel 2 - Alto: incumplimiento en respuestas concretas

C6. **Las respuestas `405` pierden el header obligatorio `Allow`. (B)**

    - **Comportamiento observado:** una peticion con un metodo no registrado
      sobre un recurso que si tiene rutas devuelve `405 Method Not Allowed`,
      pero la respuesta enviada no contiene `Allow`. Se reprodujo sobre el
      cable con rutas `GET` y `HEAD` y una peticion `POST` al mismo path.
    - **Mecanismo de fallo:** `router::match` calcula los metodos aplicables y
      ejecuta `res->set_header(header_names::kAllow, allow)` antes de devolver
      `kMethodNotAllowed`. A continuacion, `server` llama a
      `res->method_not_allowed_405()`. Ese helper entra en `response::sln`, que
      reinicia `hdr_len_` y elimina todos los headers ya presentes, incluido
      `Allow`, antes de crear el nuevo `Content-Length`.
    - **Impacto:** la respuesta incumple RFC 9110 S15.5.6, que exige generar un
      campo `Allow` en una respuesta `405`. El cliente recibe el status
      correcto, pero no puede descubrir que metodos admite el recurso.
    - **Componentes afectados:** `protocol/http/common/router.h`,
      `protocol/http/v11/server.h` y el reinicio de estado realizado por
      `protocol/http/v11/response.h`.
    - **Correccion esperada:** mantener la responsabilidad del status en
      `server`, pero conservar el valor calculado por el router y establecerlo
      despues de `method_not_allowed_405()`. La solucion debe aceptar routers
      personalizados que devuelvan `kMethodNotAllowed` sin haber creado
      `Allow`, sin cambiar la API publica ni la semantica general de los
      helpers de status.
    - **Verificacion requerida:** comprobar sobre el cable que las rutas
      estaticas, parametrizadas y wildcard producen `405` con todos los
      metodos aplicables, sin duplicados y en el orden establecido por el
      router. Cubrir tambien un router personalizado que no establezca `Allow`
      y confirmar que `404` y las respuestas normales no cambian.
    - **Por que importa:** `Allow` forma parte obligatoria de la semantica de
      `405`; calcularlo internamente no aporta conformidad si se pierde antes
      de serializar la respuesta.

C7. **OBLIGATORIO PARA LA RELEASE - Los condicionales con fecha invalida
    rechazan la request completa. (B/M)**

    - **Comportamiento confirmado:** Http11Probe `CAP-IMS-INVALID` envia
      `If-Modified-Since: not-a-date`. Doba responde `400 Bad Request` y cierra
      la conexion; la peticion deberia continuar como si el campo no estuviera
      presente.
    - **Incumplimiento:** RFC 9110 S13.1.3 exige ignorar
      `If-Modified-Since` cuando su valor no es un `HTTP-date` valido. RFC 9110
      S13.1.4 establece la misma regla para un `If-Unmodified-Since` invalido.
    - **Mecanismo de fallo:** `if_modified_since::check()` e
      `if_unmodified_since::check()` delegan en `date::check()`. El dispatcher
      generico de headers comprobados convierte cualquier `false` en
      `verdict::kReject`, por lo que una semantica especifica de "ignorar el
      campo" termina rechazando toda la request antes de que la aplicacion
      pueda procesarla.
    - **Componentes afectados:**
      `protocol/http/common/headers/if_modified_since.h`,
      `protocol/http/common/headers/if_unmodified_since.h` y
      `protocol/http/v11/decoder.h`. Debe auditarse tambien `If-Range`, porque
      tiene reglas contextuales de ignorado y usa el mismo dispatcher.
    - **Correccion esperada:** conservar la validacion ABNF de los checkers,
      pero no convertir un valor invalido de estos campos en rechazo global.
      Deben respetarse ademas la precedencia de `If-None-Match` sobre
      `If-Modified-Since` y la de `If-Match` sobre `If-Unmodified-Since`.
    - **Verificacion requerida:** pruebas externas enfocadas con fechas
      invalidas, listas de fechas y combinaciones de precedencia; confirmar
      respuesta normal, continuidad de la conexion y ausencia de regresiones
      para fechas validas. Repetir Http11Probe y comprobar que
      `CAP-IMS-INVALID` pasa.
    - **Gate de release:** este defecto debe estar corregido y verificado antes
      de publicar la release. No puede cerrarse suprimiendo, desactivando o
      reclasificando la prueba.

### Nivel 3 - Medio: paridad funcional esperada

C2. **Condicionales sin evaluar. (M/A)** Los headers condicionales
    están modelados y validados sintácticamente, pero nadie los evalúa: no se
    genera `304`, `412`, `206` ni `416`. Es objetivo de `0.1.0-beta.1`.
    C7 cubre por separado la admision incorrecta de valores invalidos y es un
    gate obligatorio anterior a esta semantica funcional.
    Antes de implementarlo hay que decidir la API mínima con la que el handler
    expone validadores y representación, y evaluar `If-Match`,
    `If-None-Match`, `If-Modified-Since`, `If-Unmodified-Since`, `Range` e
    `If-Range` con las reglas de precedencia aplicables.

    - **Que implica:** separar la comprobacion de precondiciones de la
      ejecucion del handler y seleccionar la respuesta antes de enviar una
      representacion: `304` para una lectura no modificada, `412` para una
      precondicion falsa, `206` para un rango satisfacible y `416` para uno
      fuera de representacion. Los headers de respuesta asociados deben ser
      coherentes con el status y el cuerpo finalmente emitido.
    - **Frontera:** el parser ya entrega los campos; conocer el ETag, la fecha
      de modificacion, el tamano y la fuente de la representacion es
      responsabilidad de la aplicacion. El transporte no participa. La capa
      HTTP debe proporcionar el punto minimo para que el handler declare esos
      datos sin duplicar o adivinar semantica de recurso.
    - **Decision pendiente:** definir si los validadores se establecen en la
      respuesta, se devuelven desde el handler o se exponen mediante otro
      contrato. Tambien hay que fijar el alcance inicial de rangos multiples y
      de `If-Range`; no deben anunciarse como soportados hasta tener una
      semantica completa y pruebas de wire format.
    - **Por que es necesario:** aceptar sintacticamente precondiciones y luego
      ignorarlas puede entregar un cuerpo que el cliente ya posee, sobrescribir
      un recurso que el cliente condiciono o servir un rango incorrecto. Es el
      mayor bloque de semantica HTTP pendiente y el unico que probablemente
      requiere una ampliacion deliberada de API publica.
    - **Evidencia de cierre:** matriz de requests para cada combinacion de
      metodo, ETag, fecha, rango y `If-Range`, verificando precedencia, status,
      `Content-Range`, `Content-Length`, ETag/fecha, ausencia de body en `304`
      y continuidad de la conexion.

C3. **Trailers de salida. (M)**
    `response` no tiene API para emitirlos. Es objetivo de `0.1.0-beta.1`:
    la API mínima debe declarar nombres y valores, limitar los trailers a
    respuestas chunked, emitir `Trailer` antes del body y serializar los
    campos tras el chunk terminador. Debe rechazar los campos prohibidos en
    trailers y no alterar las respuestas raw ni inline.

    - **Que implica:** reservar los nombres de trailer antes de serializar los
      headers, aceptar sus valores mientras se escribe el body chunked y emitir
      los field-lines solo despues del chunk de tamano cero. La serializacion
      debe seguir funcionando cuando el body se entrega por `body_writer` y se
      derrama a fichero.
    - **Frontera:** es framing de respuesta HTTP/1.1; no requiere cambios en
      request, router ni transporte, que ya drenan un `serialization_result`.
      La respuesta debe conservar la propiedad de los bytes hasta terminar el
      envio, igual que hoy conserva el writer de body.
    - **Decision pendiente:** concretar la API minima para declarar nombres y
      establecer valores, y decidir como se informa un intento invalido sobre
      un body raw, inline o despues de iniciar la serializacion. No se deben
      permitir campos cuyo significado depende de haberlos recibido antes del
      body.
    - **Por que es necesario:** sin trailers un usuario no puede emitir de
      forma correcta metadatos que solo conoce al terminar un streaming, como
      integridad o estadisticas. No afecta la seguridad basica, pero completa
      la capacidad chunked ya expuesta por `body_writer`.
    - **Evidencia de cierre:** capturar bytes en el cable para body chunked en
      memoria y en fichero, con cero y varios trailers; confirmar `Trailer`,
      chunk terminador, orden, rechazo de campos prohibidos y ausencia de
      regresion en respuestas raw e inline.

C4. **`OPTIONS` sin respuesta automática para recursos concretos. (B)**
    `OPTIONS *` ya responde `200` en `server.h` y el router ya calcula los
    metodos aplicables a un recurso para el caso `405`, aunque C6 impide que
    ese valor llegue actualmente al cliente. Falta responder automaticamente
    a un `OPTIONS` dirigido a un recurso concreto reutilizando ese calculo.

    - **Que implica:** detectar `OPTIONS` sobre una ruta existente, obtener los
      metodos aplicables con la misma prioridad de rutas que usa `405` y emitir
      una respuesta `200` con `Allow`, sin ejecutar el handler del recurso.
    - **Frontera:** la resolucion pertenece al router y la respuesta al
      servidor HTTP; no debe duplicarse el algoritmo de match ni introducirse
      conocimiento de rutas en el transporte.
    - **Decision pendiente:** decidir si `OPTIONS` se incluye en `Allow` cuando
      lo resuelve automaticamente el servidor y como se comporta ante rutas no
      encontradas, parametrizadas y wildcard. Esa regla debe ser unica para
      `OPTIONS` y `405`.
    - **Por que es necesario:** da al cliente una forma estandar de descubrir
      las capacidades del recurso y reutiliza informacion que Doba ya calcula.
      Depende de C6 para que `Allow` sobreviva a la construccion de la
      respuesta.
    - **Evidencia de cierre:** requests a rutas estaticas, parametrizadas y
      wildcard, con rutas solapadas y metodos no registrados; comprobar status,
      `Allow`, ausencia de ejecucion del handler y que `OPTIONS *` no cambia.

### Nivel 4 - Operabilidad y confianza

C5. **Sin límites de conexión efectivos. (B/M)**
    `connections_` es solo un contador observacional: nada lo consulta para
    dejar de aceptar.

    - **Que implica:** establecer un maximo de conexiones activas y decidir la
      admision antes de crear o registrar un contexto. Al alcanzar el limite,
      el listener debe rechazar o cerrar de inmediato conexiones nuevas sin
      alterar las existentes.
    - **Frontera:** la admision se ejecuta en el transporte, donde ocurre
      `accept`; la politica debe ser generica para no convertir IOCP ni epoll
      en capas HTTP. El contador de `v11::server` puede informar la politica,
      pero no basta si se incrementa despues de aceptar.
    - **Decision pendiente:** fijar el limite inicial, su configurabilidad, el
      comportamiento de rechazo y la distribucion entre workers Linux. Si la
      politica necesita un callback nuevo, hay que preservar a los usuarios que
      consumen el transporte directamente.
    - **Por que es necesario:** sin una admision efectiva un atacante o una
      carga accidental puede agotar descriptores, memoria y workers antes de
      que los limites de request o los timeouts tengan oportunidad de actuar.
      Es una proteccion operativa, no una regla de sintaxis HTTP.
    - **Evidencia de cierre:** abrir mas conexiones que el maximo y comprobar
      que las primeras siguen atendidas, las posteriores no reservan contexto
      de larga vida, los contadores vuelven a cero y la politica es igual en
      Windows y Linux.

## Pendientes de robustez arquitectonica y operativa

R1. **Un fallo durante `server::start` no revierte explicitamente los
    subsistemas ya iniciados. (Media, pendiente de reproduccion)**

    - **Comportamiento observado:** `server::start` inicia `date_server` y el
      transporte antes de marcarse como iniciado. El transporte limpia su
      estado si falla, pero la capa `server` no captura la excepcion para
      detener `date_server` ni para restaurar explicitamente toda la secuencia.
    - **Impacto:** un bind fallido, puerto invalido o error de inicializacion
      puede dejar hilos auxiliares activos hasta un `stop()` posterior o la
      destruccion del servidor. Tambien debe verificarse que un segundo
      `start()` sea seguro despues del fallo.
    - **Componentes afectados:** `protocol/http/v11/server.h`,
      `common/date_server.h` y ambos transportes.
    - **Verificacion requerida:** forzar cada fallo de inicializacion posible,
      comprobar que no quedan hilos o handles activos y volver a iniciar la
      misma instancia correctamente.
    - **Por que importa:** el arranque debe ofrecer una garantia clara de todo
      o nada; es esencial para reinicios, pruebas y gestores de servicio.

## Backlog de producto / conveniencia — no es compliance HTTP/1.1

Estos ítems no corrigen un incumplimiento de RFC 9110/9112: son
funcionalidad de conveniencia que un framework web suele ofrecer, pero que
un usuario de Doba puede implementar por su cuenta con la API ya existente
sin que el servidor deje de ser conforme. Se numeran de forma independiente
para no mezclarlos con el backlog de compliance.

P1. **Handler de ficheros estáticos. (M)** El streaming de
    salida ya funciona, así que la base está. El percent-decoding, del que
    depende por seguridad, ya está implementado; idealmente usaría
    `TransmitFile`/`sendfile`.

P2. **Logging de acceso.** No hay ningún punto de
    extensión para observabilidad.

P3. **Cadena de middleware.** No hay forma de componer lógica
    transversal sin duplicarla en cada handler. Requiere diseño cuidadoso para
    no contradecir el principio de "sin maquinaria de framework".

P4. **Parsing de formularios. (M)** Ni
    `x-www-form-urlencoded` ni `multipart/form-data`. Comparte primitiva con
    el percent-decoding ya implementado.

P5. **Suite de conformidad.** No existe una bateria propia
    ejecutada contra el servidor real para framing, request smuggling,
    pipelining, limites y cierre de conexion.

    - **Riesgo:** los unit tests validan piezas aisladas del decoder y de los
      headers, pero no demuestran que la composicion decoder-servidor-transporte
      produce bytes y decisiones de canal correctos frente a secuencias reales.
    - **Impacto:** una regresion puede atravesar tests locales y manifestarse
      solo con segmentacion TCP concreta, requests concatenadas o clientes que
      ejercen limites y casos ambiguos.
    - **Componentes afectados:** decoder, reglas de framing, request/response,
      contrato protocolo-transporte y ambos backends TCP/IP.
    - **Verificacion requerida:** bateria de casos positivos y negativos basada
      en RFC 9110/9112, incluyendo mensajes fragmentados en todos los puntos
      relevantes, pipelining, `Content-Length`/`Transfer-Encoding`, chunked,
      trailers, `Expect`, cierres y limites configurados.
    - **Por que importa:** es la evidencia necesaria para sostener una
      declaracion publica de implementacion HTTP/1.1 estricta.

## Pendientes de QA y validacion

Los 111 ficheros de unit tests cubren ampliamente parsers, headers y objetos de
valor. Los puntos siguientes describen capas de validacion adicionales; no
invalidan esa cobertura, pero si limitan la confianza sobre integracion,
concurrencia, seguridad y rendimiento. Se identifican como `QA1`-`QA5`.

QA1. **No hay pruebas de integracion sobre IOCP y epoll. (Alta)**

    - **Riesgo:** no se ejercitan sockets reales, aceptacion, recepcion parcial,
      envio parcial, rearmado, desconexion, errores del sistema ni orden de
      completaciones con el ejecutable de tests.
    - **Impacto:** diferencias entre backends o regresiones de ciclo de vida
      pueden compilar y pasar los unit tests sin ser detectadas. Esto afecta
      directamente la promesa publica de paridad Windows/Linux.
    - **Componentes afectados:** `transport/server/tcpip_windows.h`,
      `transport/server/tcpip_linux.h`, `protocol/http/v11/server.h` y el
      contrato de `protocol/deserialization.h`/`serialization.h`.
    - **Verificacion requerida:** levantar el servidor en un puerto efimero,
      ejecutar clientes locales y comprobar bytes, orden, cierre y callbacks.
      La misma especificacion de casos debe ejecutarse en Windows y Linux.
    - **Por que importa:** los transportes contienen la concurrencia y el ciclo
      de vida mas complejos del proyecto, pero son la zona menos cubierta por
      pruebas automatizadas.

QA2. **Faltan pruebas de concurrencia, pipelining y ciclo de vida. (Alta)**

    - **Riesgo:** no hay pruebas enfocadas para multiples workers, pipelining,
      excepciones de handler, `start`/`stop` repetido o apagado bajo carga.
    - **Impacto:** races, deadlocks, cierres incompletos o callbacks duplicados
      pueden aparecer solamente bajo interleavings concretos. R1 depende de
      esta cobertura para confirmar su mecanismo y su solucion.
    - **Componentes afectados:** servidor, ciclo de vida de contextos y colas
      `responses_` de ambos backends.
    - **Verificacion requerida:** tests deterministas con barreras/latches para
      controlar interleavings, mas pruebas repetidas de estres con muchas
      conexiones y requests pipelined.
    - **Por que importa:** la ejecucion y el envio ocurren en workers de I/O;
      su ciclo de vida es una caracteristica central de Doba, no un detalle
      interno.

QA3. **No hay fuzzing ni ejecucion con sanitizers. (Alta)**

    - **Riesgo:** el parser procesa entrada hostil mediante offsets, views,
      `memcpy`/`memmove`, buffers fijos y maquinas de estado. Los unit tests no
      exploran automaticamente combinaciones inesperadas ni todos los limites.
    - **Impacto:** lecturas fuera de rango, use-after-free, integer overflow,
      data races o estados de parser no alcanzados pueden permanecer ocultos.
    - **Componentes afectados:** helpers HTTP, decoder, body framers/readers,
      request rebasing, response serialization, byte storage y transportes.
    - **Verificacion requerida:** targets de fuzz para request-line, headers,
      chunked y secuencias incrementales; ASan/UBSan en Clang o GCC y las
      opciones equivalentes disponibles en MSVC. TSan debe evaluarse para los
      caminos concurrentes donde la plataforma lo permita.
    - **Por que importa:** una libreria de red que acepta bytes no confiables
      necesita evidencia de seguridad de memoria mas alla de casos escritos a
      mano.

QA4. **No existen benchmarks reproducibles para las afirmaciones de
     rendimiento. (Media)**

    - **Riesgo:** el README describe parsing single-pass, hot path, dispatch
      directo y buffers optimizados, pero no hay baseline de throughput,
      latencia, asignaciones, memoria o escalado por numero de conexiones.
    - **Impacto:** no se pueden detectar regresiones ni comparar decisiones como
      `shared_ptr`, `std::function`, spill a disco o tamanos de buffer. Tampoco
      se puede cuantificar la promesa de "high-performance".
    - **Componentes afectados:** decoder, router, response serialization,
      streaming de bodies y ambos transportes.
    - **Verificacion requerida:** escenarios versionados y reproducibles para
      request minima, headers grandes, body inline/streaming, pipelining y
      concurrencia; registrar toolchain, hardware y distribucion de latencias,
      no solo requests por segundo.
    - **Por que importa:** las optimizaciones y futuras refactorizaciones deben
      decidirse con medidas y conservar un baseline de release.

QA5. **El harness unitario tiene aislamiento y diagnostico limitados. (Media)**

    - **Riesgo:** todos los casos viven en un unico ejecutable y el runner llama
      cada funcion sin capturar excepciones. Una excepcion inesperada puede
      terminar el proceso y ocultar el resultado de los casos restantes. Las
      aserciones fallidas tampoco imprimen expresion, fichero ni linea, aunque
      esos datos llegan a `test_helper::expect`.
    - **Impacto:** fallos intermitentes o nuevos caminos excepcionales pueden
      ofrecer diagnosticos pobres y ralentizar la localizacion de regresiones.
    - **Componentes afectados:** `tests/unit/test_helper.h`,
      `tests/unit/test_helper.cpp` y el registro CTest, que expone toda la suite
      como un solo test.
    - **Verificacion requerida:** confirmar que una excepcion en un caso no
      impide reportar el resto y que cada fallo muestra expresion y ubicacion.
      Evaluar granularidad CTest sin introducir dependencias innecesarias.
    - **Por que importa:** con cientos de casos, la calidad del diagnostico y el
      aislamiento forman parte de la mantenibilidad de la suite.

## Pendientes de release engineering

Estos puntos no cambian el comportamiento HTTP, pero son necesarios para que
una release oficial sea reproducible, consumible y verificable. Se identifican
como `RE1`-`RE4`.

RE1. **La matriz CI cubre solo builds Debug con GCC y MSVC. (Alta)**

    - **Situacion actual:** `.github/workflows/ci.yml` configura Ubuntu/GCC y
      Windows/MSVC, compila Debug y ejecuta CTest. No hay Clang, Release,
      warnings estrictos ni jobs con sanitizers.
    - **Riesgo:** errores dependientes del compilador, optimizaciones, NDEBUG,
      aliasing o diagnosticos que no emite la configuracion actual pueden llegar
      a una release etiquetada.
    - **Componentes afectados:** todos los headers publicos, ejemplos, tests,
      presets y workflow CI.
    - **Verificacion requerida:** agregar al menos Clang, una configuracion
      Release por plataforma y una politica de warnings conocida. Integrar los
      sanitizers de QA3 en jobs donde sean estables.
    - **Por que importa:** una libreria header-only se compila dentro del
      consumidor; la portabilidad de toolchain forma parte directa del producto.

RE2. **CMake no ofrece una experiencia de consumo o instalacion de release.
     (Alta)**

    - **Situacion actual:** existe un target `INTERFACE` interno llamado
      `doba_headers`, pero no hay version en `project`, alias `doba::doba`,
      reglas `install`, export de targets, package config ni artefacto
      versionado.
    - **Riesgo:** el ejemplo compila dentro del mismo arbol, pero no valida
      `find_package`, instalacion, vendoring limpio ni consumo desde otro
      proyecto. Los usuarios deben inventar su propia integracion.
    - **Componentes afectados:** `CMakeLists.txt`, futura configuracion de
      paquete y una prueba minima de consumidor externo.
    - **Verificacion requerida:** instalar en un prefijo vacio y compilar un
      proyecto separado mediante el target namespaced. Verificar Debug/Release
      y rutas sin depender del checkout original.
    - **Por que importa:** ser header-only elimina el binario, pero no elimina
      la necesidad de versionado, descubrimiento e includes transitivos claros.

RE3. **La version minima de CMake publicada no esta verificada en CI.
      (Media)**

    - **Situacion actual:** `CMakeLists.txt` declara 3.20 como minimo del
      proyecto y `CMakePresets.json` declara `cmakeMinimumRequired` 3.25; el
      README ya documenta ambas fronteras por separado.
    - **Riesgo:** la distincion esta documentada pero no comprobada: nadie
      valida que una configuracion manual con 3.20 exacto funcione.
    - **Componentes afectados:** `CMakeLists.txt`, `CMakePresets.json`, README y
      cualquier documentacion de instalacion.
    - **Verificacion requerida:** probar en CI exactamente CMake 3.20 en
      configuracion manual y CMake 3.25 con presets.
    - **Por que importa:** los requisitos de toolchain deben ser verificables y
      no contradictorios antes de publicar una release.

RE4. **Faltan artefactos basicos de gobierno y trazabilidad de release.
     (Media)**

    - **Situacion actual:** hay licencia y README, pero no se encontraron
      changelog, politica de seguridad ni guia de contribucion. Tampoco hay una
      version de proyecto que vincule codigo, documentacion y tags.
    - **Riesgo:** consumidores y contribuidores no tienen un canal documentado
      para vulnerabilidades, compatibilidad, cambios incompatibles o proceso de
      aceptacion de contribuciones.
    - **Componentes afectados:** documentacion de raiz, metadatos CMake y
      proceso de publicacion/tagging.
    - **Verificacion requerida:** definir versionado, notas de release, politica
      de reporte de seguridad y soporte; comprobar que cada tag referencia una
      matriz CI completa y documentacion sincronizada.
    - **Por que importa:** una release oficial es tambien un contrato operativo
      con sus consumidores, no solo un snapshot que compila.

## Deuda tecnica de C++ y mantenibilidad

Estos puntos no deben modernizarse por estetica ni bloquear automaticamente la
primera release. Deben abordarse solo con tests y, cuando afecten al hot path,
benchmarks. Se identifican como `DT1`-`DT5`.

DT1. **Persisten buffers propietarios gestionados con `new[]`/`delete[]`.
     (Media)**

    - **Situacion actual:** `decoder` y `request` poseen buffers `char*` y
      gestionan manualmente construccion y destruccion. El modelo actual es
      local y las clases no son copiables ni movibles, pero la propiedad no esta
      expresada por el tipo.
    - **Riesgo:** futuros retornos tempranos, cambios de construccion o soporte
      de movimiento pueden introducir fugas o doble liberacion. El rebasing de
      `string_view` en `request` depende ademas de offsets validos sobre el
      buffer fuente.
    - **Componentes afectados:** `protocol/http/v11/decoder.h` y
      `protocol/http/v11/request.h`.
    - **Verificacion requerida:** antes de cambiar ownership, cubrir
      construccion fallida, lifetime de views, percent-decoding y dispatch con
      bodies. Comparar coste y layout si se sustituye por RAII explicito.
    - **Por que importa:** hacer visible la propiedad reduce el riesgo de
      mantenimiento en la zona con mas views y offsets del parser.

DT2. **El hot path usa ownership compartido y `std::function` sin medicion
     publicada. (Media)**

    - **Situacion actual:** las requests y los contextos de Windows usan
      `shared_ptr`; los callbacks de rutas usan `std::function`.
    - **Riesgo:** conteo atomico, type erasure y asignaciones pueden afectar
      latencia y throughput, pero reemplazarlos sin datos podria aumentar la
      complejidad o romper los contratos de lifetime existentes.
    - **Componentes afectados:** contrato de transporte, router, handlers,
      request getter y ownership de contextos.
    - **Verificacion requerida:** medir primero con QA4, identificar
      asignaciones y contencion reales y cambiar solo puntos con impacto
      demostrado.
    - **Por que importa:** la afirmacion de alto rendimiento debe apoyarse en
      medidas; modernizar o micro-optimizar sin ellas seria especulativo.

DT3. **Headers publicos muy grandes exponen detalles de implementacion y
     aumentan el coste de compilacion. (Media)**

    - **Situacion actual:** `decoder.h` ronda mil lineas y cada backend de
      transporte supera esa escala. Clases auxiliares, constantes y estructuras
      de plataforma viven en namespaces accesibles desde headers publicos.
    - **Riesgo:** mayor tiempo de compilacion, mas superficie accidental para el
      consumidor y cambios internos que fuerzan recompilacion amplia. Separar
      codigo en una libreria compilada chocaria con la decision header-only.
    - **Componentes afectados:** decoder, transportes, `platform.h` y estructura
      publica de `include/`.
    - **Verificacion requerida:** medir tiempos de compilacion y superficie de
      includes; evaluar namespaces `detail`, headers internos o particiones que
      preserven el modelo header-only y la API.
    - **Por que importa:** la facilidad de consumo incluye tambien coste de
      compilacion y estabilidad de la superficie publica.

DT4. **`platform.h` concentra includes, macros y configuracion global.
     (Media)**

    - **Situacion actual:** incluye numerosos headers estandar y de sistema,
      define `INLINE`, configura macros Windows, deshabilita warning 4996 y usa
      `#pragma comment` para librerias. En una plataforma distinta de Windows o
      Linux, `tcpip.h` no selecciona backend ni emite un diagnostico explicito.
    - **Riesgo:** contaminacion de macros, warnings ocultos, dependencias
      transitivas accidentales y errores poco claros en toolchains no
      soportadas.
    - **Componentes afectados:** `include/platform.h`, transportes y cualquier
      header que dependa indirectamente de sus includes.
    - **Verificacion requerida:** inventariar includes realmente necesarios,
      confirmar el motivo de cada macro/pragma y compilar headers publicos de
      forma autosuficiente. Definir un error claro para plataformas no
      soportadas si ese es el contrato.
    - **Por que importa:** una libreria header-only comparte su preprocesador y
      politica de warnings con el consumidor.

DT5. **Algunos getters indexados publicos no validan limites. (Baja/Media)**

    - **Situacion actual:** `request::get_header(size_t)` y
      `get_query_parameter(size_t)` usan `operator[]`; el caller debe consultar
      antes la longitud. Otras APIs del mismo objeto usan excepciones u
      `optional` para ausencia.
    - **Riesgo:** un indice invalido causa comportamiento indefinido y la
      estrategia de error de la API no es uniforme.
    - **Componentes afectados:** API publica de `protocol/http/v11/request.h`,
      tests y documentacion de contrato.
    - **Verificacion requerida:** decidir y documentar si el indice valido es
      una precondicion o si debe existir comprobacion. Cualquier cambio debe
      preservar compatibilidad y medir impacto si afecta recorridos frecuentes.
    - **Por que importa:** las precondiciones de una API publica deben ser
      explicitas, especialmente cuando su incumplimiento puede producir UB.

## Fuera del alcance de la primera release

Decisiones de producto explícitas. Estos puntos **no se numeran** junto al
resto de ítems (ni de compliance ni de producto) porque no forman parte de
ningún lote planificado para la primera release de Doba. Se conservan aquí
como pendientes futuros, para que no se pierda el contexto ya analizado.

- **TLS. (A)** El despliegue previsto para la primera release
  es detrás de un terminador TLS (reverse proxy). Cuando se aborde, debe
  encajar sin romper la frontera protocolo/transporte.

- **Compresión / GZIP.** Negociación de contenido
  opcional (`Accept-Encoding`/`Content-Encoding`) y gestión de `Vary`.
  Introduce dependencia externa, lo que choca con el "cero dependencias" del
  README.

- **Streaming HTTP progresivo y SSE. (A)** El `body_writer` actual permite
  drenar una fuente genérica, pero el handler debe terminar de producirla antes
  de entregar la respuesta; no constituye streaming progresivo. La futura API
  deberá representar inicio, fragmentos y final o error, aplicar backpressure
  por bytes y agrupar fragmentos pequeños sin penalizar el camino one-shot ni
  el hot path síncrono.

- **Barrera ordenada de upgrade. (A)** Antes de cambiar de protocolo,
  el `101` debe alcanzar la cabeza del orden de respuestas. Solo entonces se
  puede detener la decodificación HTTP, entregar al nuevo codec los bytes
  residuales ya recibidos y transferir el control del canal. La frontera debe
  seguir siendo genérica y probarse primero con un codec ficticio, sin filtrar
  semántica HTTP a IOCP ni epoll.

- **WebSockets / manejo de `channel_intent::kUpgrade`. (A)**
  `channel_intent::kUpgrade` está definido y los headers `Sec-WebSocket-*`
  están modelados, pero ningún transporte lo maneja. Depende de la barrera de
  upgrade anterior y requerirá handshake, framing, fragmentación,
  `ping`/`pong`/`close`, envíos iniciados externamente y backpressure
  bidireccional. No es un incumplimiento de RFC 9110/9112: un servidor conforme
  puede rechazar toda petición de upgrade. Lo ya modelado se conserva tal cual.

Este aplazamiento se limita al streaming público, al cambio de codec y a
WebSockets. Permanecen dentro del plan actual la operación interna multi-evento,
el hardening de ciclo de vida, límites, desconexiones y clientes lentos, y la
documentación, auditoría, pruebas de estrés y benchmarks finales.

### Documentación pendiente

- Documentar el contrato de ciclo de vida para usuarios que consuman
  directamente el transporte, fuera de `v11::server`.
- Verificar sobre el cable el `100-continue` ya implementado, dentro de P5/QA1.
- Documentar lifetime y propiedad de los `string_view` devueltos por `request`,
  y las precondiciones de getters indexados.
- Resolver la cancelación cooperativa de operaciones externas que mantienen
  una corrutina suspendida. El runtime conserva su frame hasta que finaliza y
  descarta la entrega si la conexión o el transporte ya se cerraron, pero una
  operación que nunca reanude conserva ese frame indefinidamente. No debe
  destruirse de forma forzada mientras siga suspendido.

### Secuencia recomendada

Las "tandas" son lotes de ejecución, no un ranking de criticidad: agrupan
ítems que conviene abordar juntos porque comparten dependencias o naturaleza.
Los números son los identificadores de los ítems de este documento.

| Tanda | Criterio                         | Items             | Estado     |
|-------|----------------------------------|-------------------|------------|
| 1     | Complejidad B, alto retorno      | (5, 6, 8)         | Completada |
| 2     | Correctitud de framing           | (2)               | Completada |
| 3     | Compliance localizado            | C7, C6, C5        | Pendiente  |
| 4     | Despliegue autonomo              | (2)               | Completada |
| 5     | Compliance funcional             | C2, C3, C4        | Objetivo beta |
| 6     | Proteccion de conexiones         | C1                | Objetivo beta |
| 7     | Producto y conveniencia          | P1, P2, P3, P4, P5 | Pendiente |
| 8     | QA integral                      | QA1, QA2           | Gate beta  |
| 9     | Preparacion de release           | RE1-RE4           | Gate beta  |
| 10    | Deuda tecnica medida             | DT1-DT5           | Pendiente  |

**Objetivo fijado para `0.1.0-beta.1`:** cerrar los siete ítems de compliance
pendientes. El orden de ejecución busca primero corregir el incumplimiento
visible y los riesgos operativos, y después completar la semántica HTTP.

El orden es una dependencia de trabajo, no una afirmacion de criticidad pura:
C7 abre la secuencia por ser un gate obligatorio de release; C6 es el siguiente
cambio localizado y desbloquea C4; C5 y C1 comparten el diseño generico de
admision, tiempo y cierre en ambos transportes; R1 debe reproducirse mientras
se valida ese ciclo de vida; C2 requiere congelar su contrato de aplicacion
antes de editar la API; y C3 depende solo de que el framing chunked de salida
conserve sus invariantes. Por criticidad operativa, C1 sigue siendo el riesgo
principal aunque no sea el primer cambio propuesto.

1. Item C7 - Fechas condicionales invalidas rechazan la request (Nivel 2,
   obligatorio para la release).
2. Item C6 - `405` pierde el header obligatorio `Allow` (Nivel 2).
3. Ítem C5 - Sin límites de conexión efectivos (Nivel 4).
4. Ítem C1 - Ausencia total de timeouts (Nivel 1).
5. Ítem C4 - `OPTIONS` para recursos concretos (Nivel 3).
6. Ítem C2 - Evaluación de condicionales: `304`, `412`, `206`, `416`
   (Nivel 3).
7. Ítem C3 - Trailers de salida (Nivel 3).

C1 y C5 deben diseñarse como políticas genéricas de transporte: admisión de
conexiones y notificación de timeout. HTTP/1.1 decide la respuesta, incluido
el `408` cuando corresponda, sin filtrar semántica HTTP a IOCP ni epoll.
C2 puede requerir una ampliación deliberada de la API pública; su diseño y
plan de implementación necesitan aprobación explícita antes de modificarla.

La salida de `0.1.0-beta.1` exige además reproducir y resolver los riesgos
confirmados R1, ejecutar una batería wire-level para C1-C7, pipelining y
cierres en IOCP y epoll, y completar RE1-RE4. La beta podrá declarar soporte
de condicionales, rangos y trailers de salida solo tras esas verificaciones.

TLS, compresión/GZIP, streaming HTTP/SSE, la barrera de upgrade y WebSockets
no aparecen en ninguna tanda ni en la numeración de ítems: ver "Fuera del
alcance de la primera release".

## Estado de pruebas y documentación

El arbol configura `doba_unit_tests` como un unico ejecutable CTest. El
inventario actual es de 111 ficheros `*_tests.cpp`, 359 casos `DOBA_TEST` y
1427 usos de `DOBA_EXPECT`; el foco principal son parsers, headers, reglas y
objetos de valor. `examples/001-hello_world` es un ejemplo de servidor, no una
suite de pruebas. No existe todavia una bateria de conformidad ni una suite de
integracion de transporte.

Ese inventario es una medida estructural, no un resultado de ejecucion. El
estado de build y de tests debe verificarse ejecutando de nuevo configuracion,
compilacion y CTest sobre la toolchain disponible antes de afirmar que el
arbol pasa.

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
