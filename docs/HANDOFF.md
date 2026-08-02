# Doba — Documento de traspaso de contexto

Este documento describe el árbol de código actual. No es un historial de
auditorías ni una lista de cambios previstos.

## Proyecto y reglas de trabajo

Doba es un framework de servidor C++20 header-only. La frontera esencial es
genérica: el transporte no debe conocer tipos ni semántica de HTTP/1.1. El
protocolo comunica al transporte el resultado genérico de deserialización y la
intención del canal.

- Raíz de trabajo: `D:\projects\martianlabs\doba`.
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
    http11/
      decoder.h          decoder incremental de peticiones HTTP/1.1
      request.h          objeto de request y materialización diferida
      request_getter.h   callback que termina de construir la request
      response.h         respuesta de buffer fijo
      server.h           composición HTTP/1.1 de router y transporte
      router.h           selección de rutas estáticas, parametrizadas y wildcard
      router_types.h     resultados de match y conversión de parámetros
      router_handler_static.h
                         contrato del handler de ruta estática
      router_handler_parametrized.h
                         handler tipado para parámetros de path
      router_handler_parametrized_base.h
                         interfaz interna de handlers parametrizados
      body/
        writer_raw.h     acumulación de body con Content-Length
        writer_chunked.h validación/acumulación del wire chunked
        writer_state.h   resultado de los body writers
        writer_error.h   errores de chunked
        reader.h         body::reader: selecciona reader_chunked/reader_raw
                         según el encoding real de la request
        reader_chunked.h decodificación del framing chunked al leer
        reader_raw.h     lectura directa de un body Content-Length
        reader_state.h   resultado de los body readers
        reader_error.h   errores de lectura de body
      headers/           checkers e intérpretes de headers y reglas
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
entregue una fuente genérica para el resto de bytes; la `response` HTTP actual
solo rellena el prefijo con su cuerpo inline.

## HTTP/1.1: decodificación de requests

El punto de entrada actual es `decoder<RQty, RSty>` en
`protocol/http11/decoder.h`:

1. `accumulate(char*, size)` copia hasta `RQty::kMaxHeadSize` en su buffer
   interno.
2. `deserialize()` llama a `parse_core()` mientras no haya body pendiente o a
   `parse_body()` cuando ya se ha elegido un body writer.
3. `parse_core()` procesa request-line y headers una vez. Usa
   `header_dispatchers_`, un `common::hash_map` con punteros a función.
4. Los headers modelados actualizan `context_`; al terminar los headers se
   aplican `framing`, `routing`, `directives` y `policy`.
5. Si hay body, el decoder selecciona `body::writer_chunked` cuando la
   conexión indica chunked, o `body::writer_raw` cuando hay Content-Length.
6. Al completarse la request, `dispatch()` construye el
   `deserialization_result` y reinicia el estado del decoder.

Los headers y reglas viven bajo `protocol/http11/headers/`. El mapa de
dispatch actual contiene los headers comprobados y los dispatchers dedicados
para Host, Content-Length, Transfer-Encoding, Connection, TE, Trailer,
Expect, Upgrade, Max-Forwards, Via, Forwarded y los tres X-Forwarded.

### Body de entrada

`body::writer_raw` copia exactamente los bytes delimitados por Content-Length a
un `common::writer`. `body::writer_chunked` valida el framing chunked y conserva
en el almacenamiento todos los bytes wire, incluidos tamaño de chunk,
extensiones, trailers y terminador. No decodifica el payload a una forma
distinta.

`common::writer` entrega el resultado como `common::byte_storage`.
`request_getter<RQty>` es el callback que recibe opcionalmente ese storage y
termina de devolver una `std::shared_ptr<RQty>`. La implementación actual de
`request::from` recibe además si la request usa chunked encoding y su
Content-Length, crea un `common::reader` propietario del `byte_storage` y lo
envuelve en un `body::reader` (ver `protocol/http11/body/reader.h`), que
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
No hay una sobrecarga actual de `set_body` que acepte un serializer o un source
de body externo.

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

## Pendientes técnicos conocidos

- [ ] Añadir a `response` bodies de salida externos/serializers y establecer
  el framing HTTP correcto (`Content-Length` o `Transfer-Encoding: chunked`).

- [ ] Hacer que los transportes Windows y Linux consuman
  `serialization_result::source` después de `prefix`.

- [ ] Hacer que el backend Windows respete `deserialization_result::channel`,
  especialmente `channel_intent::kClose`.

- [ ] Definir un contrato explícito para transferir el canal cuando un
  protocolo devuelva `channel_intent::kUpgrade`.


- [ ] Documentar el contrato de ciclo de vida para usuarios que consuman
  directamente el transporte, fuera de `http11::server`.

## Estado de pruebas y documentación

Los subproyectos de prueba configurados son `test/ut-001-main` y
`test/ut-002-common-io`. `ut-001-main` es un harness/ejemplo de servidor; no
debe presentarse como una suite unitaria exhaustiva sin revisar sus casos y
aserciones concretas. `ut-002-common-io` cubre `common::reader`/`common::writer`
sobre `common::byte_storage`.

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
