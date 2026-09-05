# Calidad y evidencia

[Indice](HANDOFF.md)

Inventario contrastado el 2026-09-05 sobre
`9aa4c1d64969179f450d7d3ec4f4badef2fc144d`.
Este documento distingue implementacion, pruebas disponibles y resultados
historicos. La reorganizacion documental no ejecuto builds ni CTest.
El trabajo restante se mantiene en [BACKLOG.md](BACKLOG.md).

## Contenido

- [Pruebas](#pruebas)
- [Integracion continua](#integracion-continua)
- [Compliance y rendimiento](#compliance-y-rendimiento)
- [Correcciones y decisiones completadas](#correcciones-y-decisiones-completadas)
- [Interpretacion de la evidencia](#interpretacion-de-la-evidencia)

## Pruebas

| Suite | Ficheros de casos | Casos DOBA_TEST | Registro CTest |
| --- | --- | --- | --- |
| Unitaria | 115 | 401 | `doba_unit_tests` |
| Integracion | 2 | 39 | `doba_integration_tests` |

Los consumidores CMake de `tests/package` se compilan por separado; no se
incluyen en ese recuento.

### Integracion de transporte (QA1)

[Los tests TCP/IP](../tests/integration/transport/server/tcpip_tests.cpp)
contienen 38 casos con sockets loopback reales. El mismo codigo selecciona
IOCP en Windows y epoll en Linux. Cubren aceptacion, fragmentacion, envios
acotados, desconexion, errores, streaming, orden y callbacks.

[El test HTTP](../tests/integration/protocol/http/v11/server_tests.cpp)
ejercita la composicion del servidor sobre TCP/IP, incluidos comportamientos
automaticos e interinos.

### Concurrencia y ciclo de vida (QA2)

La suite cubre pipelines sincronos y diferidos, respuestas fuera de orden,
clientes concurrentes, excepciones de handler, cancelacion, apagado con estados
mixtos y reinicio repetido. Los tests de `date_server` incluyen varios
propietarios y `start()`/`stop()` concurrentes. El servidor HTTP tiene una
regresion de rollback y reintento tras fallo del transporte.

Esta cobertura funcional no demuestra todos los interleavings ni sustituye
campanas prolongadas de estres. El trabajo adicional se recoge en el
[backlog de QA](BACKLOG.md#calidad-y-validacion).

## Integracion continua

La fuente ejecutable es [.github/workflows/ci.yml](../.github/workflows/ci.yml).

| Validacion configurada | Entorno |
| --- | --- |
| Build y CTest Debug/Release | GCC y Clang en Linux |
| Build y CTest Debug/Release | MSVC en Windows |
| ASan, UBSan y TSan | Clang Debug; suites unitaria e integracion |
| Version minima de CMake | CMake 3.20.6 |
| Instalacion y paquete | Consumidores externos Debug/Release |
| Consumo del arbol fuente | Consumidor mediante `add_subdirectory` |

### Compiladores y warnings (RE1)

CI activa `DOBA_ENABLE_STRICT_WARNINGS`: GCC/Clang usan
`-Wall -Wextra -Wpedantic -Werror`; MSVC usa `/W4 /WX`.
Los flags afectan al arbol propio y no se exportan al consumidor instalado.

### Instalacion y consumo (RE2)

CMake obtiene la version de [include/version.h](../include/version.h),
instala headers y package config y exporta `martianlabs::doba`.
CI instala en un prefijo aislado y compila consumidores externos mediante
`find_package(doba CONFIG REQUIRED)`.

[El consumidor de prueba](../tests/package/CMakeLists.txt) tambien usa
`add_subdirectory` y comprueba que no se incorporan los ejemplos ni las
suites internas. El target instalado aporta includes, C++20 y Threads.

### Compatibilidad y publicacion (RE3 y parte completada de RE4)

CMake 3.20 es el minimo del proyecto. Los presets MSVC requieren 3.25.
CI verifica 3.20.6 y utiliza los presets en Windows.

El workflow permite crear manualmente una GitHub Release desde `main`
despues de superar toda la matriz. Deriva `vMAJOR.MINOR.PATCH` de
`include/version.h` y comprueba el destino de un tag ya existente.
El mecanismo actual genera tags numericos; la publicacion de una etiqueta
prerelease requiere resolver el punto indicado en
[RE4](BACKLOG.md#re4-gobierno-y-trazabilidad-de-release).

## Compliance y rendimiento

Los adaptadores de [h1spec](../compliance/h1spec/README.md) y
[Http11Probe](../compliance/http11probe/README.md) estan versionados y se
ejecutan manualmente. Sus instrucciones permanecen junto a cada adaptador.

Existen adaptadores para
[HttpArena](../benchmarks/httparena/http/v11/README.md) y
[Web Frameworks Benchmark](../benchmarks/web-frameworks/http/v11/README.md).
HttpArena fija la revision upstream; ambos Dockerfiles permiten fijar
`DOBA_REF`. La existencia de estos adaptadores no equivale a un baseline
de release ni a un resultado de rendimiento reproducible.

## Correcciones y decisiones completadas

### Fechas condicionales invalidas (C7)

El dispatcher convertia el fallo del checker estricto en rechazo de toda la
request. El decoder ahora conserva `If-Modified-Since` e
`If-Unmodified-Since` con fechas invalidas sin rechazar la peticion.
La sintaxis general del field-value y los checkers independientes siguen
siendo estrictos. La evaluacion de precondiciones y `If-Range` no cambiaron.

Referencias: RFC 9110 S13.1.3 y S13.1.4. Hay regresiones para ambos campos en
[decoder_tests.cpp](../tests/unit/protocol/http/v11/decoder_tests.cpp)
y comprobacion sobre TCP/IP de respuesta 200 en lugar de 400.

### Respuestas sin contenido

La serializacion de 205 elimina el source y el cuerpo inline, retira
`Transfer-Encoding` y fuerza `Content-Length: 0`. La regresion compartida
con 1xx, 204 y 304 reside en
[response_tests.cpp](../tests/unit/protocol/http/v11/response_tests.cpp).
Referencia de 205: RFC 9110 S15.3.6.

### Finalizacion del almacenamiento

`finish()` sella el escritor y el storage: escribir despues falla y repetir
la finalizacion conserva el tamano. El contrato se prueba con memoria y spill
en [writer_tests.cpp](../tests/unit/common/writer_tests.cpp) y
[byte_storage_tests.cpp](../tests/unit/common/byte_storage_tests.cpp).

### Fecha compartida y rollback del servidor (R1)

El servicio de fecha cuenta propietarios bajo mutex y permanece activo hasta
el ultimo `stop()`. Hay pruebas de propietarios simultaneos y concurrencia en
[date_server_tests.cpp](../tests/unit/common/date_server_tests.cpp).

Si `transport_.start()` lanza, la capa HTTP libera su adquisicion de fecha
y relanza la excepcion original. Una instancia que no arranco no libera
recursos ajenos al detenerse. El transporte fake reproduce el fallo y el
reintento en [server_tests.cpp](../tests/unit/protocol/http/v11/server_tests.cpp).

### Buffers propietarios bajo RAII (DT1)

`decoder` y `request` usan `std::unique_ptr<char[]>` y
`make_unique_for_overwrite`, con destructores predeterminados y sin
inicializacion adicional del buffer. Sus tests cubren lifetime de vistas,
percent-decoding, dispatch incremental y bodies.

El registro anterior de DT1 conserva estos resultados historicos:

- Suites completas con GCC y MSVC Debug/Release y warnings estrictos.
- GCC bajo ASan y UBSan.
- GCC 13: `request` de 200 bytes y `decoder` de 712 bytes.
- Cinco muestras de `/pipeline`, una conexion y profundidad 32: medianas
  de 377142 req/s antes y 376937 req/s despues (-0,05%); p50 de 75 us
  en ambos casos.

Estas medidas se conservan como antecedente de la decision. No fueron
repetidas durante la reorganizacion ni constituyen el baseline de release
descrito en [QA4](BACKLOG.md#qa4-baseline-de-rendimiento).

## Interpretacion de la evidencia

Los recuentos anteriores describen el arbol, y la tabla CI describe lo que el
workflow ejecuta. Para afirmar que una revision pasa, comprobar su ejecucion
concreta de configuracion, build y CTest y la matriz remota correspondiente.

Los sanitizers detectan defectos en los caminos ejecutados. Las pruebas
actuales no demuestran conformidad exhaustiva ni ausencia de carreras en todos
los interleavings. El fuzzing, la ampliacion sistematica de compliance y el
baseline tienen entradas independientes en el backlog.
