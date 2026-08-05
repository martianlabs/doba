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
        (entrada: framing del body recibido)
        framer_raw.h     framing de un body delimitado por Content-Length
        framer_chunked.h validación del wire chunked recibido
        framer_state.h   resultado de los body framers
        framer_error.h   errores de framing de entrada
        (entrada: lectura del body ya acumulado)
        reader.h         body::reader: selecciona reader_chunked/reader_raw
                         según el encoding real de la request
        reader_chunked.h decodificación del framing chunked al leer
        reader_raw.h     lectura directa de un body Content-Length
        reader_state.h   resultado de los body readers
        reader_error.h   errores de lectura de body
        (salida: body de la respuesta)
        writer.h         body::body_writer: fachada raw/chunked de salida
        writer_raw.h     codificación de salida con Content-Length
        writer_chunked.h codificación del wire chunked de salida
        writer_state.h   resultado de los body writers
        writer_error.h   errores de escritura de salida
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
entregue una fuente genérica para el resto de bytes. La `response` HTTP rellena
`prefix` con la status-line, los headers y el cuerpo inline cuando lo hay, y
emplaza `source` con el `common::reader` liberado por su `body::body_writer`
cuando el cuerpo se entregó como writer. Ambos transportes consumen primero
`prefix` y después drenan `source`.

## HTTP/1.1: decodificación de requests

El punto de entrada actual es `decoder<RQty, RSty>` en
`protocol/http11/decoder.h`:

1. `accumulate(char*, size)` copia en su buffer interno hasta agotar
   `kDecodingBufferSize` (16384 bytes, constante privada del decoder) y
   devuelve cuántos bytes admitió.
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

Los headers y reglas viven bajo `protocol/http11/headers/`. El mapa de
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

1. **`Date` no se emite. (B)** `common::date_server` está implementado y
   `server.h` lo incluye, pero `date_server::get()` no se invoca en ningún
   punto del árbol. RFC 9110 §6.6.1 lo exige a un origin server; sin él las
   caches intermedias no pueden calcular frescura. Decidir si se inyecta en
   `response::serialize()` (cubre también las respuestas de error del
   transporte) o en `server`.

2. **`Connection: close` no se refleja en el mensaje. (B)** El protocolo
   calcula `channel_intent::kClose` y el transporte lo honra, pero el header
   nunca se emite: el cliente ve una respuesta aparentemente persistente
   seguida de un FIN inesperado, y reutiliza la conexión.

3. **Semántica de `HEAD`, `204` y `304`. (M)** Nada suprime el cuerpo. Si un
   handler de `HEAD` llama a `set_body`, el cuerpo se envía y la conexión se
   desincroniza. Requiere que el punto de serialización conozca el método, dato
   que hoy no cruza esa frontera.

4. **Ausencia total de timeouts. (A)** Sin deadline de cabeceras, body,
   escritura ni idle de keep-alive en ninguno de los dos backends. Exposición
   directa a slowloris y prerequisito para emitir `408`, cuyo status-line ya
   existe pero nunca se usa.

5. **Todo rechazo colapsa en `400`. (M)** La causa raíz es que `verdict` es
   binario (`kAccept`/`kReject`): ninguna regla puede expresar *por qué*
   rechaza. Impide distinguir `413`, `414`, `431`, `501` y `505`, cuyos
   status-lines ya existen. Ampliar `verdict` toca todos los `interpret()` de
   `headers/` y las cuatro reglas de `rules/`: conviene planificarlo aparte.

### Nivel 2 — Alto: bloquea despliegue real o usabilidad básica

6. **Sin TLS. (A)** Ni Schannel ni OpenSSL ni ALPN. Hoy solo es desplegable
   detrás de un terminador TLS. Debe encajar sin romper la frontera
   protocolo/transporte.

7. **`100-continue` y respuestas 1xx interinas. (A)** `Expect` se parsea e
   interpreta, pero no se emite `100 Continue` ni `417`. El obstáculo
   estructural es que `on_send` es de un solo uso. Sin ello, los clientes que
   envían `Expect` esperan su timeout en cada petición con cuerpo grande.

8. **Una excepción del handler cierra sin `500`. (B)** En ambos transportes el
   `catch` alrededor de `on_request_` hace `close()` y retorna. Un bug de
   usuario se manifiesta como conexión cortada, indistinguible de un fallo de
   red. Existe `set_on_bad_request` para errores de decodificación; falta el
   simétrico para errores de handler.

9. **`request` sin acceso a headers por nombre. (B)** Solo hay
   `get_header(std::size_t)`. Leer `Authorization` obliga a iterar y comparar
   sin distinguir mayúsculas a mano. Igual para query-parameters y cookies.

10. **Sin percent-decoding ni normalización de path. (M)** No existe ninguna
    función de decodificación porcentual. Afecta al enrutado de paths
    codificados y, sobre todo, a la seguridad de cualquier handler que mapee a
    disco. Debe decodificarse después de segmentar por `/`, nunca antes.

11. **Solo se acepta `HTTP/1.1`. (B)** Cualquier otra versión devuelve
    `kInvalidSource` y acaba en `400`. Falta semántica `HTTP/1.0` (cierre
    implícito, prohibición de chunked) y `505` para versiones superiores; esto
    último depende del punto 5.

### Nivel 3 — Medio: paridad funcional esperada

12. **Condicionales y `Range` sin semántica. (A)** `etag`, `if_*` y `range`
    están modelados y validados sintácticamente, pero nadie los evalúa: no se
    genera `304`, `412`, `206` ni `416`. Requiere decidir cómo expone el
    handler sus validadores.

13. **Compresión y negociación de contenido. (A)** Sin gzip/deflate/br ni
    gestión de `Vary`. Introduce dependencia externa, lo que choca con el
    "cero dependencias" del README: es decisión de producto, no solo técnica.

14. **`OPTIONS` por recurso y `TRACE`. (M)** `OPTIONS *` responde `200` sin
    `Allow`. El router ya sabe calcular los métodos aplicables (lo hace para el
    `405`); falta exponerlo.

15. **Sin handler de ficheros estáticos. (M)** El streaming de salida ya
    funciona, así que la base está. Depende del punto 10 por seguridad e
    idealmente de `TransmitFile`/`sendfile`.

16. **`channel_intent::kUpgrade` sin contrato de traspaso. (A)** El valor está
    definido y los headers `Sec-WebSocket-*` modelados, pero ningún transporte
    lo maneja. Hay que definir quién posee el socket tras el `101`, cómo se
    drena el buffer ya acumulado y cómo se desactiva el pipelining, sin filtrar
    semántica HTTP al transporte.

17. **Trailers de salida. (M)** El lado de entrada los conserva y expone;
    `response` no tiene API para emitirlos.

### Nivel 4 — Operabilidad y confianza

18. **Sin límites de conexión ni backpressure de aceptación. (M)**
    `connections_` es solo un contador observacional: nada lo consulta para
    dejar de aceptar.

19. **Sin logging de acceso, métricas ni trazas. (M)** No hay punto de
    extensión para observabilidad.

20. **Sin cadena de middleware. (M)** No hay forma de aplicar lógica
    transversal sin duplicarla en cada handler. Requiere diseño cuidadoso para
    no contradecir el principio de "sin maquinaria de framework".

21. **Sin parsing de formularios. (M)** Ni `x-www-form-urlencoded` ni
    `multipart/form-data`. Comparte primitiva con el punto 10.

22. **Sin suite de conformidad. (M)** Solo un harness de ejemplo. Sin batería
    de casos de protocolo (framing, smuggling, pipelining, límites), cada
    cambio en el decoder es una apuesta.

### Documentación pendiente

- Documentar el contrato de ciclo de vida para usuarios que consuman
  directamente el transporte, fuera de `http11::server`.

### Secuencia recomendada

Tanda 1 (complejidad B, alto retorno): 1, 2, 8, 9, 11.
Tanda 2 (correctitud de framing): 3, luego 5 aislado.
Tanda 3 (robustez): 4, 18.
Tanda 4 (despliegue autónomo): 6, 7.
Tanda 5 (paridad funcional, depende de 5 y/o 10): 12, 17, 14, 21, 10, 15, 16.
Tanda 6 (producto/observabilidad, sin dependencias bloqueantes): 13, 19, 20, 22.

## Estado de pruebas y documentación

El único subproyecto de prueba configurado es `test/ut-001-main`, que es un
harness/ejemplo de servidor; no debe presentarse como una suite unitaria
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
