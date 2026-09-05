# Backlog

[Indice](HANDOFF.md)

Fuente de verdad para el trabajo pendiente de doba. Los identificadores
historicos se conservan aunque cambie la categoria: P5 pertenece a QA y
C2-C4 son conveniencia opcional. La evidencia de los elementos cerrados
se conserva en [QUALITY.md](QUALITY.md).

Un item pendiente no implica que su diseno este decidido. Los criterios
siguientes sirven para preparar su implementacion; las decisiones abiertas
se indican expresamente. Las prioridades sin valor previo figuran como
"Sin fijar". Las etiquetas originales B/M/A estimaban complejidad y no se
reinterpretan como severidad.

## Contenido

- [Objetivo de release](#objetivo-de-release)
- [Inventario](#inventario)
- [Hardening operativo](#hardening-operativo)
- [Producto y conveniencia](#producto-y-conveniencia)
- [Calidad y validacion](#calidad-y-validacion)
- [Release engineering](#release-engineering)
- [Mantenibilidad C++](#mantenibilidad-c)
- [Documentacion publica](#documentacion-publica)
- [Fuera de la primera release](#fuera-de-la-primera-release)

## Objetivo de release

El objetivo vigente de `0.1.0-beta.1` es completar C1 y C5 y verificarlos
sobre sockets reales en Windows y Linux. La salida requiere tambien conservar
las validaciones RE1-RE3 y completar la parte restante de RE4.

Orden recomendado:

1. C1: timeout unico de inactividad.
2. C5: maximo global de conexiones activas.
3. Cerrar RE4 y verificar toda la matriz de release.

C1 y C5 comparten configuracion generica previa a `start()` y ejecucion
especifica en IOCP y epoll. Conviene definir esa parte comun antes de abordar
cada cambio localizado. No se han elegido aun valores predeterminados ni la
forma exacta de la API.

R2, fuzzing y automatizacion de herramientas externas estan expresamente
diferidos fuera del hardening actual. C2-C4 no son gates de compliance ni
release. DT4 y DT5 tampoco bloquean automaticamente la primera release.
Los demas pendientes no tienen una version asignada.

## Inventario

| Item | Categoria | Estado | Prioridad | Objetivo |
| --- | --- | --- | --- | --- |
| [C1](#c1-timeout-unico-de-inactividad) | Hardening | Pendiente | Objetivo beta | 0.1.0-beta.1 |
| [C5](#c5-maximo-global-de-conexiones-activas) | Hardening | Pendiente | Objetivo beta | 0.1.0-beta.1 |
| [R2](#r2-limites-efectivos-por-request) | Hardening | Diferido | Alta | Sin version |
| [C2](#c2-condicionales-y-rangos-automaticos) | Producto | Diferido | Sin fijar | Sin version |
| [C3](#c3-trailers-de-salida) | Producto | Diferido | Sin fijar | Sin version |
| [C4](#c4-options-automatico-de-recurso) | Producto | Diferido | Sin fijar | Sin version |
| [P1](#p1-handler-de-ficheros-estaticos) | Producto | Pendiente | Sin fijar | Sin version |
| [P2](#p2-logging-de-acceso) | Producto | Pendiente | Sin fijar | Sin version |
| [P3](#p3-cadena-de-middleware) | Producto | Pendiente | Sin fijar | Sin version |
| [P4](#p4-parsing-de-formularios) | Producto | Pendiente | Sin fijar | Sin version |
| [P5](#p5-suite-de-conformidad-exhaustiva) | QA | Pendiente | Sin fijar | Sin version |
| [QA3](#qa3-fuzzing) | QA | Diferido | Alta | Sin version |
| [QA4](#qa4-baseline-de-rendimiento) | QA | Pendiente | Media | Sin version |
| [QA5](#qa5-diagnostico-y-aislamiento-del-harness) | QA | Pendiente | Media | Sin version |
| [RE4](#re4-gobierno-y-trazabilidad-de-release) | Release | Parcial | Media | 0.1.0-beta.1 |
| [DT4](#dt4-dependencias-y-efectos-globales-de-platformh) | C++ | Pendiente | Media | Sin version |
| [DT5](#dt5-contrato-de-getters-indexados) | C++ | Pendiente | Baja/Media | Sin version |

Los pendientes sin identificador historico se detallan en
[QA adicional](#validacion-adicional), [documentacion](#documentacion-publica)
y [capacidades futuras](#fuera-de-la-primera-release). Permanecen fuera de
la numeracion y de los lotes de la primera release.

## Hardening operativo

### C1: Timeout unico de inactividad

**Contexto.** Ningun backend cierra hoy una conexion abierta que deja de
progresar. Estimacion original de complejidad: M.

**Alcance.** Un unico `inactivity_timeout`, configurable antes de `start()`,
aplicable a lectura, keep-alive y escritura. Recibir o enviar bytes renueva
el plazo; la duracion total de una conexion o request no lo consume mientras
exista progreso. Al vencer, el transporte cierra de forma segura.

**Componentes.** Configuracion del transporte, `tcpip_windows.h`,
`tcpip_linux.h` y tests de integracion TCP/IP.

**Aceptacion y pruebas.**

- Una lectura fragmentada que progresa conserva la conexion.
- Una request parcial detenida, un keep-alive inactivo y un cliente que no
  lee provocan cierre al vencer el plazo aplicable.
- Comprobar plazo, lifetime, orden y un unico callback por cierre.
- Ejecutar escenarios equivalentes en IOCP y epoll.

**Dependencias y decisiones.** Compartir con C5 la configuracion generica;
determinar API, valor por defecto, posible desactivacion y tolerancia temporal
de los tests antes de implementar.

**Fuera de alcance.** Plazos separados por fase, duracion maxima absoluta,
configuracion dinamica y generacion automatica de respuestas `408`.

### C5: Maximo global de conexiones activas

**Contexto.** `connections_` es observacional y no limita la admision.
Estimacion original de complejidad: M.

**Alcance.** Un maximo global configurado antes de `start()`. Cada backend
reserva el cupo atomicamente antes de admitir un contexto. Si se alcanza el
limite, cierra inmediatamente la nueva conexion y conserva las ya admitidas.

**Componentes.** Configuracion generica, admision y cierre en ambos backends,
contador de conexiones y tests TCP/IP.

**Aceptacion y pruebas.**

- Superar el maximo con clientes concurrentes sin sobrepasar el cupo.
- Las conexiones ya admitidas siguen atendidas.
- Cada cierre libera exactamente una reserva, tambien ante errores.
- Una nueva conexion puede entrar al recuperarse el cupo.
- Verificar equivalencia en Windows y Linux.

**Dependencias y decisiones.** Coordinacion con C1; determinar defaults y
semantica de un eventual valor ilimitado. C5 controla conexiones; R2 controla
recursos por request.

**Fuera de alcance.** Cuotas por worker, cambios dinamicos, backpressure de
aceptacion, callbacks nuevos y respuestas HTTP de rechazo.

### R2: Limites efectivos por request

**Contexto.** `max_content_length`, `max_forwarding_hops`,
`max_transfer_codings`, `max_uri_length` y `max_header_section_size` usan
cero como ilimitado. El servidor por defecto no ofrece configuracion.
El buffer interno acota el head, pero no proporciona una politica general
de recursos. Los bodies pueden derramar a disco.

**Alcance futuro.** Definir defaults seguros y una API minima previa a
`start()`. Rechazar un Content-Length excesivo antes de consumir el body.
Verificar por separado la politica aplicable a un body chunked, cuyo tamano
no se conoce de antemano.

**Componentes.** `policies.h`, `limits.h`, `context.h`, `decoder.h`,
reglas de headers, composicion del servidor y sus tests.

**Aceptacion y pruebas.**

- Documentar que limita cada valor y como se aplica en el servidor estandar.
- Probar el valor exacto, una unidad por encima y la semantica de cero.
- Comprobar rechazo temprano y razon de rechazo correspondiente.
- Verificar que la configuracion no cambia de forma insegura durante uso.
- Medir si el cambio afecta al hot path.

**Dependencias y decisiones.** Requiere diseno propio de API y defaults.
Se ha diferido expresamente; el plan actual no cambia estas politicas ni sus
consumidores. No confundirlo con C5.

## Producto y conveniencia

### C2: Condicionales y rangos automaticos

**Contexto.** Doba conserva campos condicionales; el handler dispone de los
validadores del recurso. La aplicacion, cuando actua como origin server,
es responsable de evaluar precondiciones. Los rangos son opcionales.

**Alcance futuro.** Helpers para `304`, `412`, `206` o `416`, si se
decide ofrecerlos. El framework no debe inventar metadatos de la
representacion seleccionada.

**Componentes.** API HTTP, headers condicionales, response y ejemplos.

**Aceptacion y pruebas.** Definir el contrato de validadores aportados por la
aplicacion, probar precedencia de precondiciones y conservar la posibilidad
de ignorar Range y atender el GET normal.

**Referencias.** RFC 9110 S13.2, S13.2.2 y S14.

**Fuera de alcance.** No es un gate de compliance del core ni de release.
La correccion de fechas invalidas esta cerrada y documentada en calidad.

### C3: Trailers de salida

**Contexto.** `response` no expone una API para emitir trailers.

**Alcance futuro.** Definir una API limitada al framing que los admite,
con validacion de campos y finalizacion coherente del body.

**Componentes.** Response, writers de salida y sus tests.

**Aceptacion y pruebas.** Verificar el wire completo, el terminador y las
restricciones de campos de trailers; conservar la salida sin trailers.

**Referencia.** RFC 9110 S6.5.

**Fuera de alcance.** Capacidad opcional, sin gate de release asignado.

### C4: OPTIONS automatico de recurso

**Contexto.** `OPTIONS *` ya es automatico. La aplicacion puede registrar
handlers `OPTIONS` para recursos concretos.

**Alcance futuro.** Sintetizar respuestas desde el router reutilizando el
calculo de `Allow` empleado por `405`.

**Componentes.** Router, server HTTP y pruebas de rutas.

**Aceptacion y pruebas.** Conservar prioridades de rutas estaticas,
parametrizadas y wildcard; definir la precedencia de un handler explicito y
probar recursos existentes y ausentes.

**Fuera de alcance.** Conveniencia opcional; no es gate de release.

### P1: Handler de ficheros estaticos

**Contexto.** El drenaje de bodies de salida y el percent-decoding ya existen.
Este ultimo es una dependencia de seguridad. Estimacion original: M.

**Alcance por definir.** Un handler de ficheros que use esas primitivas.
`TransmitFile`/`sendfile` son opciones a evaluar con medidas, no un
requisito arquitectonico decidido.

**Componentes.** Handler HTTP, resolucion de paths, body de salida y ejemplos.

**Aceptacion propuesta.** Definir la raiz permitida, errores de acceso y
lifetime del fichero; verificar paths codificados, traversal, ficheros
inexistentes, cuerpo binario y salida grande con memoria acotada.

**Dependencias.** Preservar la frontera generica si se introduce una ruta de
envio especifica de plataforma. No presupone soporte automatico de rangos.

### P2: Logging de acceso

**Contexto.** No existe un punto dedicado a logging de acceso. El transporte
tiene callbacks de ciclo de vida y el logger comun no equivale a un registro
de requests.

**Alcance por definir.** Decidir el punto de observacion y los datos expuestos,
incluyendo cuando una respuesta puede considerarse terminada.

**Componentes.** Composicion HTTP/transporte, logger, tests y ejemplos.

**Aceptacion propuesta.** Comprobar registros de exito, rechazo, desconexion y
fallo de envio sin duplicados; definir lifetime de datos y coste cuando el
logging esta desactivado.

**Dependencias.** Decidir si necesita P3; no introducir esa dependencia por
defecto ni exponer semantica HTTP dentro del transporte.

### P3: Cadena de middleware

**Contexto.** Componer logica transversal exige hoy repetirla en handlers.
El diseno debe respetar la orientacion ligera del framework.

**Alcance por definir.** Resolver primero los casos concretos de composicion y
su interaccion con handlers sincronos y diferidos.

**Componentes.** API de registro de rutas, contratos de handlers y tests.

**Aceptacion propuesta.** Especificar orden, interrupcion de cadena, errores,
cancelacion y ownership; probarlos sin penalizar el camino sin middleware.

**Fuera de alcance.** No existe una arquitectura de middleware decidida ni
justificacion para introducir un framework general de extensiones.

### P4: Parsing de formularios

**Contexto.** No hay parsing de `application/x-www-form-urlencoded` ni
`multipart/form-data`. Existe una primitiva de percent-decoding.
Estimacion original: M.

**Alcance por definir.** Separar ambos formatos y decidir representacion de
campos repetidos, payloads binarios y errores sin asumir que sus gramaticas
coinciden con las de una query.

**Componentes.** Helpers HTTP, body reader, API de formularios y tests.

**Aceptacion propuesta.** Casos validos y malformados, codificacion, campos
vacios/repetidos, limites y boundaries fragmentados para multipart.

**Dependencias.** Reutilizar las primitivas existentes solo donde coincida su
semantica. Coordinar presupuestos de recursos con R2.

## Calidad y validacion

### P5: Suite de conformidad exhaustiva

**Contexto.** La integracion propia cubre framing, fragmentacion, pipelining
y cierre con sockets reales. No constituye una matriz RFC exhaustiva.

**Riesgo.** Una regresion puede depender de segmentacion TCP, requests
concatenadas, limites o framing ambiguo y escapar a los casos actuales.

**Alcance.** Ampliar sistematicamente positivos y negativos de request
smuggling, Content-Length/Transfer-Encoding, chunked, trailers y limites.

**Componentes.** Decoder, reglas de framing, request/response, contrato
protocolo-transporte y ambos backends.

**Aceptacion y pruebas.** Trazar cada grupo de casos a la regla RFC, cubrir
fragmentaciones relevantes y verificar resultados equivalentes en IOCP y
epoll. Los casos de limites configurados dependen de lo decidido en R2.

**Referencias.** RFC 9110 y RFC 9112. La suite aporta evidencia a la declaracion
de HTTP/1.1 estricto; no sustituye la revision del contrato.

### QA3: Fuzzing

**Contexto.** ASan, UBSan y TSan ya estan integrados. Los tests escritos
ejecutan un conjunto finito de caminos.

**Alcance futuro.** Preparar un plan independiente de fuzzing para helpers,
decoder, framers/readers, rebasing de request, serializacion, byte storage y
las fronteras relevantes del transporte.

**Aceptacion propuesta.** Definir targets, corpus inicial, presupuestos y
reproduccion de fallos. Conservar cada fallo confirmado como regresion
determinista y ejecutar los targets con el sanitizer adecuado.

**Dependencias y limites.** Elegir infraestructura antes de incorporar
dependencias. La integracion CI de fuzzing esta diferida fuera del hardening
actual; no reabrir el trabajo ya completado de sanitizers.

### QA4: Baseline de rendimiento

**Contexto.** Hay adaptadores para HttpArena y Web Frameworks Benchmark.
Falta un baseline persistente y un gate sobre throughput, latencia,
asignaciones, memoria y escalado. El runner upstream de Web Frameworks no
tiene un commit fijado.

**Impacto.** No se puede cuantificar la promesa de alto rendimiento ni
comparar con confianza decisiones sobre shared_ptr, std::function, spill
o buffers.

**Alcance.** Fijar revisiones, escenarios y condiciones para request minima,
headers grandes, body inline/streaming, pipelining y concurrencia.

**Componentes.** Adaptadores, decoder, router, serializacion y transportes.

**Aceptacion y pruebas.** Registrar toolchain, hardware, revisiones exactas,
distribuciones de latencia y repeticion de muestras. Definir tolerancias
segun ruido medido y una forma reproducible de comparar con la release.

**Dependencias.** Separar medicion de optimizacion. La medida historica de DT1
se conserva en calidad, pero no sustituye este baseline.

### QA5: Diagnostico y aislamiento del harness

**Contexto.** El runner unitario invoca cada caso sin capturar excepciones.
Una excepcion inesperada puede terminar el ejecutable. `expect` recibe
expresion, fichero y linea, pero no los imprime. CTest registra toda la suite
unitaria como un solo test.

**Impacto.** Los fallos intermitentes o excepcionales ofrecen poco diagnostico
y pueden ocultar resultados de los casos posteriores.

**Componentes.** `tests/unit/test_helper.h`, `test_helper.cpp` y registro
CTest; revisar el helper de integracion si comparte el comportamiento.

**Aceptacion y pruebas.** Un caso que lanza debe identificarse como fallido y
permitir reportar los siguientes. Cada asercion fallida muestra expresion y
ubicacion. Evaluar granularidad CTest sin dependencias innecesarias.

### Validacion adicional

**Campanas de estres.** La cobertura QA1/QA2 esta completada. Queda explorar
soak tests prolongados y, cuando sea viable, interleavings controlados entre
workers. Definir duracion, carga, recursos observados y reproduccion antes de
crear nuevas pruebas; relacionarlas con C1/C5 y los escenarios de QA4.

**Automatizacion de compliance externo.** h1spec y Http11Probe se ejecutan
manualmente. Su automatizacion esta diferida fuera del hardening actual.
Al retomarla, usar los adaptadores versionados, fijar el entorno y distinguir
fallos del runner de fallos de protocolo. Conservar logs y revisiones de cada
ejecucion. Esta tarea complementa P5.

## Release engineering

### RE4: Gobierno y trazabilidad de release

**Contexto.** La version procede de `include/version.h`. Existe licencia,
README y un workflow de publicacion condicionado a toda la matriz CI.
El trabajo implementado se describe en calidad.

**Pendiente.** Crear changelog, politica de seguridad y guia de contribucion.
Definir canales para vulnerabilidades, compatibilidad y contribuciones.

**Componentes.** Documentos publicos de proyecto, version y procedimiento de
release; modificar el workflow solo si el mecanismo elegido lo requiere.

**Aceptacion y verificacion.**

- Cada documento publica procedimientos y canales realmente disponibles.
- Cada tag apunta a una revision con matriz CI verde y docs sincronizadas.
- La version y las notas describen el contenido publicado.
- Resolver como publicar `0.1.0-beta.1`: el workflow actual deriva solamente
  `vMAJOR.MINOR.PATCH` y no expresa sufijos prerelease. Esta observacion del
  workflow requiere una decision, no implica una modificacion ya acordada.

**Dependencias.** RE1-RE3 deben seguir verdes y C1/C5 deben superar sus pruebas
sobre sockets reales para el objetivo beta. Esta guia de desarrollo no
sustituye una guia de contribucion publica con canales y procedimiento.

## Mantenibilidad C++

### DT4: Dependencias y efectos globales de platform.h

**Contexto.** `platform.h` concentra headers estandar/de sistema, `INLINE`,
macros Windows, desactivacion de warning 4996 y pragmas de enlace.
En otras plataformas, `tcpip.h` no selecciona backend ni diagnostica
explicitamente la falta de soporte.

**Riesgo.** Macros y warnings afectan al consumidor; los includes transitivos
ocultan dependencias y producen errores confusos.

**Componentes.** `include/platform.h`, selectores y backends, y todos los
headers que dependan de sus includes.

**Aceptacion y verificacion.** Inventariar usos reales, justificar cada
macro/pragma y compilar los headers publicos de forma autosuficiente.
Definir un diagnostico para plataformas no soportadas si ese es el contrato.
Comprobar que no cambian inadvertidamente las opciones del consumidor.

**Limites.** Cambios locales basados en evidencia; no se propone una
reorganizacion general de includes ni ampliar plataformas por defecto.

### DT5: Contrato de getters indexados

**Contexto.** `request::get_header(size_t)` y
`get_query_parameter(size_t)` usan `operator[]`; un indice valido depende
del caller. Otras APIs usan excepciones u optional para ausencia.

**Riesgo.** Un indice invalido produce UB si se incumple esa precondicion.

**Componentes.** API de request, tests y documentacion publica.

**Aceptacion y verificacion.** Decidir si la validez del indice es una
precondicion documentada o requiere comprobacion. Especificar el contrato,
probar los limites conforme a la decision y preservar compatibilidad.
Medir el impacto si afecta recorridos frecuentes.

**Dependencias.** Coordinar con la documentacion de vistas y getters siguiente.
La inconsistencia no autoriza por si sola un cambio de API.

## Documentacion publica

### Ciclo de vida del transporte

**Pendiente.** Documentar el uso directo del transporte fuera de
`v11::server`.

**Contexto a conservar.** `stop()` se llama fuera de sus workers y los
callbacks no cambian durante start/stop. El contrato distingue cierre
ordenado y aborto fatal.

**Aceptacion.** Contrastar y documentar orden de llamadas, hilos permitidos,
callbacks, reinicio y errores de arranque con ejemplos publicos. Enlazar
desde arquitectura y verificar cada ejemplo con los tests correspondientes.

### Vistas y getters de request

**Pendiente.** Documentar propiedad y lifetime de string_view/header_view y
precondiciones de getters indexados.

**Contexto a conservar.** Los getters no entregan copias propietarias;
request posee el buffer del head y rebasa las vistas. El reader prestado
requiere un almacenamiento externo que sobreviva a sus usos.

**Aceptacion.** Mostrar usos validos, invalidacion por destruccion y la
precondicion elegida en DT5. Contrastar con las pruebas de lifetime y enlazar
el contrato desde los ejemplos correspondientes.

## Fuera de la primera release

Estas capacidades quedan diferidas y sin identificadores de los lotes 0.1.
El aplazamiento no incluye limites, clientes lentos, estres ni baselines:
esos trabajos conservan sus entradas anteriores.

### TLS

**Contexto.** El despliegue previsto para la primera release usa un terminador
TLS, como un reverse proxy. Estimacion original: A.

**Alcance futuro.** Integrar TLS preservando la frontera protocolo-transporte.

**Decisiones y verificacion.** Elegir dependencia y modelo de ownership antes
de implementar; especificar handshake, cierre y errores con pruebas
equivalentes en las plataformas soportadas.

### Compresion y GZIP

**Contexto.** La negociacion `Accept-Encoding`/`Content-Encoding` y la
gestion de `Vary` son capacidades opcionales.

**Decisiones.** Una biblioteca de compresion externa entra en conflicto con
la promesa actual de cero dependencias. Resolver esa politica y el alcance
de formatos antes de disenar la API.

**Aceptacion propuesta.** Negociacion y `Vary` coherentes, framing correcto
y pruebas de cuerpos vacios, binarios y grandes.

### Streaming progresivo y SSE

**Contexto.** El handler termina de producir el body antes de entregar la
respuesta; el drenaje actual no constituye streaming progresivo.
Estimacion original: A.

**Alcance futuro.** Representar inicio, fragmentos y final/error; aplicar
backpressure por bytes y agrupar fragmentos pequenos.

**Aceptacion propuesta.** Probar cancelacion, cliente lento, errores parciales
y orden. Conservar el coste del camino one-shot y del hot path sincrono,
comparandolo con QA4.

### Barrera ordenada de upgrade

**Contexto.** El `101` debe alcanzar la cabeza del orden de respuestas antes
de transferir el canal. Estimacion original: A.

**Alcance futuro.** Detener la decodificacion HTTP en el momento correcto,
entregar los bytes residuales al nuevo codec y transferir el control.

**Aceptacion propuesta.** Probar primero con un codec ficticio: respuestas
previas, bytes ya recibidos, cierre y errores. El contrato sigue siendo
generico y no filtra HTTP a IOCP ni epoll.

**Dependencias.** Prerrequisito del soporte de WebSockets.

### WebSockets

**Contexto.** `channel_intent::kUpgrade` esta definido y los headers
Sec-WebSocket-* estan modelados, pero los transportes no manejan el upgrade.
La primera release no declara esa capacidad. Estimacion original: A.

**Alcance futuro.** Handshake, framing, fragmentacion, ping/pong/close,
envios iniciados externamente y backpressure bidireccional.

**Aceptacion propuesta.** Definir primero el contrato y la conformidad del
protocolo; probar mensajes fragmentados, cierre simultaneo, errores y
clientes lentos en ambas plataformas.

**Dependencias.** Barrera ordenada de upgrade. Conservar lo ya modelado;
aplazar esta funcionalidad no convierte por si solo el core en incumplidor
de HTTP/1.1.
