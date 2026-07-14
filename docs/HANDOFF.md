# 📦 Doba — Documento de Traspaso de Contexto

> Pega este documento (o su contenido) en el primer mensaje de un nuevo Copilot Chat para
> retomar el desarrollo desde otra máquina.

## 1. Qué es el proyecto

**Doba** es una **librería/framework de servidor genérico de transportes y protocolos** en
**C++20**, header-only en su núcleo de parsing/semántica.

- **Regla de oro (inviolable):** Doba es un servidor **genérico**. HTTP/1.1 es **solo un
  protocolo más**. Transportes y protocolos se pueden **mezclar libremente**. Toda solución
  debe ser **agnóstica de protocolo/transporte**: nunca filtrar tipos ni conceptos
  específicos de HTTP/1.1 hacia las capas genéricas (transporte, contrato de deserialización).
- **Antes de tomar cualquier decisión de diseño**, hay que poder garantizar que la solución
  es válida para **cualquier par transporte/protocolo**.

### Entorno / Build

| Aspecto        | Valor                                                         |
| -------------- | ------------------------------------------------------------- |
| Lenguaje       | C++20                                                         |
| Build system   | CMake ≥ 3.20 (usada: 4.3.1-msvc1) + **Ninja**                 |
| Toolchain      | **MSVC**                                                      |
| IDE            | Visual Studio Community 2026 (18.7.3)                         |
| Shell          | PowerShell                                                    |
| Workspace root | `D:\projects\martianlabs\doba\`                               |
| Repo           | `origin https://github.com/martianlabs/doba`, rama `main`     |
| Presets        | `CMakePresets.json` → `msvc-debug`, `msvc-release` (`out/build/`) |

> ⚠️ **NO realizar operaciones Git** en el repo doba (regla del proyecto).
> ⚠️ Todos los ficheros llevan cabecera de licencia Apache 2.0.

---

## 1b. Normas de desarrollo (Copilot / cualquier colaborador)

Estas reglas son **inviolables** para todo cambio de código en este repositorio.

### Correcciones y cambios

- **Pensar antes de escribir.** Antes de tocar una línea, leer el código circundante
  completo para entender el invariante que se mantiene. Una corrección que no entiende
  el invariante rompe otro cosa.
- **Una corrección, un problema.** Cada cambio resuelve exactamente el problema
  identificado y nada más. No "mejorar de paso" código adyacente en el mismo commit.
- **Verificar que el fix no abre una ventana nueva.** Para cada corrección, enumerar
  explícitamente los caminos de ejecución afectados y confirmar que ninguno introduce
  una race condition, un leak, o un UB nuevo.

### Simplicidad

- **No sobre-ingeniería.** La solución más simple que resuelve el problema correctamente
  es la buena. La complejidad extra encarece el mantenimiento, reduce la legibilidad y
  degrada el rendimiento (más indirecciones, más locks, más allocations).
- **Eliminar código muerto.** Variables, flags, wrappers triviales o condiciones que
  nunca pueden tomar un valor determinado se eliminan; no se dejan "por si acaso".
- **Wrappers de una línea no aportan.** Una función que solo reenvía a otra del mismo
  objeto sin añadir lógica es ruido. Llamar directamente.

### Comentarios

- **Solo comentarios que aporten comprensión real.** Un comentario que repite lo que el
  código ya dice claramente no debe existir. Un comentario sí es necesario cuando:
  - Explica un invariante no obvio que el lector necesita conocer para razonar sobre el
    código (p.ej. "EPOLLONESHOT garantiza que…").
  - Documenta una decisión de diseño con alternatives descartadas.
  - Advierte de una trampa o un comportamiento del sistema operativo / hardware.
- **Mantener los comentarios sincronizados.** Si se cambia el diseño, los comentarios
  que lo describían se actualizan en el mismo cambio. Documentación obsoleta es peor
  que no tener documentación.

---

## 2. Arquitectura de la capa HTTP/1.1

Organización de directorios (dentro de `include/`):

```
protocol/
  deserialization.h            → tipos genéricos: deserialization_status,
								  channel_intent, deserialization_result<RQty>
  serialization.h              → contrato genérico de salida: serialization_result { prefix, source }
  common/
    byte_storage.h             → almacenamiento genérico de bytes: memoria + spill opcional a fichero temporal
    reader.h / writer.h         → helpers move-only genéricos sobre byte_storage
  http11/
  request.h                  → deserializador principal (single-pass) + registro de dispatchers
  connection.h               → estado hop-by-hop de la conexión (mutable)
  context.h                  → estado transversal cross-header (MOVIDO aquí desde interpreters/rules)
  policies.h                 → límites de política inbound (read-only)
  parsed_types.h             → structs de datos parseados neutrales (parsed_host_port, etc.)
  verdict.h                  → enum class verdict { kAccept, kReject }
  helpers.h                  → parsing común reusable (iequals, consume_token, ABNF helpers…)
  headers.h / target.h
  body/
    encoder.h / decoder.h     → contratos de encode/decode, estados, errores y resultados
    encoder_raw.h             → encoder identidad (raw): sin framing adicional
    encoder_chunked.h         → encoder chunked: framing de salida
    decoder_raw.h             → decoder identidad (raw): lectura de payload sin transformación
    decoder_chunked.h         → decoder chunked: máquina de estados de entrada
    serializer.h              → orchestrador de escritura; usa encoders y common::writer
    deserializer.h            → orchestrador de lectura; usa decoders y common::reader; expone
                                get_body_deserializer() como const con estado interno mutable
  checkers/headers/*.h       → verificadores SINTÁCTICOS (1 fichero por header)
  interpreters/
	headers/*.h              → intérpretes SEMÁNTICOS intra-header (1 por header modelado)
	rules/                   → reglas SEMÁNTICAS transversales cross-header
	  directives.h           → (antes connection.h) reglas de Connection option tokens
	  framing.h              → Content-Length vs Transfer-Encoding
	  routing.h              → Host / request-target authority
	  policy.h               → agregados de política (p.ej. suma de forwarding hops)
transport/
  server/tcpip_windows.h       → transporte TCP/IP genérico (WinSock/IOCP), consume solo channel_intent
  server/tcpip_linux.h         → transporte TCP/IP genérico (epoll/EPOLLET+EPOLLONESHOT), misma semántica que Windows
  server/tcpip.h               → selector de plataforma (incluye Windows o Linux según _WIN32/__linux__)
```

### Flujo de `request::deserialize` (una sola pasada — zero-copy)

1. Parsea request-line (method, request-target en origin/absolute/authority/asterisk-form,
   HTTP-version — solo acepta `HTTP/1.1`). El componente **query** del request-target se
   preserva crudo (`query_part_` / `get_query()`) y, además, se descompone en pares
   **clave/valor** (`application/x-www-form-urlencoded`) que se materializan en la `request`
   (`query_parameters_` / `get_query_parameter(i)` / `get_query_parameters_length()`).
2. Recorre headers **una vez**. Para cada header:
   - Extrae `field_name` (SIN el `:`) y `field_value` (con `ows_trim`).
   - Busca en `header_dispatchers_` (mapa `common::hash_map`, case-insensitive, O(1),
	 function pointers sin `std::function`).
   - El dispatcher **parsea el valor una única vez** y, si el header está **modelado**,
	 ejecuta su intérprete intra-header, registrando señales cross-header en `ctx`.
3. Al terminar los headers, aplica las **4 reglas transversales inline** sobre el `context`
   totalmente poblado:

   ```cpp
   framing::apply(ctx) → routing::apply(ctx) → directives::apply(ctx) → policy::apply(ctx)
   ```

   Cualquier `kReject` ⇒ `deserialization_status::kInvalidSource`.
4. Traduce el estado de `connection` al **`channel_intent`** genérico:

   ```cpp
   channel = connection_tmp.close_requested ? kClose : kKeep;   // kUpgrade reservado para fase futura
   ```
5. Devuelve `deserialization_result(req, bytes_used, channel)`.

### Contrato transporte ↔ protocolo

`channel_intent { kKeep, kClose, kUpgrade }` es el **único** vocabulario que ve el transporte.
El transporte (`tcpip_windows.h`) usa `result.channel` para decidir cerrar tras enviar la
respuesta pendiente o rearmar otro receive. **No conoce nada de HTTP/1.1.**

---

## 3. Decisiones de diseño clave (y por qué)

1. **Una sola pasada de parsing.** El rendimiento manda: nunca se parsea un header dos veces.
   Por eso los checkers modelados exponen un **único overload productor**
   `check(sv, parsed_T&)` (se eliminaron los overloads `check(sv)` puros para evitar
   ambigüedad y forzar 1 parseo). El `dispatch<CHty>` genérico — reservado a headers **no
   modelados** — sigue llamando a `CHty::check(sv)` de un argumento; esta coherencia está
   ahora **blindada en compilación** con un `static_assert` que exige
   `CHty::check(std::string_view) -> bool`, de modo que un checker con dependencia de
   contexto nunca puede cablearse por error a este bridge.
2. **Dispatchers como function pointers** (no `std::function`): registro de cero-alocación,
   lookup O(1), llamadas inlinables. Hay **14 dispatchers dedicados** para headers modelados
   + `dispatch<CHty>` para el resto.
3. **Separación sintáctica/semántica en carpetas espejo:** `checkers/headers/*` ↔
   `interpreters/headers/*`, más `interpreters/rules/*` para lo transversal.
4. **`context` (antes `interpretation_context`)** agrupa la vista agregada que necesitan las
   reglas: referencia mutable a `connection`, referencia const a `policies`, y señales
   cross-header que **no son hop-by-hop** (flags de presencia + pocos valores parseados a
   comparar). Todos los `string_view` son zero-copy sobre el buffer de la request (que
   sobrevive porque las reglas corren antes de que `deserialize` retorne).
5. **`context.h` se movió** a `protocol/http11/` (mismo nivel que `request.h`) — no tenía
   sentido bajo `interpreters/rules/`.
6. **`interpreters/rules/connection.h` → `directives.h`** para evitar dos ficheros
   `connection.h` indistinguibles (ya existe `protocol/http11/connection.h`). La clase pasó
   de `connection` a `directives`.
7. **Se eliminó `request_interpreter`** (el antiguo coordinador semántico). Agrupar 4 reglas
   en una clase era excesivo; ahora las reglas se aplican explícitamente inline en
   `deserialize`.
8. **Helpers de bytes genéricos y serialización protocol-agnóstica.** El caller elige solo el
   framing HTTP (`raw`/`chunked`); los serializers/deserializers usan `common::writer`/
   `common::reader` sobre `common::byte_storage`, que empieza en memoria y derrama a fichero
   temporal cuando supera `common::byte_storage_options::spill_threshold`. La transición es
   automática y transparente. El transporte recibe únicamente
   `protocol::serialization_result { prefix, source }`, sin conocer HTTP/1.1 ni sus encoders.
    - `body::serializer<ENty>`: interfaz de escritura de alto nivel. `write(string_view)`/
      `write(span<byte>)` son encadenables; `finish()` cierra el encoder (añade el chunk final
     `0\r\n\r\n` en chunked); `release_reader()` llama a `finish()` internamente y devuelve un
      `common::reader` listo para consumir. `response` recibe directamente los serializers raw o
      chunked para preservar su framing. Factories: `raw(opts)` y `chunked(opts)`.
    - `body::deserializer<DEty>`: interfaz de lectura de alto nivel, complemento del reader.
      `get_body_deserializer()` es `const` (el estado del decoder es `mutable`). Factories:
     `raw(opts, content_length)` y `chunked(opts)`. Wiring en `request::deserialize()`:
     construye `body::deserializer<body::decoder_chunked>` si `connection_tmp.chunked`, o
     `body::deserializer<body::decoder_raw>` con `max_raw_size = content_length` si
     `ctx.has_content_length && content_length > 0`; de lo contrario, sin body. El result
     vive en `mutable std::optional<body_deserializer_t>` (alias público
     `request::body_deserializer_t = std::variant<body::deserializer<body::decoder_raw>,
     body::deserializer<body::decoder_chunked>>`); `has_body()` y `get_body_deserializer()` lo exponen.
    - **Wiring en `response`:** `set_body(body::serializer<body::encoder_raw>&&)` y
      `set_body(body::serializer<body::encoder_chunked>&&)` conservan el framing correcto.
      `serialize()` devuelve `protocol::serialization_result`; el transporte posee su `prefix`
      y consume su `common::reader source` en segmentos acotados.

### Estado de datos (referencia rápida)

- **`connection`** (hop-by-hop, mutable): `persistent`, `close_requested`, `transfer_codings`,
  `chunked`, `te_codings`, `accepts_trailers`, `trailer_names`, `upgrade_offer`,
  `connection_options`.
- **`policies`** (read-only): `max_content_length`, `max_forwarding_hops`,
  `max_transfer_codings`, `allow_chunked`, `allow_upgrade`.
- **`context`**: `conn&`, `pol&`, `has_content_length`/`multiple_content_length`/
  `content_length`, `has_transfer_encoding`, `has_host`/`multiple_host`/`host`,
  `has_target_authority`/`target_authority`, `forwarding_hops`.
- **`parsed_types.h`**: `parsed_host_port`, `parsed_token_list`, `parsed_parameter_list`,
  `parsed_scalar`, `parsed_via_element`/`parsed_via_list`, `parsed_forwarded_pair`/
  `parsed_forwarded_element`/`parsed_forwarded_list`, `parsed_host_port_list`.
- **`request` (datos materializados, propios y durables)**: `method_`, `absolute_path_`,
  `query_part_` (query crudo) **+ `query_parameters_` (pares clave/valor del query,
  `application/x-www-form-urlencoded`)**, `host_`/`host_port_`/`host_type_`,
   `target_authority_*`, `headers_`, **`body_` (`std::optional<body_deserializer_t>`, presente
  cuando el request lleva body — raw con `max_raw_size=content_length`, o chunked)**.

  > ⚠️ **Corrección (dejó de existir):** este documento mencionaba antes un segundo modelo
  > `request_other` (variante *fixed-buffer* que "espejaba" `request` con offsets `(off,len)`).
  > Verificado por listado de directorio: **ese fichero no existe** en
  > `include/protocol/http11/` — solo hay un modelo de `request` (el de arriba). Referencia
  > obsoleta eliminada; no hay dos modelos que mantener en paralelo.

---

## 4. Estado actual (✅ hecho)

- **66 headers** con checker sintáctico implementado — el "HTTP/1.1 SERVER HEADER CHECKLIST"
  en `request.h` está **completo, todos `[x]`** (RFC 9110 + CORS + Sec-WebSocket-* +
  Via/Forwarded/X-Forwarded-* + Keep-Alive + Proxy-Connection).
- **14 headers modelados** con intérprete intra-header + dispatcher dedicado: Host,
  Content-Length, Transfer-Encoding, Connection, TE, Trailer, Expect, Upgrade, Max-Forwards,
  Via, Forwarded, X-Forwarded-For, X-Forwarded-Host, X-Forwarded-Proto.
- Reglas transversales: `framing`, `routing`, `directives`, `policy` — creadas, compilando,
  aplicadas inline.
- `channel_intent` integrado en `deserialization_result`; transporte migrado a
  `result.channel`.
- Renombrado `interpretation_context` → `context`; movido `context.h`; renombrado
  `connection.h` → `directives.h` (clase `directives`).
- Eliminado `request_interpreter`.
- **Bug corregido:** `field_name` incluía por error el `:` → rompía el lookup de dispatchers
  (Host nunca se procesaba y `routing::apply` rechazaba). Corregido con
  `sv.substr(fn_start, i - 1 - fn_start)`. Bloque de dispatch que se había comentado
  temporalmente: restaurado.
- Harness de test `test/ut-001-main/src/main.cpp` actualizado para usar
  `deserialized_02.channel`.
- **Build tree recuperado.** Ambos presets (`msvc-debug`, `msvc-release`) compilan y enlazan
  limpio tras regenerar bajo el Developer Environment completo de VS (`VsDevCmd.bat
  -arch=x64 -host_arch=x64`); la contaminación anterior era de entorno (mezcla x86/x64), no
  de código.
- **`ctx.has_target_authority` / `ctx.target_authority` poblados.** `helpers::
  try_to_deserialize_as_authority_form` y `helpers::try_to_deserialize_as_absolute_form` se
  extendieron con out-params (`uri_host`, `port`, `host_type`, y `has_authority` en
  absolute-form) que exponen la autoridad ya parseada sin re-parseo; `request::deserialize`
  la captura zero-copy en `ctx.target_authority` cuando el request-target la lleva
  (authority-form vía CONNECT, o absolute-form con `//authority`). `routing::apply` ya tenía
  la comparación Host ↔ authority (RFC 9112 §3.2.2/§3.3); ahora opera con datos reales para
  las 5 formas de request-target.
- **Ciclo cerrado: Host / request-target authority materializados en `request` y
  consumidos por `server.h`.** `request` ahora posee copias propias (`std::string`) de
  `Host` (`host_`/`host_port_`/`host_type_`, getters `get_host()`/`get_host_port()`/
  `get_host_type()`/`has_host()`) y de la request-target authority
  (`target_authority_host_`/`_port_`/`_type_`, getters `get_target_authority_host()`/
  `get_target_authority_port()`/`get_target_authority_type()`/`has_target_authority()`),
  copiadas desde `ctx.host`/`ctx.target_authority` en el momento de construir el `request`
  (mismo patrón que `method_`/`absolute_path_`/`query_part_`) — sobreviven a `deserialize()`.
  Con esos datos ya durables, `server.h` dejó de tener los dos `case` de
  `target::kAuthorityForm`/`target::kAsteriskForm` sin `on_send` (bug: dejaban la conexión
  colgada, a diferencia de todos los demás `case` del switch). Ahora: `kAuthorityForm`
  (CONNECT) responde `501 Not Implemented` — el tunneling en sí sigue sin implementarse, pero
  ya no cuelga la respuesta, y la autoridad parseada queda accesible vía
  `req->get_target_authority_host()`/`_port()` para cuando se aborde; `kAsteriskForm`
  (`OPTIONS *`) responde `200 OK` reconociendo la petición server-wide sin enrutarla.
- **Auditoría de `request`/`deserialize` (excluyendo body) — completada.** Revisión completa
  de `request.h`, `helpers.h` (parsing de las 5 formas de request-target, ABNF de URI/host),
  `context.h` y las 4 reglas transversales (`framing`, `routing`, `directives`, `policy`) en
  busca de bugs, problemas de seguridad y discrepancias con el RFC:
  - **🐛 Bug de seguridad corregido: Content-Length duplicado no se detectaba (vector
    CL.CL de request smuggling).** A diferencia de Host (`ctx.multiple_host`), un segundo
    header `Content-Length` sobreescribía en silencio a `ctx.content_length` sin rechazo,
    incluso con valores discrepantes — violación directa de RFC 9112 §6.3 bullet 4
    ("a recipient MUST treat multiple Content-Length header fields... as an unrecoverable
    error"). Corregido con el mismo patrón que `multiple_host`: nuevo campo
    `ctx.multiple_content_length` (poblado en `dispatch_content_length`, igual que
    `dispatch_host` puebla `multiple_host`) y nuevo chequeo en `framing::apply` que rechaza
    incondicionalmente cuando hay más de un Content-Length, sin importar si los valores
    coinciden. Ambos presets (`msvc-debug`, `msvc-release`) recompilados y enlazados limpio
    tras el fix.
  - **Resto de la auditoría: sin hallazgos adicionales.** El lookup de `header_dispatchers_`
    (`common::hash_map` con `base_hash`/`base_equal`) es case-insensitive ASCII, por lo que no
    existe bypass de header modelado vía variación de mayúsculas/minúsculas (p.ej.
    `content-length` en vez de `Content-Length` sigue pasando por `dispatch_content_length`).
    Las comparaciones semánticas relevantes para smuggling (`chunked` como coding final,
    TE+CL simultáneos, opciones de `Connection`, Host vs authority) ya usan `helpers::iequals`
    de forma consistente. El parsing de authority-form/absolute-form/origin-form valida
    pct-encoded (2 hex digits), rechaza caracteres fuera de la gramática ABNF de RFC 3986, y
    distingue correctamente IP-literal (`[...]`) de reg-name/IPv4 al localizar el separador
    `:` puerto/host. No se encontraron discrepancias adicionales con el RFC en el código
    revisado.
  - **Conclusión: `request` se considera completa** salvo por el trabajo de body/framing ya
    diferido explícitamente (ver §5 OBLIGATORIO #2). Las decisiones antes anotadas como deuda no
    bloqueante (coherencia de `dispatch<CHty>` y comparación de puerto en `routing.h`) han
    quedado **resueltas** — ver §4.
- **Auditoría de seguridad de memoria de `response.h` (excluyendo body) — completada.**
  `response` es un serializador **zero-alloc** sobre un buffer fijo (`char memory_[8192]`,
  body reservado en `[8192-4096, 8192)`); status-line + headers crecen desde el offset 0 y
  `serialize()` compacta el body justo tras el bloque core. Las constantes
  `status_lines::kXXX` **ya incluyen su `\r\n`** (macro `SL(x)` en `status_lines.h`), por lo
  que el CRLF que añade `serialize()` es el separador de fin de headers (no un duplicado).
  Correcciones aplicadas (ambos presets recompilados limpio):
  - **🐛 `serialize()` devolvía longitud incorrecta (crítico).** `return memory_;` convertía el
    `char[8192]` a `string_view` midiendo con `strlen`, no con la longitud real
    (`end_of_core + bdy_len_`); los consumidores reales hacen `out->append(res->serialize())`
    en `tcpip_windows.h` (líneas ~480 y ~588), por lo que un body con byte `NUL` se **truncaba**
    y la longitud enviada era errónea. Ahora devuelve `std::string_view(memory_, end_of_core +
    bdy_len_)`.
  - **🐛 Escrituras sin bounds-check (OOB write).** `serialize()` escribía el CRLF de cierre y
    hacía `memmove` del body sin verificar límites; `add_header` reservaba solo los bytes del
    propio header (podía llenar hasta `bdy_beg_` exacto y dejar a `serialize()` sin hueco para
    el CRLF terminador); `sln()` copiaba el status-line sin comprobar tamaño. Añadidas guardas
    defensivas "si no cabe, no-op seguro" en las tres (`serialize` reserva `crlf_bytes` contra
    `bdy_beg_` y comprueba `end_of_core+bdy_len_ ≤ kMaxSizeInMemory`; `add_header` reserva
    además los 2 bytes del CRLF de cierre; `sln` valida `len ≤ bdy_beg_`).
- **`response`: gestión de headers implementada (`set_header`, `get_header`, `has_header`,
  `get_headers_length`, `remove_header`) — contrato (excluyendo body) CERRADO.**
  Los headers viven serializados en `memory_[sln_len_ .. sln_len_+hdr_len_)` como líneas
  `key:value\r\n` consecutivas (no hay índice). Se añadió un único helper privado
  `find_header(k, line_off, val_off, val_len, line_len)` que escanea ese bloque y localiza la
  primera coincidencia **case-insensitive**, reutilizado por los métodos de lectura/escritura:
  - `set_header(k, v)`: si el header existe reemplaza el valor **in situ** (reubica el resto del
    bloque con `memmove`, actualiza `hdr_len_`, con guarda de capacidad al crecer que reserva el
    CRLF de cierre); si no existe **delega en `add_header`**. Con sobrecarga aritmética
    `set_header(key, const T&)`.
  - `get_header(std::string_view key)` (por clave) y `get_header(std::size_t index)` (por índice,
    0-based) **devuelven `std::pair<std::string, std::string>` (K, V) por copia** (owned, NO
    aliasan `memory_`, por lo que sobreviven a mutaciones/`serialize()`). Contrato de error por
    ausencia: `get_header(key)` lanza `std::runtime_error("header not found!")`;
    `get_header(index)` lanza `std::out_of_range("header index out of range!")`.
    ⚠️ *Cambio de API:* antes eran `bool get_header(..., std::string_view&)` (out-params view). Se
    migró a par + excepciones a petición para evitar exponer aliases mutables del buffer interno.
  - `has_header(std::string_view key)` → `bool`: permite **probar** presencia sin capturar la
    excepción de `get_header`.
  - `get_headers_length()` → `std::size_t`: cuenta los headers para **iterar por índice de forma
    segura** con `get_header(index)` sin depender de capturar `std::out_of_range` (mismo patrón que
    `request::get_headers_length`).
  - `remove_header(k)`: compacta el bloque con `memmove` y reduce `hdr_len_`; encadenable. Eliminar
    un header **ausente es un no-op intencional (idempotente)** — a diferencia de `get_header`, NO
    lanza. Documentado en el propio método.
  - Se añadieron helpers privados `tolower_ascii`/`iequals` **locales** (réplica ASCII de
    `helpers::iequals`) para **no acoplar** `response.h` a `helpers.h` (solo incluye
    `status_lines.h`, más `<stdexcept>`/`<utility>`). Punto único a tocar si en el futuro se
    normaliza OWS alrededor del valor: `find_header`.
- **`response`: política de errores por falta de espacio UNIFICADA (excepciones).** Todos los
  puntos donde se intenta escribir pero no hay espacio físico en el buffer fijo lanzan ahora
  `std::out_of_range` (antes eran bail-outs silenciosos `return *this;`/`return {};`):
  `serialize()` (CRLF de cierre y compactación del body), `add_header`, `set_header` (al crecer el
  valor), `set_body` y `sln()` (status-line). Las sobrecargas aritméticas (`add_header<T>`,
  `set_header<T>`, `set_body<T>`) heredan la política vía delegación. `serialize()` mantiene las
  guardas `crlf_bytes` contra `bdy_beg_` y `end_of_core+bdy_len_ ≤ kMaxSizeInMemory`.
- **Parámetros de query (`query_parameters_`) poblados y expuestos en `deserialize`.** Antes
  `query_parameters_` existía como miembro privado en `request.h` pero **nunca se poblaba ni
  se exponía**. Ahora el componente query del
  request-target (ya extraído crudo por `helpers::try_to_deserialize_as_query`, sin el `?`
  inicial) se descompone en pares clave/valor siguiendo la convención
  `application/x-www-form-urlencoded`:
  - **Helper centralizado** `helpers::split_query_parameters(query, out_keys, out_values,
    max_pairs)` (`public static constexpr`): separa pares por `&`, usa el **primer `=`** de cada
    par como separador clave/valor, un par sin `=` produce **valor vacío**, y los pares vacíos
    (`&&`, `&` inicial/final) se omiten. Emite vistas **zero-copy** sobre el query de entrada,
    escribe como mucho `max_pairs` entradas y devuelve el conteo (respeta la política de
    capacidad fija ya usada para headers). Los checkers/parsers no duplican esta lógica (regla
    de los HTTP header checkers: parsing común y reusable centralizado en `helpers.h`).
  - **`request.h`** (modelo *owned*): tras materializar `query_part_`, si el query no está vacío
    llama al helper sobre arrays locales de `string_view` y **copia** cada par a entradas
    `std::string` en `query_parameters_` (`std::vector<std::pair<std::string,std::string>>`), de
    modo que sobreviven a `deserialize()` con independencia del buffer de origen (mismo patrón de
    propiedad que `method_`/`absolute_path_`/`query_part_`). Getters nuevos:
    `get_query_parameter(i)` y `get_query_parameters_length()`. La constante privada
    `kMaxQueryParameters = 128` solo dimensiona los arrays de parseo transitorios (el `vector` es
    no acotado; el exceso se descarta).
  - **El query crudo se conserva intacto** (`get_query()` / `query_part_`) — la nueva estructura
    es aditiva. Validado compilando y enlazando `ut-001-main` (preset `msvc-debug`) con un uso
    temporal de los nuevos getters (query de prueba `q=doba&lang=es&flag&empty=`), revertido
    tras la validación.
    - **Coherencia de `dispatch<CHty>` blindada (cierre de deuda documentada).** El bridge
      genérico de checkers no modelados incorpora ahora un `static_assert` en `request.h` que
      exige `CHty::check(std::string_view) -> bool`. La auditoría confirmó
      que la incoherencia sospechada **no existía** (los checkers no modelados exponen un
      `check(sv)` válido de un argumento; los modelados usan sus dispatchers dedicados); el
      `static_assert` fija el invariante en compilación para que un checker con dependencia de
      contexto no pueda cablearse por error a este template. Cambio sin impacto en runtime.
    - **Comparación de puerto en `routing.h` normalizada por scheme (cierre de deuda
      documentada).** `try_to_deserialize_as_absolute_form` (en `helpers.h`) captura ahora el
      **scheme** del request-target absolute-form como out-param zero-copy; el deserializador
      lo guarda en `ctx.target_authority.scheme` (nuevo campo en `parsed_host_port`). Se añadieron
      `helpers::default_port_for_scheme` (`http`→`80`, `https`→`443`, case-insensitive) y
      `helpers::ports_equivalent(scheme, a, b)`, y `routing::apply` sustituyó el `!=` de string crudo
      por `ports_equivalent(...)`. Así `Host: example.com` reconcilia con
      `http://example.com:80/...` (default del scheme ≡ puerto omitido) mientras `:8080` vs `` sigue
      siendo *mismatch*. La authority-form (CONNECT) no transporta scheme → el campo queda vacío y la
      comparación degrada a igualdad exacta, sin cambio de comportamiento. El scheme es zero-copy
      sobre `sv` y lo consume `routing` **antes** de materializar. Validado compilando y enlazando
      `ut-001-main` (preset `msvc-debug`, toolchain MSVC x64).
    - **Fuga de memoria corregida en `receive`/`send` si `WSARecv`/`WSASend` fallaban
      síncronamente (cierre de deuda documentada).**
      `send()` (`WSASend`) creaban el `OVERLAPPED` (`overlapped_receive`/`overlapped_send`) con
      `new` y, ante un error real (`SOCKET_ERROR` con código distinto de `WSA_IO_PENDING`), hacían
      `return false;` sin `delete` del objeto — el `delete` normal solo ocurre en
      `handle_overlapped(ovb)`, que nunca se ejecuta si la operación falla en el acto (IOCP no
      llega a encolarla). Corregido añadiendo `delete ovr;`/`delete ovs;` antes de cada
      `return false;`, replicando el patrón ya usado en `post_accept()` (`delete ova;` ante fallo
      síncrono de `AcceptEx`). Ambos presets (`msvc-debug`, `msvc-release`) recompilados y
      enlazados limpio tras el fix.
    - **`handle_error(ovb)`/`handle_overlapped(ovb)` protegidos contra `ovb == nullptr` (cierre
      de deuda documentada).**
      `GetQueuedCompletionStatus` devolvía `FALSE` se llamaba incondicionalmente a
      `handle_error(ovb)` y `handle_overlapped(ovb)`, ambas con `ovb->get_type()` — si `lpo ==
      NULL` (la propia espera del IOCP falla sin haber dequeue-ado ningún packet, p.ej. handle de
      puerto inválido), `ovb` es `nullptr` y la desreferencia era **UB**. Corregido: la rama
      `else` ahora distingue `ovb != nullptr` (operación concreta fallida — comportamiento sin
      cambios, sigue liberándose vía `handle_overlapped`) de `ovb == nullptr` (nada que
      inspeccionar ni liberar); en este último caso se marca `stopping = true` para que ese
      worker termine su bucle con limpieza en vez de arriesgar un bucle ocupado si el puerto deja
      de ser utilizable fuera de la secuencia controlada de `stop()`. La llamada final a
      `handle_overlapped(ovb)` pasa a condicionarse también a `ovb != nullptr`. Ambos presets
      (`msvc-debug`, `msvc-release`) recompilados y enlazados limpio tras el fix.
    - **Comentarios placeholder `pepe`/`pepe fin` eliminados de `stop()` (limpieza de código).**
      Las dos ramas de error de `stop()` (`PostQueuedCompletionStatus` y `CloseHandle`) tenían
      bloques de comentario vacíos a modo de placeholder. Sustituidos por manejo real: en el fallo
      de `PostQueuedCompletionStatus`, se libera con `delete` el `overlapped_stop` recién creado
      (que de otro modo quedaría sin encolar y sin liberar, ya que solo `handle_overlapped` lo
      hace) y se documenta que el worker afectado igualmente terminará su bucle limpio gracias al
      guard `ovb == nullptr` del punto anterior; en el fallo de `CloseHandle` se documenta que no
      hay recuperación posible y que `io_h_` queda de todos modos consistente (se resetea a
      `nullptr` justo después). Ambas ramas añaden `assert(false && "...")` (sin efecto en
      release, igual que `assert_sending_held()` ya usado en este mismo archivo) para hacer
      visible el fallo en debug sin usar `throw` (inapropiado aquí porque `stop()` se invoca desde
      `~tcpip()`). Ambos presets (`msvc-debug`, `msvc-release`) recompilados y enlazados limpio
      tras el fix.
    - **Eliminados los `#ifndef NDEBUG`/`#endif` de `tcpip_windows.h` (limpieza de código, a
      petición explícita del usuario).** `context` tenía 4 bloques de compilación condicional
      manual envolviendo `sending_owner_dbg`, el cuerpo de `sending_guard` y `assert_sending_held()`
      para "desactivarlos" en release. Es redundante: `assert()` (de `<cassert>`) ya se convierte en
      no-op cuando `NDEBUG` está definido, que es exactamente el mismo mecanismo ya usado (sin
      `#ifdef`) por los `assert(false && "...")` añadidos en `stop()` más arriba. Se quitaron los 4
      bloques `#ifndef NDEBUG`/`#endif` dejando el `store` atómico de `sending_owner_dbg` y la
      llamada a `assert(...)` incondicionales en el código fuente; en builds release el `store` se
      sigue ejecutando (coste insignificante frente al `std::lock_guard` ya tomado ahí mismo) pero
      `assert_sending_held()` no comprueba nada porque su `assert(...)` se compila a nada. Sin
      cambio de comportamiento observable en ningún preset. Ambos presets (`msvc-debug`,
      `msvc-release`) recompilados y enlazados limpio tras el fix.
    - **`protocol::http11::server` parametrizado por `RQty`/`RSty` (petición explícita del
      usuario: hacer el servidor plenamente configurable).** Antes `server` solo aceptaba `TRty`
      (transporte) como parámetro de plantilla y usaba `protocol::http11::request`/`response` de
      forma hardcodeada (`router_fn`, los lambdas `on_request`/`on_bad_request` cableados en el
      constructor, y `TRty<request, response, 4096> transport_`). El transporte
      (`transport::server::tcpip<RQty, RSty, BFsz>`) y `transport::server::types::*_delegate<...>`
      ya eran genéricos sobre `RQty`/`RSty`; `server` era la única capa que forzaba los tipos
      concretos. Ahora `server` es
      `template <typename RQty = request, typename RSty = response, template<typename, typename,
      std::size_t> class TRty = transport::server::tcpip> class server`, con **valores por
      defecto** que preservan `server srv;` sin argumentos (usado en `test/ut-001-main/src/main.cpp`)
      sin cambios. `router_fn`, los lambdas del constructor y `transport_` ahora usan `RQty`/`RSty`
      en vez de los tipos concretos. `target` (el enum de formas de request-target) se mantiene sin
      tocar, ya que es un concepto propio del namespace `http11` y no de `request`. El contrato
      duck-typed que un `RQty`/`RSty` custom debe cumplir (métodos como `get_target()`,
      `get_method()`, `get_absolute_path()`, `ok_200()`/`not_found_404()`/`bad_request_400()`/
      `not_implemented_501()`/`set_body()`, y `static deserialize(...)` para `RQty`) queda **sin
      formalizar como `concept` de C++20 todavía** — eso se abordará en un cambio posterior,
      explícitamente diferido por decisión del usuario en favor de mejorar la legibilidad de los
      errores de plantilla. Ambos presets (`msvc-debug`, `msvc-release`) recompilados y enlazados
      limpio tras el fix.
    - **`protocol::http11::server` también parametrizado por `ROty` (router), continuación
      inmediata del punto anterior (petición explícita del usuario).** `router<router_fn>` estaba
      hardcodeado tanto en el tipo del atributo `router_` como en `router<router_fn>::data` (el tipo
      de retorno de `match`, envuelto en `std::optional` dentro del lambda `on_request`). Ahora
      `server` añade un cuarto parámetro de plantilla,
      `template<typename> class ROty = router`, con **valor por defecto** que preserva `server srv;`
      sin argumentos (`test/ut-001-main/src/main.cpp`) sin cambios. Al pasar de `router` (nombre
      concreto) a `ROty` (parámetro de plantilla), `ROty<router_fn>::data` se convirtió en un
      *dependent name*, por lo que requirió anteponer `typename` (mismo patrón ya usado en este
      repo: `typename context::sending_guard` en `tcpip_windows.h`, `typename DEty::RequestHandler`
      en `tcpip_linux.h`). `ROty` se añadió al final de la lista de parámetros (tras `TRty`) en vez
      de junto a `RQty`/`RSty`, ya que ningún punto del repo instancia `server<...>` explícitamente
      hoy, así que no había un orden preexistente que romper. Ambos presets (`msvc-debug`,
      `msvc-release`) recompilados y enlazados limpio tras el fix.
    - **`sending_owner_dbg` (owner-tracking de debug) eliminado de `tcpip_windows.h` (limpieza de
      código, a petición explícita del usuario: "no lo veo necesario").** `context` guardaba un
      `std::atomic<std::thread::id> sending_owner_dbg` que `sending_guard` escribía en su
      constructor y reseteaba en su destructor, solo para que `assert_sending_held()` (invocada
      desde `push_pending_response()`/`drain_pending_responses()`) comprobara que el hilo llamante
      sostenía `sending_mutex`. Los 6 puntos de uso de `sending_guard` (`assign_sequence()`,
      `deposit_and_drain()`, el cierre por `kClose` en `handle_receive`,
      `send_error_and_mark_context_for_closing()`, `arm_pending_send_operation()` y
      `arm_next_send_operation()`) ya toman el guard antes de tocar `responses`/`reorder`, así que
      el assert nunca podía dispararse en la práctica: era defensa en profundidad sin protección
      real, a costa de un `store` atómico en cada adquisición/liberación del guard. Se eliminaron
      `sending_owner_dbg`, el cuerpo de `assert_sending_held()` y sus dos llamadas; `sending_guard`
      queda como un RAII directo sobre `sending_mutex` (solo `std::lock_guard<std::mutex> lock_`).
      Sin cambio de comportamiento observable en ningún preset. Ambos presets (`msvc-debug`,
      `msvc-release`) recompilados y enlazados limpio tras el fix.
    - **Bytes parciales de `WSARecv` ya no se pierden — buffer único por conexión
      (`tcpip_windows.h`, OBLIGATORIO #1 cerrado — diseño final).** `handle_receive` hacía
      `break` al recibir `kMoreBytesNeeded` con `bytes_used == 0` y rearmaba un nuevo
      `overlapped_receive` con buffer vacío, descartando el residuo. El fix definitivo mueve
      el estado de recepción directamente a `context` (donde pertenece, dado que solo existe
      una `WSARecv` activa simultánea por conexión): se añaden `CHAR recv_buf[BFsz]` y
      `DWORD recv_off{0}` a `context`; `overlapped_receive` queda reducido a `WSABUF wsa`
      únicamente (sin buffer propio ni offset). `receive()` no acepta parámetros de residuo:
      apunta `wsa.buf = ctx->recv_buf + ctx->recv_off` y `wsa.len = BFsz - ctx->recv_off`.
      `arm_next_receive_operation()` tampoco recibe parámetros de residuo: sigue siendo un
      wrapper simple sobre `receive(ctx)`. En `handle_receive` el límite del bucle pasa de
      `bytes_received` a `total = ctx->recv_off + bytes_received`; al salir por
      `kMoreBytesNeeded` se calcula `m = total - oin`, se cierra defensivamente si `m == BFsz`
      (petición demasiado grande para el buffer), y en caso contrario hace
      `memmove(ctx->recv_buf, ctx->recv_buf + oin, m)` + `ctx->recv_off = m` antes de llamar
      a `arm_next_receive_operation(ctx)`. Al terminar el batch sin residuo, `ctx->recv_off`
      se resetea a 0. No hay `memcpy` entre overlapped objects ni parámetros opcionales en
      ninguna función. Ambos presets (`msvc-debug`, `msvc-release`) recompilados y enlazados
      limpio tras el fix.

- **Módulo de body HTTP/1.1 — arquitectura final cerrada (`common/byte_storage.h` +
  `common/reader.h` + `common/writer.h` + `protocol/serialization.h` + `http11/body/` +
  wiring en `request`/`response`).**
  - **Contratos separados** en `body/encoder.h` y `body/decoder.h`: estados, errores y
    resultados de cada dirección; `decoder.h` declara además `kDefaultBufferSize`.
  - **Storage unificado genérico** en `common/byte_storage.h`: único backend que empieza en memoria
    (`std::string`) y derrama a fichero temporal cuando supera
    `common::byte_storage_options::spill_threshold`. La
    transición es transparente para codecs y callers. El fichero se elimina en el destructor.
    `common::reader` y `common::writer` son wrappers move-only sin semántica HTTP.
    Todos los tamaños, límites, offsets y contadores de bytes del pipeline usan
    `std::size_t`; `std::uint64_t` queda reservado para valores que no representan tamaños.
  - **Encoders** (`ENty`): `body/encoder_raw.h` (identidad) y
    `body/encoder_chunked.h` (framing chunked de salida). **Decoders** (`DEty`):
    `body/decoder_raw.h` (identidad) y `body/decoder_chunked.h` (máquina de estados de entrada
    por pasos `step_*`, sin `goto`). No conocen almacenamiento.
  - **`body::serializer<ENty>`** (alto nivel, escritura): `write(string_view)` /
    `write(span<byte>)` encadenables; `finish()` cierra el codec (añade `0\r\n\r\n` en chunked);
    `release_reader()` llama a `finish()` internamente y devuelve `common::reader` listo.
    Factories: `raw(opts)` / `chunked(opts)`.
  - **`body::deserializer<DEty>`** (alto nivel, lectura): `get_body_deserializer() const`
    (estado del codec `mutable`). Factories: `raw(opts, content_length)` / `chunked(opts)`.

  **Wiring en `request`:** `request::deserialize()` construye automáticamente el deserializador
  correcto: `body::deserializer<body::decoder_chunked>` si `connection_tmp.chunked`,
  `body::deserializer<body::decoder_raw>` con `max_raw_size = content_length` si
  `ctx.has_content_length && content_length > 0`, o ninguno. Vive en
  `mutable std::optional<body_deserializer_t>` (alias público
  `request::body_deserializer_t = std::variant<body::deserializer<body::decoder_raw>,
  body::deserializer<body::decoder_chunked>>`). API pública: `has_body()`, `get_body_deserializer()`.

  **Wiring en `response`:** las sobrecargas explícitas para serializers raw/chunked preservan el
  framing HTTP. `serialize()` devuelve un `protocol::serialization_result` propietario: `prefix`
  contiene status-line, headers y body inline; `source` contiene un `common::reader` opcional para
  streaming. Windows y Linux mantienen ese resultado hasta enviar el prefijo y consumir el source
  en segmentos acotados. Ambos presets (`msvc-debug`, `msvc-release`) compilados y enlazados limpio.

### Convenciones a respetar al continuar

- **🚫 Nunca `assert()` ni `#ifdef`/`#ifndef` de modo de compilación (debug/release) para
  verificar condiciones en tiempo de ejecución (regla de oro, inviolable — indicación
  explícita del usuario).** No se insertará `assert(...)` como mecanismo para verificar
  invariantes o condiciones en runtime, ni compilación condicional atada al modo de build
  (`#ifdef DEBUG`, `#ifndef NDEBUG`, o equivalentes) para activar/desactivar ese tipo de
  comprobaciones. **Motivo:** estas técnicas no sirven para lo que más preocupa — los bugs
  de **race-condition**, los más difíciles de atajar, casi nunca se manifiestan en binarios
  DEBUG (sin optimizar, con timing/scheduling de hilos distinto al de producción), así que
  un `assert` ahí da una falsa sensación de seguridad; y en RELEASE (donde el bug sí puede
  aparecer bajo carga y concurrencia real) `assert()` se compila a no-op porque `NDEBUG`
  está definido, así que tampoco protege donde de verdad importa. Si una condición en
  runtime necesita verificarse de verdad, la comprobación debe ser **incondicional y activa
  en todas las configuraciones** (debug y release) — manejo de error real, `throw`, log +
  cierre controlado, etc. — nunca detrás de un mecanismo que desaparece según el modo de
  build. **Nota:** esto matiza los puntos "Eliminados los `#ifndef NDEBUG`" y
  "`sending_owner_dbg` eliminado" de la sección 4 — quitar el `#ifdef` redundante alrededor
  de `assert()` siguió siendo correcto en su momento, pero de cara a trabajo futuro **no se
  debe seguir apoyando la verificación en `assert()` en sí**. Los dos
  `assert(false && "...")` que quedan en `stop()` (fallos de
  `PostQueuedCompletionStatus`/`CloseHandle`) son anteriores a esta convención y quedan
  pendientes de revisar bajo esta regla; no se tocan en este cambio.
- **Planes ≠ implementación (regla de flujo de trabajo).** Cuando se pide *un plan*
  (p.ej. "crea un plan", "diseña cómo haríamos X", "evalúa el enfoque"), el resultado es
  **solo el plan**: NO se debe tocar ni escribir código. Tras presentar el plan hay que
  **esperar la instrucción directa y explícita del usuario** para acometerlo (o no). No se
  asume aprobación por silencio ni se encadena la ejecución al acto de planificar. Solo se
  implementa cuando el usuario lo pide de forma inequívoca.
- Mismo **estilo, formato y reglas** que los checkers existentes (cabeceras de caja ASCII
  `+---+`, secciones `[>]`, licencia Apache 2.0).
- **HTTP Header Checkers:** centralizar parsing común/reusable en `helpers.h` como
  `public static constexpr`; los checkers delegan en `helpers::` en vez de duplicar lógica.
- En `request.h`: los `#include` de `checkers/headers/*` **y** las entradas del mapa
  `header_checkers_`/dispatchers deben seguir **el orden del "HTTP/1.1 SERVER HEADER
  CHECKLIST"**, no orden alfabético. Al añadir un header nuevo, colócalo en su posición del
  checklist y marca la fila `[x]`.
- Mantener **agnóstico** el transporte y el contrato genérico (regla de oro).

---

## 5. 🚦 Producción: qué falta para que el servidor HTTP/1.1 sea *production-ready*

> Análisis hecho por lectura directa del código: `server.h`, `tcpip_windows.h`,
> `tcpip_linux.h`, `tcpip.h`, `thread_pool.h`, `request.h`, `policies.h`, `framing.h`, `policy.h`,
> `connection.h`, `router.h`, `response.h`, `date_server.h`. La sección OBLIGATORIO recoge los
> bloqueantes reales para producción; la sección OPCIONAL los elementos diferibles o de evolución
> futura. No hay backlog separado — todo el trabajo pendiente vive aquí.

### 🔴 OBLIGATORIO (bloqueante — orden de mayor a menor importancia)

1. ✅ **Soporte de body HTTP/1.1 — arquitectura final cerrada (`common/byte_storage.h` +
   `common/reader.h` + `common/writer.h` + `protocol/serialization.h` + `http11/body/` +
   codecs + serializers/deserializers + wiring en `request`/`response`).** Storage automático y
   genérico (sin inyección por plantilla); codecs intercambiables como único parámetro de plantilla
   (`ENty`/`DEty`). La API de alto nivel construye/consume bodies mediante serializers/deserializers; el
   transporte recibe `protocol::serialization_result` y consume un `common::reader` sin conocer
   HTTP/1.1. Ambos presets (`msvc-debug`, `msvc-release`) compilados y enlazados limpio. — Ver §4.
2. **Una excepción de un handler `kAsync` termina el proceso completo.** El worker de
   `thread_pool.h` ejecuta `task();` (línea 109) sin `try/catch`. La ruta síncrona sí está
   protegida (el `try/catch` de `tcpip_windows.h` ~L621-666 envuelve la llamada a `on_request`,
   que para `kSync` ejecuta `router_entry->first(*req, *res)` inline); pero para `kAsync`,
   `server.h` (~L131-135) solo hace `thread_pool_->enqueue(...)`, que retorna de inmediato — la
   ejecución real del handler ocurre después, ya fuera de ese `try/catch`. Una excepción no
   capturada ahí escapa del hilo worker sin handler → `std::terminate()`. Se derriba **todo el
   servidor** (todas las conexiones), no solo la que la originó.
3. **Recursos sin límite: cola async y conexiones concurrentes.** `thread_pool_::tasks_`
   (`std::queue<std::function<void()>>`, línea 159) no tiene tope; `enqueue()` (~L141-152) solo
   comprueba `running_`, nunca un tamaño máximo. En paralelo, `server.h` cuenta `connections_`
   (`std::atomic<uint32_t>`, ~L167-168 y 209) pero nunca lo usa como cap para rechazar/limitar
   nuevas conexiones. Ambos son vectores directos de agotamiento de memoria/handles bajo carga o
   ataque.
4. **Sin timeout de conexión inactiva (idle/read timeout).** Nada en `tcpip_windows.h` cierra una
   conexión aceptada que no envía datos (o que los gotea a propósito): vector clásico
   *slow-loris*. Sin esto, un cliente colgado o malicioso retiene contexto y buffer
   indefinidamente.
5. **`policies.max_content_length` declarado pero nunca aplicado.** El campo existe en
   `policies.h`, pero ni `framing.h` ni `policy.h` (los dos puntos donde se aplican límites
   agregados hoy) lo comprueban contra el `Content-Length` real. Depende del punto 1 (no tiene
   sentido limitar un body que aún no se consume), pero debe implementarse en el mismo esfuerzo
   para no reabrir esa capa dos veces.
6. **Sin respuesta ante excepción de negocio, e incluso pérdida de respuestas ya listas del
   mismo batch.** Hoy, si un handler `kSync` lanza, `tcpip_windows.h` captura la excepción pero
   solo hace `mark_context_for_closing` (~L661-665) y `return` inmediato: la conexión se cierra
   **sin enviar ninguna respuesta**, y si esa request formaba parte de un batch pipelined con
   requests anteriores ya resueltas y depositadas (`deposit_and_drain`) pero aún no volcadas
   (`batch` todavía `true`), esas respuestas ya calculadas también se pierden porque el `return`
   se produce antes del flush de fin de batch. Debería, como mínimo, responder
   `500 Internal Server Error` para la request que falló y flushear lo ya depositado antes de
   cerrar. Va de la mano del punto 2 (que resuelve el caso `kAsync`).
7. ✅ **`next_seq_to_assign` — no es un data race (cerrado).** IOCP garantiza que solo un
   worker toca el receive path de cada conexión a la vez (solo hay una `WSARecv` activa por
   conexión; la siguiente solo se arma al final de `handle_receive`), exactamente la misma
   garantía que `EPOLLONESHOT` da en Linux. Por eso el backend Linux dejó de usar el atómico
   en la misma sesión en que se analizó este punto — no era necesario. `next_seq_to_assign`
   es `std::uint64_t` ordinario en ambos transportes; no requiere protección adicional.
8. **El router compara rutas y métodos case-insensitive.** `router.h` reutiliza
   `common::hash_map`, cuyo `base_equal` normaliza ASCII a minúsculas. Por tanto `/Users` y
   `/users`, o `GET` y `get`, se consideran equivalentes. Las rutas HTTP son case-sensitive y los
   métodos deben respetar su token literal; esta normalización puede seleccionar un handler
   incorrecto o eludir una política de autorización basada en rutas.
9. **Inyección de headers de respuesta.** `response::add_header`/`set_header` copian key/value
   sin validar la gramática de `field-name` ni rechazar `CR`/`LF` en el valor. Un handler que
   inserte datos no confiables en un header (por ejemplo `Location`) puede producir response
   splitting. Centralizar una validación siempre activa antes de escribir al buffer de respuesta.
10. **Excepción async: el proceso sobrevive, pero queda una request sin respuesta.**
    `thread_pool` ya absorbe excepciones de una tarea para impedir `std::terminate`, pero el lambda
    async de `server.h` solo invoca `on_send(res)` después de que el handler retorne. Si lanza, no
    se deposita respuesta; en una conexión pipelined puede bloquear la ventana de reordenamiento y
    acabar cerrando el canal. Definir un contrato de error: generar una respuesta terminal (p.ej.
    500) o cerrar el contexto de forma coordinada liberando su secuencia.
11. **`stop()` no cierra explícitamente las conexiones activas.** Windows cierra solo el socket de
    escucha antes de parar workers; Linux hace lo mismo y después cierra epoll. No existe un
    registro de contextos activos que permita cerrar sus sockets, cancelar/liberar overlapped o
    tokens pendientes y notificar ordenadamente el shutdown. Bajo procesos de larga vida esto deja
    un shutdown incompleto y puede retener recursos hasta que el proceso termine.

### 🟡 OPCIONAL (se puede vivir con ello en producción — ir añadiendo poco a poco)

1. **TLS/HTTPS.** No lo exige HTTP/1.1 en sí mismo; el patrón habitual es terminarlo en un proxy
   delante (nginx, ALB, etc.). Sube de prioridad solo si el plan es exponer este servidor
   directamente a Internet sin nada delante.
2. **IPv6.** Hoy `tcpip_windows.h` solo usa `AF_INET` (líneas 379 y 751 — creación de socket y
   `sockaddr_in`).
3. **`SO_REUSEADDR`/equivalente en el socket de escucha.** Facilita reinicios rápidos sin esperar
   el `TIME_WAIT` del puerto.
4. **Observabilidad (logging/métricas).** No hay ningún punto de instrumentación en los caminos
   de error de `tcpip_windows.h`/`thread_pool.h`; hoy los fallos solo se ven vía `assert` (debug)
   o cierre silencioso de conexión.
5. **`channel_intent::kUpgrade`** (WebSocket, etc.) Cablear la ruta de upgrade cuando se
   soporte; hoy `deserialize` solo emite `kKeep`/`kClose`. Es una preocupación **distinta** del
   ítem OPCIONAL #6 (CONNECT): el upgrade cambia de protocolo sobre la misma conexión ya aceptada;
   CONNECT requiere abrir una conexión nueva hacia otro destino.
6. **Tunneling CONNECT completo.** Hoy `server.h` responde `501 Not Implemented` para CONNECT
   porque completar el túnel exige: (a) abrir una conexión TCP *saliente* hacia la target authority
   ya materializada en `request` (`get_target_authority_host()`/`get_target_authority_port()`),
   (b) responder `200 Connection Established` solo si esa conexión saliente tiene éxito, y (c)
   actuar de relay bidireccional de bytes crudos entre el socket aceptado y el saliente. Se
   resolverá cuando exista el módulo cliente de Doba (dial-out): `server` delegará en él para abrir
   la conexión saliente y gestionar el relay, sin que el transporte de servidor (`tcpip_windows.h`,
   accept-only vía IOCP) necesite conocer nada de CONNECT ni de conexiones salientes.
7. ✅ **Transporte Linux implementado y refactorizado (`tcpip_linux.h`).** Backend epoll completo
   con semántica equivalente al transporte Windows. Detalles del diseño final:
   - **`EPOLLET` + `EPOLLONESHOT` por conexión** (socket de escucha incluido). Un único fd epoll
     compartido por todos los workers; `epoll_wait` con timeout de 200 ms para chequeo periódico
     del stop-token. `kEpollMaxEvents = 128` eventos por llamada.
   - **`fd_token`** (heap, `epoll_event.data.ptr`): tag de tipo fuerte para distinguir el
     listening socket de los sockets de conexión sin necesidad de comparar fds. Lifetime gestionado
     por `handle_accept` (alloc) y `mark_context_for_closing` (dealloc tras `epoll_ctl(DEL)`).
   - **`context`** (por conexión, `shared_ptr`): contiene `recv_buf[BFsz]` + `recv_off` (carry-over
     de recepción, campo `off` — nombre explícito del proyecto), ventana de reordenamiento
     `reorder[kReorderWindow]` (`kReorderWindow = 256`), `send_pending_` +
     `send_pending_off_` (envíos parciales), y los delegates `on_request`/`on_bad_request`.
   - **Carry-over de recepción**: al salir del bucle de `handle_receive` por `kMoreBytesNeeded`,
     los bytes residuales se mueven al inicio de `recv_buf` con `memmove` y se actualiza
     `recv_off`; el siguiente `recv()` apunta a `recv_buf + recv_off`. Si el residuo llena el
     buffer completo se cierra la conexión (petición demasiado grande).
   - **Envíos parciales**: si `send()` devuelve menos bytes que el buffer, el tail se almacena en
     `send_pending_` (offset `send_pending_off_`) y el fd se re-arma con `EPOLLOUT`;
     `handle_send_resume` continúa el envío cuando el socket vuelve a estar disponible.
   - **Pipelining y reordenamiento**: `deposit_and_drain` usa un array circular de
     `kReorderWindow` slots; `deposit_result::kOverflow` cierra la conexión si la distancia de
     secuencia supera la ventana.
   - **Shutdown cooperativo**: workers son `std::jthread`; `stop()` llama a
     `w.request_stop()` en todos y luego `workers_.clear()` los joinea. No hay IOCP ni
     `PostQueuedCompletionStatus` — el mecanismo de parada es puramente el stop-token de
     `jthread`.
   - **`send_error_and_mark_context_for_closing`**: path de error compartido por todos los
     puntos de `handle_receive` que deben responder con un error HTTP antes de cerrar.
   El selector de plataforma `tcpip.h` incluye automáticamente el backend correcto.
8. **Funcionalidades avanzadas de body** (el soporte básico del punto OBLIGATORIO #1 ya está
   cerrado — ver §4): trailers declarados en el request (wiring `connection.accepts_trailers`
   → `common::reader`), `100-continue` end-to-end (intercepción en transporte antes de leer el
   body), y streaming a disco para bodies grandes (el spill automático a fichero ya está
   implementado en `common::byte_storage`; queda cablear `spill_threshold` desde las `policies`
   del servidor para que sea configurable end-to-end).
9. **Parámetros de tuning configurables en runtime/compile-time** (p.ej. `kReorderWindow`, tamaño
   de buffer `BFsz`, tamaño del pool) — evolución futura ya planeada, no requisito actual.
10. **Persistencia de estado de conexión entre requests y lifetime de `std::string_view`.** Si los
    datos de conexión deben sobrevivir a `deserialize()` (p.ej. para enrutar por `Host` en
    requests pipelined), los campos de `connection` que hoy son `string_view` zero-copy sobre el
    buffer del request actual dejarán de ser válidos en el siguiente ciclo. Requiere decidir qué
    estado hop-by-hop debe materializarse como `std::string` (siguiendo el patrón
    `method_`/`absolute_path_`/`host_`) y qué puede seguir siendo efímero.
11. **Señales semánticas expuestas pero no consumibles por handlers.** Tras `deserialize()`,
    `connection` porta `te_codings`, `accepts_trailers`, `trailer_names`, `connection_options` y
    `upgrade_offer`; `context` porta `forwarding_hops`. Hoy estas señales se validan pero no se
    exponen al handler vía `request`. Si un handler necesita saber si el cliente acepta trailers,
    cuántos hops ha recorrido la request, o qué upgrade se ha ofrecido, no tiene forma de
    obtenerlo. Decidir qué señales deben materializarse en `request` (con sus getters) y cuáles
    son solo internas al ciclo de deserialización.
12. **Limpieza de documentación interna** (`tcpip_windows.h`, `request.h`, et al.): verificar que
    no queden comentarios obsoletos que aún describan el modelo "coordinator" o referencias a
    decisiones de diseño ya superadas. Tarea de mantenimiento de bajo riesgo.
13. **Reemplazar los `assert(false && "...")` de `stop()` por manejo de error real
    (`tcpip_windows.h`, fallos de `PostQueuedCompletionStatus`/`CloseHandle`).** Ambos
    quedan en tensión con la nueva convención de la sección 4 ("Nunca `assert()` ni
    `#ifdef`/`#ifndef` de modo de compilación para verificar condiciones en tiempo de
    ejecución"): en release son no-op y en debug no reproducen el escenario de concurrencia
    real donde estos fallos importarían. Pendiente decidir el reemplazo (log + continuar,
    código de error propagado, etc.) sin usar `assert` ni compilación condicional por modo
    de build.
14. **Tests/UT** — diferido por decisión explícita del usuario. Cobertura mínima que debería
    añadirse cuando se retome:
    - **`response` — serialización y headers (contrato cerrado, excl. body):** `serialize()`
      con/sin headers y con body (incl. verificación de la longitud exacta devuelta y de body con
      byte `NUL`); `add_header`/`set_header` (rama "existe" → reemplazo in situ con crecer/encoger
      el valor, y rama "no existe" → append); `get_header` por clave y por índice **devolviendo par
      (K,V)** (incl. `std::runtime_error` en clave ausente y `std::out_of_range` en índice fuera de
      rango, y match case-insensitive); `has_header` (true/false); `get_headers_length` (conteo y
      recorrido indexado seguro); `remove_header` (compactación **y no-op idempotente** en key
      ausente); y la **política uniforme de `std::out_of_range`** por falta de espacio en
      `serialize`/`add_header`/`set_header`/`set_body`/`sln`.
    - **`request` — deserialización:** casos de las 5 formas de request-target, y regresión del
      smuggling CL.CL (Content-Length duplicado → rechazo) y TE+CL / `chunked` no final.
      Incluir también **routing por puerto normalizado**: `Host: example.com` (sin puerto) debe
      reconciliar con `http://example.com:80/...` (default del scheme), y `:8080` vs `` sigue
      siendo *mismatch*.
    *(No hay tests nuevos escritos todavía; este ítem solo registra la deuda.)*
15. **`date_server::current()` devuelve una vista con lifetime no garantizado.** El método expone
    un `std::string_view` a uno de dos buffers rotativos. Un consumidor puede retener la vista
    mientras el escritor, dos ciclos después, sobrescribe ese mismo buffer; la publicación atómica
    del puntero no extiende la vida de los caracteres ni evita esa carrera. Devolver una copia
    owned o redefinir explícitamente un contrato de consumo inmediato con almacenamiento estable.
16. **Getters públicos indexados sin chequeo de rango en `request`.** Los accesores de query/header
    por índice usan `operator[]` directamente. Un consumidor que pase un índice inválido incurre
    en UB en lugar de recibir un error controlado. Aplicar la misma política pública de
    `std::out_of_range` ya documentada para accesores equivalentes de `response`.

### 🧩 Perspectiva API: qué necesita un consumidor para desplegar doba en producción

> Los bloques de arriba miran la robustez **interna** (transporte, hilos, framing). Esta
> subsección mira lo mismo desde el lado del **consumidor de la librería**: alguien que hace
> `#include "protocol/http11/server.h"` y quiere operar un servicio real. Fundamentado en
> lectura directa de `server.h`, `router.h`, `policies.h`, `request.h` (wiring de `policies_tmp`),
> `date_server.h` y `tcpip_windows.h` (bind address). No repite ítems ya trackeados arriba.

**🔴 OBLIGATORIO (API):**

1. **`policies` no es configurable desde la API pública — no hay wiring end-to-end.**
   `request::deserialize` es `static` y siempre instancia `policies_tmp` en pila
   (`request.h` ~L241) con los valores por defecto (todo "unlimited"). No existe ningún
   parámetro, setter, ni builder en `server`/`transport_`/`request` para que un consumidor fije
   `max_content_length`, `max_forwarding_hops`, `max_transfer_codings`, `allow_chunked` o
   `allow_upgrade`. Aunque se resuelva el punto OBLIGATORIO #6 de arriba (aplicar
   `max_content_length` en `framing`/`policy`), el valor comprobado seguiría siendo siempre el
   default permisivo — falta el mecanismo para que la configuración del consumidor llegue hasta
   ahí. Bloquea cualquier despliegue expuesto que necesite límites de ingestión propios.
2. **No hay forma de elegir la interfaz de bind.** `server::start(const char port[])` solo acepta
   puerto; `tcpip_windows.h` (~L394) siempre usa `INADDR_ANY`. Un consumidor no puede bindear a
   `127.0.0.1` ni a una interfaz interna concreta — el servidor siempre escucha en todas las
   interfaces, indeseable en varios escenarios de producción (p.ej. bind solo a una NIC privada
   detrás de un load balancer).
3. **Rutas sin parámetros ni comodines.** `router::match` (`router.h` ~L117-125) hace lookup
   exacto del `path` completo contra un `hash_map`; no hay segmentos variables (`/users/{id}`),
   prefijos ni wildcards. Cualquier API REST con recursos identificados por id no se puede
   expresar sin trocear el path a mano dentro del handler. Es una limitación central de
   expresividad para usar doba como servidor de una API HTTP típica.
4. **`add_route` con colisión `(method, route)` falla en silencio.** `router::add` (`router.h`
   ~L109-113) hace `map.emplace(route, ...)` y descarta el resultado; si la key ya existe,
   `emplace` no inserta y no informa. Registrar dos veces el mismo endpoint (error de
   programación, o un módulo que registra encima de otro) deja activo el primer handler sin
   ningún diagnóstico — puede enmascarar bugs de configuración en producción.
5. **Sin punto de extensión para el cuerpo de una respuesta de error.** Relacionado con el punto
   OBLIGATORIO #7 de arriba (pérdida de respuesta ante excepción): incluso resuelto a nivel
   transporte, la API no ofrece al consumidor personalizar el formato del `500` (p.ej. JSON de
   error uniforme, correlación con un request-id) — hoy no es un punto de extensión.

**🟡 DESEABLE (API):**

1. **`Date` no se añade nunca a las respuestas pese a existir `date_server`.** Confirmado por
   grep sobre todo `include/`: `date_server::get().current()` no se invoca en ningún sitio; solo
   se llama `.start()` (`server.h` ~L182). RFC 9110 §6.6.1 recomienda que un servidor de origen
   incluya siempre `Date`. Hoy el consumidor tendría que añadirlo a mano en cada handler, y para
   hacerlo tendría que incluir `common/date_server.h` directamente, rompiendo la capa de
   abstracción del protocolo.
2. **Sin middleware/hooks globales (logging, CORS, auth, headers de seguridad).** Cada ruta es un
   `std::function` aislado; no hay ningún punto de extensión transversal. Cualquier
   funcionalidad cross-cutting debe repetirse a mano en cada handler.
3. **Un único puerto/listener por instancia `server`.** No hay forma de escuchar en más de un
   puerto (p.ej. HTTP de negocio + puerto de métricas/healthcheck) con la misma API sin
   instanciar `server`s completos por separado, cada uno con su propio `router_`/`thread_pool_`.
4. **Tuning fijo en tiempo de compilación/arranque.** El alias de plantilla de `server` fija
   `BFsz=4096` (`TRty<RQty, RSty, 4096>`, `server.h` ~L210) sin exponerlo como parámetro de
   `server`; el pool siempre usa `std::thread::hardware_concurrency()`. Ya trackeado como punto
   OPCIONAL #9 de arriba a nivel interno, pero aplica igual desde la óptica de API: un consumidor
   no puede ajustar estos valores sin tocar el código fuente de doba.
5. **Sin introspección de rutas ni healthcheck de fábrica.** `router_` es privado y sin getter en
   `server`: no hay forma de enumerar rutas registradas, ni un endpoint de salud provisto por el
   framework — cada consumidor debe montarlo a mano como una ruta más.

### 📋 Resumen para estimación (¿cuándo estamos *alive*?)

> Auditoría confirmatoria de segunda pasada (lectura íntegra de `hash_map.h`, `context.h`,
> `parsed_types.h`, `response.h`, `date_server.h`, `router.h`, y el bloque de bind/listen de
> `tcpip_windows.h` ~L379-403/751): **no han aparecido bloqueantes nuevos** más allá de los ya
> listados arriba. De los 12 puntos OBLIGATORIO originales (7 transporte + 5 API), el #1 de
> transporte (soporte de body) quedó **cerrado** con la implementación completa de
> `body_serializer`/`body_deserializer` (más `common::writer`/`common::reader`/
> `common::byte_storage`/codecs),
> y el OPCIONAL #7 (transporte Linux) quedó **cerrado** con el refactor completo de
> `tcpip_linux.h`; el antiguo OBLIGATORIO #7 (`next_seq_to_assign` data race) quedó **cerrado**
> tras análisis confirmatorio (IOCP serializa el receive path igual que EPOLLONESHOT) —
> quedan **10 bloqueantes** (5 transporte + 5 API). El trabajo pendiente
> completo (bloqueante y diferible) vive en este §5.

- **Bloqueantes reales: 10** (5 transporte + 5 API). Se agrupan así por causa raíz/dependencia,
  para poder secuenciar el esfuerzo de estimación en bloques en vez de 11 ítems sueltos:
  - **Framing por-conexión** (transporte #5 — `max_content_length` sin aplicar): ahora es el
    único ítem de este grupo; el soporte de body (antiguo transporte #1) ya está cerrado.
    *(Los bytes parciales de `WSARecv`, el body básico y el transporte Linux ya están cerrados
    — ver §4 y OPCIONAL #7.)*
  - **Contención de fallos de handler** (transporte #2 + #6): excepción `kAsync` derriba el
    proceso; excepción `kSync` pierde la respuesta (y las de un batch pipelined ya listas). Es
    el mismo problema de fondo (falta manejo de error end-to-end) en dos rutas de código
    distintas (async vs sync).
  - **Superficie de configuración pública** (API #1 + #2): `policies` y la interfaz de bind no
    llegan desde la API pública — misma clase de hueco (falta de wiring consumidor → transporte)
    en dos mecanismos distintos.
  - **Independientes** (no comparten causa con otros): recursos sin límite (transporte #3),
    idle timeout (transporte #4), router case-insensitive (transporte #8), inyección de headers
    (transporte #9), rutas exactas + colisión silenciosa (API #3 y #4, relacionados entre sí
    vía `router.h` pero no con el resto), y sin hook de error custom (API #5).
- **No bloqueantes: 17** (12 transporte OPCIONAL + 5 API DESEABLE) — se pueden ir incorporando
  de forma incremental sin impedir un primer despliegue en producción.

No se han encontrado ficheros ni rutas de código adicionales con hallazgos fuera de lo ya
documentado arriba; la auditoría confirmatoria se da por **cerrada**.
