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
      router.h           selección de rutas estáticas y parametrizadas
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
`request::from` crea un `common::reader` propietario del `byte_storage` y lo
expone mediante `request::get_body_reader()` cuando hay body.

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

`response` no es copiable ni movible y usa `char memory_[8192]`:

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
`transport::server::tcpip` y `router`. `server` hereda de
`ROty<RQty, RSty>`: `add_route` pertenece al router y `start` recibe el puerto.
Origin-form y absolute-form se enrutan; authority-form responde 501 y
asterisk-form responde 200.

El router registra rutas estáticas y parametrizadas mediante
`add_route(method, path, lambda)`. El handler recibe
`std::shared_ptr<const RQty>` y `std::shared_ptr<RSty>` como sus dos primeros
argumentos. El patrón parametrizado usa segmentos `:nombre`, por ejemplo
`/users/:id`, y sus parámetros se declaran a continuación en la lambda. Se
convierten en orden y soportan `std::string`, `bool`, tipos integrales y tipos
de punto flotante.

`match_route` evalúa primero las rutas estáticas y después las parametrizadas;
una coincidencia estática tiene prioridad. Devuelve `kMatched`, `kNotFound` o
`kMethodNotAllowed`; `server` traduce los dos últimos a 404 y 405,
respectivamente. No documentar políticas de ejecución por ruta: la API actual
no recibe ese parámetro.

## Transporte

`transport/server/tcpip.h` selecciona `tcpip_windows.h` o `tcpip_linux.h` según
la plataforma. Los dos backends deben seguir usando exclusivamente los tipos
genéricos de protocolo y conservar el orden de respuestas pipelined.

No modificar la frontera protocolo/transporte para resolver una necesidad
exclusiva de HTTP. Si un cambio requiere semántica HTTP, debe vivir en la capa
HTTP o expresarse en el contrato genérico ya existente.

## Estado de pruebas y documentación

El único subproyecto de prueba configurado es `test/ut-001-main`. Su ejecutable
es un harness/ejemplo de servidor; no debe presentarse como una suite unitaria
exhaustiva sin revisar sus casos y aserciones concretas.

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
