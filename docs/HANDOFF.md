# Doba - Documento de traspaso de contexto

Este documento describe el arbol de codigo actual, conserva el contexto de
decisiones ya resueltas y mantiene los backlogs tecnicos pendientes. Las
afirmaciones sobre la implementacion deben contrastarse con el codigo cuando
se retome un punto.

## Proyecto y reglas de trabajo

Doba es un framework de servidor C++20 header-only. La frontera esencial es
generica: el transporte no debe conocer tipos ni semantica de HTTP/1.1. El
protocolo comunica al transporte el resultado generico de deserializacion y la
intencion del canal.

- Build manual: CMake 3.20 o superior y C++20. Los presets `msvc-debug` /
  `msvc-release` requieren CMake 3.25 o superior.
- Todo archivo C++ nuevo debe llevar la cabecera Apache/doba exacta de un
  archivo equivalente.
- `AGENTS.md` es la norma de trabajo vigente. Antes de modificar codigo,
  pruebas, configuracion o documentacion hay que presentar un plan concreto y
  esperar aprobacion explicita.
- Este documento aporta contexto tecnico, pero no anade reglas de trabajo
  independientes ni sustituye a `AGENTS.md`.
- El arbol puede contener cambios locales del usuario. No sobrescribir ni
  revertir cambios ajenos.
- Los tests de cada componente replican bajo `tests/unit`,
  `tests/integration` o `tests/package` la ruta de su header bajo `include`.
  Los helpers y los `CMakeLists.txt` compartidos permanecen en la raiz de la
  suite.

### Codificacion y formato del codigo fuente

La politica vigente para los archivos fuente C++ esta definida en `AGENTS.md`:

- UTF-8 **sin** BOM.
- Finales de linea CRLF en el working tree.
- Newline final obligatorio.
- Contenido exclusivamente ASCII (0x00-0x7F).
- Limite de 80 columnas por linea.
- Sin whitespace al final de linea.

`.editorconfig` aplica actualmente estas reglas a los archivos C++ bajo
`include/` y `examples/`; no cubre `tests/` ni la documentacion.
`.gitattributes` normaliza a CRLF los archivos de texto en el working tree,
pero no fuerza por si mismo el encoding, la ausencia de BOM, ASCII ni el limite
de columnas. Esas restricciones siguen siendo politica obligatoria para C++
aunque una ruta no este incluida todavia en `.editorconfig`.

Al no haber BOM, `CMakeLists.txt` pasa `/utf-8` a MSVC para fijar el juego de
caracteres de origen y de ejecucion.

En los comentarios se usa `S` para las secciones RFC (por ejemplo
`RFC 9110 S7.6.1`) y `-` en lugar de raya. Cualquier sustitucion dentro de un
comentario enmarcado debe conservar el ancho original para no desalinear la
`|` de cierre.

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

## Contrato protocolo <-> transporte

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
   `kDecodingBufferSize` (16384 bytes, alias privado del decoder que reutiliza
   `limits::kDecodingBufferSize`) y devuelve cuantos bytes admitio.
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
cuando cabe en `kMaxBodySizeInMemory` y fija `Content-Length`; si no cabe,
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

Los campos de metadatos de la implementacion actual son principalmente
`std::string_view` y `header_view`; no documentar esos getters como copias
owned. Cualquier cambio en su propiedad o lifetime requiere una revision
explicita de la interaccion entre `decoder`, `request_getter` y el buffer del
decoder.

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
`stop()`. La lectura de la fecha publicada permanece lock-free.

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
tipados; cualquier otra
combinacion con `*` se rechaza durante `add_route`.

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

Lo que sigue pendiente en el transporte no es el backend en si, sino politicas
transversales aun no implementadas en ninguno de los dos: timeouts (C1) y
limites de conexion efectivos (C5).

No modificar la frontera protocolo/transporte para resolver una necesidad
exclusiva de HTTP. Si un cambio requiere semantica HTTP, debe vivir en la capa
HTTP o expresarse en el contrato generico ya existente.

## Estado de compliance y hardening HTTP/1.1

C7 documenta una correccion RFC completada. C1 y C5 son protecciones
operativas pendientes. C2-C4 son funcionalidad opcional o responsabilidad de
la aplicacion, no gates de compliance del core.

### Hardening operativo pendiente

C1. **Timeout unico de inactividad. (M)** Ninguno de los dos backends cierra
    una conexion que permanece abierta sin realizar progreso.

    - **Alcance inicial:** un unico `inactivity_timeout`, configurado antes de
      `start()`, aplicable durante lectura, keep-alive y escritura.
    - **Progreso:** recibir o enviar bytes renueva el plazo. La duracion total
      de la conexion o de una request no lo consume mientras exista progreso.
    - **Vencimiento:** el transporte cierra la conexion de forma segura y
      conserva las garantias actuales de lifetime, orden y callbacks. No tiene
      que generar automaticamente una respuesta `408`.
    - **Frontera:** la configuracion es comun y cada fichero de plataforma
      implementa la espera y el cierre con sus primitivas de IOCP o epoll.
    - **Fuera del alcance inicial:** plazos distintos para head, body,
      escritura o keep-alive, duracion maxima absoluta, configuracion dinamica
      y generacion automatica de `408`.
    - **Evidencia de cierre:** tests con lectura fragmentada que progresa,
      request parcial detenida, keep-alive inactivo y cliente que no lee. Deben
      comprobar plazo, cierre y un unico callback en Windows y Linux.

### Correccion RFC completada

C7. **Fechas condicionales invalidas. (B/M, completada)**

    - **Causa confirmada:** el dispatcher generico convertia el fallo del
      checker estricto de fecha en rechazo de toda la request.
    - **Resolucion:** el decoder acepta y conserva el field-value de
      `If-Modified-Since` e `If-Unmodified-Since` aunque la fecha sea invalida,
      como exigen RFC 9110 S13.1.3 y S13.1.4. La sintaxis estructural general
      del field-value y los checkers de fecha independientes siguen siendo
      estrictos.
    - **Alcance:** no se ha implementado evaluacion de precondiciones ni se ha
      cambiado `If-Range`.
    - **Verificacion:** tests unitarios de decoder para ambos campos y test de
      integracion sobre TCP/IP que confirma una respuesta `200` en lugar de
      `400`.

### Funcionalidad opcional o responsabilidad de la aplicacion

C2. **Evaluacion automatica de condicionales y rangos. (Reclasificada)**

    - **Situacion actual:** Doba valida y conserva los campos condicionales para
      que el handler pueda evaluarlos con los validadores de su recurso.
    - **Responsabilidad:** cuando la aplicacion actua como origin server, el
      handler debe aplicar las precondiciones de RFC 9110 S13.2. Doba no conoce
      la representacion seleccionada y no debe inventar sus metadatos.
    - **Range:** RFC 9110 S14 define los rangos como opcionales; ignorar `Range`
      y responder como a un GET normal es valido.
    - **Decision:** un helper automatico para `304`, `412`, `206` o `416` es
      funcionalidad de conveniencia diferida. Si se solicita, necesitara un plan
      propio y debera respetar la precedencia de RFC 9110 S13.2.2.

C3. **Trailers de salida. (Reclasificada)**

    - **Situacion actual:** `response` no expone una API para emitir trailers.
    - **Decision:** es una capacidad opcional y queda en el backlog de producto,
      no como requisito de compliance ni gate de release.
    - **Condicion futura:** si se implementa, debe limitarse al framing que los
      permite y respetar las restricciones de RFC 9110 S6.5.

C4. **`OPTIONS` automatico para recursos. (Reclasificada)**

    - **Situacion actual:** `OPTIONS *` se genera automaticamente y la
      aplicacion puede registrar handlers `OPTIONS` para recursos concretos.
    - **Decision:** sintetizar esas respuestas desde el router es conveniencia
      opcional, no un requisito del core ni un gate de release.
    - **Condicion futura:** si se implementa, debe reutilizar el calculo de
      `Allow` del `405` y conservar la prioridad actual de rutas.

### Proteccion operativa pendiente

C5. **Maximo global de conexiones activas. (M)** `connections_` es solo un
    contador observacional y no limita la admision.

    - **Alcance inicial:** un unico limite global, configurado antes de
      `start()`. Al alcanzarlo, cada backend cierra inmediatamente la nueva
      conexion sin afectar a las ya admitidas.
    - **Frontera:** la configuracion es comun y la reserva atomica se aplica en
      IOCP y epoll antes de admitir el contexto como conexion activa.
    - **Fuera del alcance inicial:** cuotas por worker, cambios dinamicos,
      backpressure de aceptacion, callbacks nuevos y respuestas HTTP de rechazo.
    - **Evidencia de cierre:** superar el maximo en Windows y Linux, comprobar
      que las conexiones existentes siguen atendidas y que el cupo se recupera
      exactamente una vez al cerrar cada conexion.

## Pendientes de robustez arquitectonica y operativa

R1. **Rollback de `server::start`. (Media, completada)**

    - **Causa confirmada:** una excepcion de `transport_.start()` dejaba una
      adquisicion de `date_server` sin liberar.
    - **Resolucion:** la capa HTTP libera solo su adquisicion fallida y relanza
      la excepcion original. `stop()` no libera recursos de una instancia que
      no llego a arrancar.
    - **Verificacion:** el transporte fake fuerza la excepcion y comprueba que
      el servicio de fecha queda detenido al liberar el propietario restante.

R2. **Limites efectivos de request. (Alta, diferida)**

    - **Situacion confirmada:** las politicas de `max_content_length`,
      `max_forwarding_hops`, `max_transfer_codings`, `max_uri_length` y
      `max_header_section_size` usan cero como ilimitado y el `server`
      predeterminado no expone una via para configurarlas.
    - **Distincion:** este punto limita recursos por request y no es C5, que
      controla la admision del numero de conexiones activas.
    - **Decision:** queda expresamente fuera del plan de hardening actual. No
      se han cambiado defaults, API, decoder, transportes ni comportamiento.
    - **Trabajo futuro:** definir en un plan propio defaults seguros y la API
      minima de configuracion previa a `start()`, con rechazo antes de consumir
      el body y sin penalizar el hot path.

## Backlog de producto / conveniencia - no es compliance HTTP/1.1

Estos items no corrigen un incumplimiento de RFC 9110/9112: son
funcionalidad de conveniencia que un framework web suele ofrecer, pero que
un usuario de Doba puede implementar por su cuenta con la API ya existente
sin que el servidor deje de ser conforme. Se numeran de forma independiente
para no mezclarlos con el backlog de compliance.

P1. **Handler de ficheros estaticos. (M)** El streaming de
    salida ya funciona, asi que la base esta. El percent-decoding, del que
    depende por seguridad, ya esta implementado; idealmente usaria
    `TransmitFile`/`sendfile`.

P2. **Logging de acceso.** No hay ningun punto de
    extension para observabilidad.

P3. **Cadena de middleware.** No hay forma de componer logica
    transversal sin duplicarla en cada handler. Requiere diseno cuidadoso para
    no contradecir el principio de "sin maquinaria de framework".

P4. **Parsing de formularios. (M)** Ni
    `x-www-form-urlencoded` ni `multipart/form-data`. Comparte primitiva con
    el percent-decoding ya implementado.

P5. **Suite de conformidad exhaustiva.** La suite de integracion propia usa
    sockets reales y cubre framing, fragmentacion, pipelining y cierre, pero no
    constituye aun una matriz exhaustiva de conformidad RFC.

    - **Riesgo:** faltan casos sistematicos de request smuggling, todas las
      fragmentaciones relevantes, limites configurados y combinaciones de
      framing ambiguo.
    - **Impacto:** una regresion puede atravesar tests locales y manifestarse
      solo con segmentacion TCP concreta, requests concatenadas o clientes que
      ejercen limites y casos ambiguos.
    - **Componentes afectados:** decoder, reglas de framing, request/response,
      contrato protocolo-transporte y ambos backends TCP/IP.
    - **Verificacion requerida:** ampliar la bateria con casos positivos y
      negativos basados en RFC 9110/9112, incluyendo mensajes fragmentados en
      todos los puntos relevantes, `Content-Length`/`Transfer-Encoding`,
      chunked, trailers y limites configurados.
    - **Por que importa:** es la evidencia necesaria para sostener una
      declaracion publica de implementacion HTTP/1.1 estricta.

## Pendientes de QA y validacion

Los 115 ficheros de unit tests cubren ampliamente parsers, headers y objetos de
valor. Los puntos siguientes describen capas de validacion adicionales; no
invalidan esa cobertura, pero si limitan la confianza sobre integracion,
concurrencia, seguridad y rendimiento. Se identifican como `QA1`-`QA5`.

QA1. **Integracion sobre IOCP y epoll. (Alta, completada)**

    - **Situacion actual:** 38 casos de transporte usan sockets loopback reales
      y el mismo codigo de test selecciona IOCP en Windows y epoll en Linux.
      Un caso adicional ejercita la composicion HTTP/1.1 sobre TCP/IP.
    - **Cobertura:** aceptacion, recepcion fragmentada, envios acotados,
      desconexion, errores, streaming, orden y callbacks.
    - **CI:** las suites de integracion se ejecutan en las matrices Windows y
      Linux y bajo los sanitizers configurados.
    - **Limite:** esta cobertura no sustituye la suite exhaustiva descrita en
      P5 ni las campanas prolongadas de estres.

QA2. **Concurrencia, pipelining y ciclo de vida. (Alta, completada)**

    - **Situacion actual:** hay casos enfocados para pipelines sincronos y
      diferidos, respuestas fuera de orden, clientes concurrentes, excepciones
      de handler, cancelacion, apagado con estados mixtos y reinicio repetido.
    - **Ciclo de vida global:** `date_server` tiene pruebas de varios
      propietarios y de `start()`/`stop()` concurrentes; el servidor HTTP tiene
      una prueba de rollback y reintento tras un fallo de transporte.
    - **Limite:** no hay soak tests prolongados ni control determinista de cada
      interleaving posible entre workers. Son validaciones futuras, no ausencia
      de cobertura funcional.

QA3. **Sanitizers integrados; fuzzing diferido. (Alta, parcial)**

    - **Situacion actual:** CI compila con Clang y ejecuta CTest bajo ASan,
      UBSan y TSan. Cada job ejecuta las suites unitaria y de integracion.
    - **Riesgo residual:** los tests escritos no exploran automaticamente
      combinaciones inesperadas ni todos los limites del decoder y los body
      framers.
    - **Componentes afectados:** helpers HTTP, decoder, body framers/readers,
      request rebasing, response serialization, byte storage y transportes.
    - **Decision:** el fuzzing no esta integrado en CI y queda fuera del plan
      de hardening actual. Requiere un plan independiente antes de retomarse.
    - **Por que importa:** los sanitizers detectan defectos en caminos
      ejecutados; el fuzzing ampliaria sistematicamente esos caminos.

QA4. **Adaptadores de benchmark disponibles; baseline pendiente. (Media)**

    - **Situacion actual:** existen adaptadores versionados para HttpArena y Web
      Frameworks Benchmark. HttpArena fija la revision upstream y ambos
      Dockerfiles permiten fijar la revision de Doba mediante `DOBA_REF`.
    - **Riesgo:** no hay un baseline de release persistente ni un gate que mida
      throughput, latencia, asignaciones, memoria o escalado por conexiones. El
      runner upstream de Web Frameworks tampoco tiene un commit fijado.
    - **Impacto:** no se pueden detectar regresiones ni comparar decisiones como
      `shared_ptr`, `std::function`, spill a disco o tamanos de buffer. Tampoco
      se puede cuantificar la promesa de "high-performance".
    - **Componentes afectados:** decoder, router, response serialization,
      streaming de bodies y ambos transportes.
    - **Verificacion requerida:** fijar escenarios y baselines de release para
      request minima, headers grandes, body inline/streaming, pipelining y
      concurrencia; registrar toolchain, hardware y distribucion de latencias.
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

RE1. **Matriz CI y warnings estrictos. (Alta, completada)**

    - **Situacion actual:** CI compila y ejecuta CTest con GCC y Clang en Debug
      y Release, MSVC en Debug y Release, y Clang con ASan, UBSan y TSan.
    - **Warnings:** `DOBA_ENABLE_STRICT_WARNINGS` esta desactivado por defecto y
      CI lo activa. Aplica `-Wall -Wextra -Wpedantic -Werror` con GCC/Clang y
      `/W4 /WX` con MSVC solo a targets propios del arbol.
    - **Contrato de consumo:** los flags no forman parte de la interfaz
      instalada `martianlabs::doba`.
    - **Evidencia requerida:** cada cambio debe conservar verde la matriz; el
      estado de un checkout local no sustituye la ejecucion remota de CI.

RE2. **Consumo e instalacion CMake. (Alta, completada)**

    - **Situacion actual:** el proyecto obtiene su version de
      `include/version.h`, instala headers y package config, y exporta
      `martianlabs::doba`.
    - **Verificacion:** CI instala en un prefijo aislado y compila consumidores
      externos Debug y Release mediante `find_package(doba CONFIG REQUIRED)`.
      Tambien compila un consumidor mediante `add_subdirectory` y comprueba
      que no se incorporan los ejemplos ni tests internos de doba.
    - **Contrato:** el target instalado aporta includes, C++20 y Threads sin
      heredar opciones internas de warnings o sanitizers.

RE3. **Version minima de CMake verificada. (Media, completada)**

    - **Situacion actual:** el proyecto declara CMake 3.20; los presets MSVC
      declaran 3.25 y ambas fronteras estan documentadas por separado.
    - **Verificacion:** CI configura, compila y ejecuta CTest con CMake 3.20.6.
      La matriz Windows usa los presets MSVC.

RE4. **Gobierno y trazabilidad de release. (Media, parcial)**

    - **Situacion actual:** la version de CMake procede de `include/version.h`
      y el workflow puede crear manualmente una GitHub Release despues de la
      matriz completa. Existen licencia y README.
    - **Pendiente:** no hay changelog, politica de seguridad ni guia de
      contribucion.
    - **Riesgo:** consumidores y contribuidores aun no tienen canales
      documentados para vulnerabilidades, compatibilidad o contribuciones.
    - **Verificacion pendiente:** definir esos documentos y comprobar que cada
      tag referencia una matriz CI verde y documentacion sincronizada.

## Deuda tecnica de C++ y mantenibilidad

DT1 se conserva como antecedente completado. Los pendientes activos son DT4 y
DT5; no deben bloquear automaticamente la primera release y requieren un plan
propio antes de abordarse.

DT1. **Buffers propietarios bajo RAII explicito. (Media, completada)**

    - **Situacion actual:** `decoder` y `request` poseen sus buffers mediante
      `std::unique_ptr<char[]>` y reservan con `make_unique_for_overwrite`, sin
      inicializacion adicional. Sus destructores son los predeterminados.
    - **Verificacion funcional:** las suites completas pasan con GCC y MSVC en
      Debug y Release con warnings estrictos, y con GCC bajo ASan y UBSan. Los
      tests cubren lifetime de views, percent-decoding, dispatch incremental y
      bodies.
    - **Layout:** con GCC 13, `request` conserva 200 bytes y `decoder` 712.
    - **Rendimiento:** cinco muestras de `/pipeline`, una conexion y profundidad
      32, dieron medianas de 377142 req/s antes y 376937 req/s despues (-0,05%,
      dentro del ruido), con p50 de 75 us en ambos casos.
    - **Resultado:** no quedan buffers propietarios gestionados mediante
      `new[]`/`delete[]` en el codigo.

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

Decisiones de producto explicitas. Estos puntos **no se numeran** junto al
resto de items (ni de compliance ni de producto) porque no forman parte de
ningun lote planificado para la primera release de Doba. Se conservan aqui
como pendientes futuros, para que no se pierda el contexto ya analizado.

- **TLS. (A)** El despliegue previsto para la primera release
  es detras de un terminador TLS (reverse proxy). Cuando se aborde, debe
  encajar sin romper la frontera protocolo/transporte.

- **Compresion / GZIP.** Negociacion de contenido
  opcional (`Accept-Encoding`/`Content-Encoding`) y gestion de `Vary`.
  Introduce dependencia externa, lo que choca con el "cero dependencias" del
  README.

- **Streaming HTTP progresivo y SSE. (A)** El `body_writer` actual permite
  drenar una fuente generica, pero el handler debe terminar de producirla antes
  de entregar la respuesta; no constituye streaming progresivo. La futura API
  debera representar inicio, fragmentos y final o error, aplicar backpressure
  por bytes y agrupar fragmentos pequenos sin penalizar el camino one-shot ni
  el hot path sincrono.

- **Barrera ordenada de upgrade. (A)** Antes de cambiar de protocolo,
  el `101` debe alcanzar la cabeza del orden de respuestas. Solo entonces se
  puede detener la decodificacion HTTP, entregar al nuevo codec los bytes
  residuales ya recibidos y transferir el control del canal. La frontera debe
  seguir siendo generica y probarse primero con un codec ficticio, sin filtrar
  semantica HTTP a IOCP ni epoll.

- **WebSockets / manejo de `channel_intent::kUpgrade`. (A)**
  `channel_intent::kUpgrade` esta definido y los headers `Sec-WebSocket-*`
  estan modelados, pero ningun transporte lo maneja. Depende de la barrera de
  upgrade anterior y requerira handshake, framing, fragmentacion,
  `ping`/`pong`/`close`, envios iniciados externamente y backpressure
  bidireccional. No es un incumplimiento de RFC 9110/9112: un servidor conforme
  puede rechazar toda peticion de upgrade. Lo ya modelado se conserva tal cual.

Este aplazamiento se limita al streaming publico, al cambio de codec y a
WebSockets. Los limites, clientes lentos, pruebas de estres y baselines de
rendimiento permanecen en sus backlogs respectivos; no forman parte de un plan
aprobado por el mero hecho de aparecer en este documento.

### Documentacion pendiente

- Documentar el contrato de ciclo de vida para usuarios que consuman
  directamente el transporte, fuera de `v11::server`.
- Documentar lifetime y propiedad de los `string_view` devueltos por `request`,
  y las precondiciones de getters indexados.
### Secuencia recomendada

Las "tandas" son lotes de ejecucion, no un ranking de criticidad: agrupan
items que conviene abordar juntos porque comparten dependencias o naturaleza.
Los numeros son los identificadores de los items de este documento.

| Tanda | Criterio                         | Items             | Estado     |
|-------|----------------------------------|-------------------|------------|
| 1     | Complejidad B, alto retorno      | (5, 6, 8)         | Completada |
| 2     | Correctitud de framing           | (2)               | Completada |
| 3     | Correccion RFC                    | C7                | Completada |
| 4     | Despliegue autonomo              | (2)               | Completada |
| 5     | Funcionalidad opcional           | C2, C3, C4        | Diferida |
| 6     | Hardening operativo              | C1, C5            | Objetivo beta |
| 7     | Producto y conveniencia          | P1, P2, P3, P4, P5 | Pendiente |
| 8     | QA integral                      | QA1, QA2           | Completada |
| 9     | Preparacion de release           | RE1-RE4           | RE4 parcial |
| 10    | Deuda tecnica medida             | DT4, DT5          | Pendiente  |

**Objetivo fijado para `0.1.0-beta.1`:** completar las dos protecciones
operativas acotadas C1 y C5. C7 ya esta completado. C2-C4 estan diferidos y no
son gates de compliance ni de release.

1. Item C1 - Timeout unico de inactividad.
2. Item C5 - Maximo global de conexiones activas.

C1 y C5 comparten configuracion generica y ejecucion especifica en IOCP y
epoll. Cada item necesita su propio plan aprobado y pruebas equivalentes en
Windows y Linux. C1 no incluye inicialmente respuestas `408` ni plazos por
fase; C5 no incluye cuotas por worker ni configuracion dinamica.

La aplicacion sigue siendo responsable de evaluar las precondiciones cuando
actua como origin server. Doba no declara soporte automatico de condicionales,
rangos, `OPTIONS` de recurso ni trailers de salida.

La salida de `0.1.0-beta.1` exige ademas verificar C1 y C5 sobre sockets reales
y completar RE1-RE4.

TLS, compresion/GZIP, streaming HTTP/SSE, la barrera de upgrade y WebSockets
no aparecen en ninguna tanda ni en la numeracion de items: ver "Fuera del
alcance de la primera release".

## Estado de pruebas y documentacion

El arbol configura dos ejecutables CTest: `doba_unit_tests` y
`doba_integration_tests`. El inventario versionado actual contiene 115
ficheros unitarios `*_tests.cpp` con 401 casos `DOBA_TEST`, y dos ficheros de
integracion con 39 casos. Los tests de componente replican bajo cada suite la
ruta del header probado a partir de `include/`; los helpers compartidos quedan
en la raiz de la suite.

Los adaptadores de h1spec y Http11Probe existen, pero su ejecucion sigue siendo
manual. Su automatizacion y el fuzzing estan diferidos fuera del plan de
hardening actual. El inventario anterior es estructural; el estado debe
confirmarse mediante configuracion, compilacion y CTest antes de
afirmar que un checkout concreto pasa.

## Al continuar

1. Leer `AGENTS.md`, este documento y los archivos directamente relacionados
   con la tarea.
2. Antes de editar, presentar un plan con causa, archivos, cambio, pruebas y
   riesgos; esperar aprobacion explicita.
3. Preservar la API publica, el modelo de ownership y el estilo del modulo
   afectado salvo que el encargo aprobado requiera cambiarlos.
4. Mantener los cambios estrictamente locales. No anadir abstracciones,
   sobrecargas publicas, configuracion, dependencias ni cambios de CMake sin
   que formen parte del plan aprobado.
5. Tras un cambio aprobado, comunicar exactamente que se verifico y que queda
   sin verificar.
