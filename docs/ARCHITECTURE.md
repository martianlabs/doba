# Arquitectura de doba

[Indice](HANDOFF.md)

Doba es un framework de servidor C++20 header-only para Windows y Linux.
Este documento describe la implementacion y sus contratos. El trabajo futuro
se mantiene en [BACKLOG.md](BACKLOG.md).

## Contenido

- [Mapa de componentes](#arquitectura-actual)
- [Contrato protocolo-transporte](#contrato-entre-protocolo-y-transporte)
- [Decodificacion y bodies](#http11-decodificacion-de-requests)
- [Objetos HTTP publicos](#objetos-http-publicos)
- [Transporte](#transporte)

## Arquitectura actual

```text
include/
  common/
    byte_storage.h       almacenamiento temporal en memoria o fichero
    reader.h             lector move-only de byte_storage o span prestado
    writer.h             escritor move-only sobre byte_storage
    date_server.h        generacion de Date
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
        policies.h       politicas de decodificacion (0 = sin limite)
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

## Contrato entre protocolo y transporte

`protocol/deserialization.h` define:

- `deserialization_status`: `kSucceeded`, `kInvalidSource` y
  `kMoreBytesNeeded`.
- `channel_intent`: `kKeep`, `kClose` y `kUpgrade`.
- `deserialization_result<RQty>`, que entrega una `std::shared_ptr<RQty>` y la
  intencion de ciclo de vida del canal. Incluye ademas `reason`, un `int`
  opaco con el motivo de rechazo, e `interim`, un `std::string_view` con
  bytes que el protocolo necesita en el cable antes de poder continuar. El
  transporte los escribe sin interpretarlos, reservandoles su propio
  `response_id`, y su almacenamiento pertenece al protocolo (hoy lo usa
  HTTP/1.1 para el `100 Continue`).

El transporte debe basarse unicamente en esos tipos genericos. HTTP decide si
una conexion se mantiene o se cierra; el transporte no debe inspeccionar
headers, metodos ni status-lines.

`protocol/serialization.h` define `serialization_result`:

```cpp
struct serialization_result {
  std::string prefix;
  std::optional<common::reader> source;
};
```

`prefix` posee los bytes ya materializados. `source` permite que un protocolo
entregue una fuente generica para el resto de bytes. La `response` HTTP rellena
`prefix` con la status-line, los headers y el cuerpo inline cuando lo hay, y
emplaza `source` con el `common::reader` liberado por su `body::body_writer`
cuando el cuerpo se entrego como writer. Ambos transportes consumen primero
`prefix` y despues drenan `source`.

## HTTP/1.1: decodificacion de requests

El punto de entrada actual es `decoder<RQty, RSty>` en
`protocol/http/v11/decoder.h`:

1. `accumulate(char*, size)` copia en su buffer interno hasta agotar
   `limits::kDecodingBufferSize` (5120 bytes) y devuelve cuantos bytes admitio.
2. `deserialize()` llama a `parse_core()` mientras no haya body pendiente o a
   `parse_body()` cuando ya se ha elegido un body framer.
3. `parse_core()` procesa request-line y headers una vez. Usa
   `header_dispatchers_`, un `common::hash_map` con punteros a funcion.
4. Los headers modelados actualizan `context_`; al terminar los headers se
   aplican `framing`, `routing`, `directives` y `policy`.
5. Si hay body, el decoder abre un `common::writer` con `spill_threshold` de
   65535 bytes y selecciona `body::framer_chunked` cuando la conexion indica
   chunked, o `body::framer_raw(content_length)` en caso contrario.
6. Al completarse la request, `dispatch()` construye el
   `deserialization_result` y reinicia el estado del decoder.

Los headers y reglas viven bajo `protocol/http/v11/headers/`. El mapa de
dispatch actual contiene los headers comprobados y los dispatchers dedicados
para Host, Content-Length, Transfer-Encoding, Connection, TE, Trailer,
Expect, Upgrade, Max-Forwards, Via, Forwarded y los tres X-Forwarded.

### Body de entrada

Las clases de entrada son los `framer`, no los `writer` (estos ultimos son de
salida). `body::framer_raw` delimita exactamente los bytes indicados por
Content-Length. `body::framer_chunked` valida el framing chunked y conserva
todos los bytes wire, incluidos tamano de chunk, extensiones, trailers y
terminador. Ninguno decodifica el payload a una forma distinta: el decoder
vuelca lo aceptado a un `common::writer`.

`common::writer` entrega el resultado como `common::byte_storage`.
`request_getter<RQty>` es el callback que recibe opcionalmente ese storage y
termina de devolver una `std::shared_ptr<RQty>`. La implementacion actual de
`request::from` recibe ademas si la request usa chunked encoding y su
Content-Length, crea un `common::reader` propietario del `byte_storage` y lo
envuelve en un `body::reader` (ver `protocol/http/v11/body/reader.h`), que
selecciona internamente `body::reader_chunked` o `body::reader_raw` segun el
encoding real usado por la request. `request::get_body_reader()` expone ese
`body::reader` ya construido cuando hay body, de forma que el llamador no
necesita saber que framing se uso para leer el payload decodificado.

`common::byte_storage` empieza en memoria y puede derramar a fichero cuando
`spill_threshold` es mayor que cero. `common::reader` es move-only y permite
leer el storage, obtener bytes individuales y vaciarlo a un `std::string` con
`read_all`. Tambien ofrece `reader::borrowed(span)`: esa variante no posee los
bytes y exige que el almacenamiento del llamador sobreviva al reader y a todos
los objetos a los que se mueva.

`finish()` sella tanto `common::writer` como `common::byte_storage`: las
escrituras posteriores fallan y repetir `finish()` no altera el tamano final.
El contrato es identico para almacenamiento en memoria y para spill a fichero.

### Body de salida

`body::body_writer` (`body/writer.h`) es la fachada del cuerpo de respuesta.
Se construye con `body_writer::raw(opts)` o `body_writer::chunked(opts)` y
mantiene internamente un `std::variant<writer_chunked, writer_raw>` sobre un
`common::writer`, por lo que tambien puede derramar a fichero.

`response::set_body(std::string_view)` copia el payload a la zona inline
cuando cabe en `limits::kMaxResponseBodySizeInMemory` y fija `Content-Length`; si no cabe,
crea internamente un writer raw y delega en el.
`response::set_body(body::body_writer&&)` adopta un writer ya construido por
el llamador. En ambos casos `apply_body_framing()` emite
`Transfer-Encoding: chunked` para writers chunked o un `Content-Length`
derivado de `bytes_written()` para los raw. `serialize()` consume el writer y
lo entrega como `serialization_result::source`.

`serialize()` nunca entrega contenido para respuestas 1xx, 204, 205 o 304.
Una respuesta 205 elimina `Transfer-Encoding` y fuerza `Content-Length: 0`.

## Objetos HTTP publicos

### `request`

`request` no es copiable ni movible. Su construccion publica se realiza a
traves de `request::from(...)`, que devuelve `request_getter<request>`.

Expone metodo, forma del target, path, headers, parametros de query, Host y
autoridad del request-target, ademas de `get_body_reader()`.

Los getters devuelven principalmente `std::string_view` y `header_view`,
no copias propietarias. `request` copia el head en su propio buffer RAII y
rebasa las vistas sobre ese almacenamiento. Las vistas devueltas dependen de
la vida de la request. El detalle del contrato publico y sus precondiciones
se sigue en el [backlog de documentacion](BACKLOG.md#documentacion-publica).

### `response`

`response` no es copiable y usa `char memory_[4096]`:

- status-line y headers ocupan la parte inicial;
- el body inline se reserva al final;
- `set_body(std::string_view)` copia el body a esa zona;
- `serialize()` devuelve `protocol::serialization_result` con el contenido
  inline en `prefix`.

La API incluye `add_header`, `set_header`, `has_header`, `get_header` por clave
o indice, `get_headers_length`, `remove_header` y los helpers de status-line.
Para el cuerpo existen las dos sobrecargas de `set_body` descritas en "Body de
salida": una inline y otra que adopta un `body::body_writer`.

### `server` y `router`

`server` esta parametrizado por request, response, decoder, transporte y
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

Los servidores comparten el singleton `common::date_server`. Su ciclo de vida
cuenta propietarios bajo mutex y mantiene el servicio activo hasta el ultimo
`stop()`. La lectura de la fecha publicada usa atomicos sin tomar ese mutex.

El router registra rutas estaticas, parametrizadas y wildcard mediante
`add_route(method, path, lambda)`. El patron parametrizado usa segmentos
`:nombre`, por ejemplo `/users/:id`. Los parametros se convierten en orden y
soportan `std::string`, `bool`, tipos integrales y tipos de punto flotante.

`match` evalua primero las rutas estaticas, despues las parametrizadas y por
ultimo las wildcard. `server` traduce la ausencia de ruta o de metodo
permitido a 404 y 405, respectivamente.

Los paths son sensibles a mayusculas. Una ruta parametrizada con barra final
solo coincide con un path que tambien la tenga. Un patron que termina en `/*`
coincide con cualquier path que empiece por su prefijo, incluida la porcion
vacia: `/assets/*` coincide con `/assets/` y con `/assets/a/b`, pero no con
`/assets`. La ruta wildcard no entrega esa porcion como argumento: el handler
puede consultarla en la request. La prioridad es estatica, parametrizada y
wildcard. Ante un metodo no permitido, el servidor genera `405` y anade
`Allow` con los metodos aplicables, incluidos los wildcard. Una wildcard
contiene un unico `*` como segmento final y su handler no admite parametros
tipados; cualquier otra combinacion con `*` se rechaza durante `add_route`.

## Transporte

Ambos backends estan implementados y operativos. `transport/server/tcpip.h`
selecciona `tcpip_windows.h` (IOCP) o `tcpip_linux.h` (EPOLL) en tiempo de
compilacion segun la plataforma; no hay backend de referencia. Los dos exponen
exactamente la misma API publica al protocolo
(`start`/`stop`, callbacks de request, bad-request y ciclo de vida de canal).

Ambos backends consumen `serialization_result` de la misma forma: primero
vuelcan `prefix` una sola vez y despues drenan `source` en trozos acotados por
`kSendChunkSz`, deteniendose cuando el buffer de envio alcanza
`kSendBufferMaxSz`. Un `failed()` del reader cierra el contexto; una lectura de
cero bytes retira la respuesta.

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

La semantica exclusiva de HTTP reside en la capa HTTP. El transporte recibe
unicamente las operaciones expresadas por el contrato generico.
