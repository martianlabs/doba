# Correcciones CI tras policies — 2026-10-28

Base: `e0e4112b7005a1c9e0ab889112156b25e60598e9`, `feature/limits`.
Run remoto analizado: `40699904594`, PR 32.

## Diagnóstico y cambios

- [x] Identificar la aserción Linux: `peer.negotiate()` en «TLS handshake
  progresses while a small output BIO drains». Reproducido localmente con un
  BIO de 256 bytes, que fuerza la fragmentación de la salida del certificado
  independientemente de los grupos predeterminados de OpenSSL.
- [x] Mantener trabajo de salida pendiente cuando `SSL_do_handshake` queda
  suspendido por `SSL_ERROR_WANT_WRITE`, incluso después de vaciar el BIO.
  La siguiente oportunidad de salida reintenta la negociación. Se conserva
  `pending()` como número de bytes físicos. Sin reactivar el sondeo HTTP por envío.
- [x] Añadir regresión unitaria y de integración para TLS 1.2 y 1.3. El caso de
  integración falla antes del arreglo y pasa después (`reproduce.log`, `fix.log`).
- [x] Identificar la aserción Windows: callback perdido en «last completion races
  stop without losing or duplicating callbacks». El log CI muestra el fallo;
  5.000 repeticiones locales del grupo anterior no lo reprodujeron. El nuevo
  caso persistente sí falla con el header anterior, en su primera ejecución
  Linux GCC (`race-before-last.log`, aserción `mismatches == 0`).
- [x] Corregir la publicación cruzada entre los atomics `active_` y `stopping_`.
  Release/acquire sobre objetos distintos permitía que ambos actores vieran el
  valor anterior y omitieran el callback. Las cuatro operaciones del protocolo
  usan orden secuencialmente consistente. No se añaden mutexes ni operaciones
  atómicas; `begin()` conserva su implementación.
- [x] Añadir un caso con dos threads persistentes y 2.048 carreras por ejecución.
  Cada parada debe completar inmediatamente o invocar exactamente un callback.
- [x] Actualizar la documentación de las suites unitaria y de integración.

## Validación

Completada: construcción completa (incluye ejemplos), seis grupos CTest por
configuración, veinte repeticiones adicionales del nuevo caso TLS por
configuración y doscientas repeticiones del grupo de concurrencia en GCC, TSan
y en las tres configuraciones Windows. Todas las configuraciones habilitan TLS;
las suites completas incluyen TCP y TLS.

| Configuración | Suite completa | Repeticiones TLS | Concurrencia ×200 |
| --- | --- | --- | --- |
| Linux GCC Release | 6/6 | 20/20 | 200/200 |
| Linux Clang ASan | 6/6 | 20/20 | — |
| Linux Clang UBSan | 6/6 | 20/20 | — |
| Linux Clang TSan | 6/6 | 20/20 | 200/200 |
| Windows MSVC Release | 6/6 | 20/20 | 200/200 |
| Windows MSVC ASan | 6/6 | 20/20 | 200/200 |
| Windows Clang UBSan | 6/6 | 20/20 | 200/200 |

Exportación del índice: los 442 archivos de `include/` y `tests/` coinciden
con el árbol de trabajo, normalizando CRLF. Sus 27 unitarios de sesión TLS y
concurrencia se compilan y ejecutan con ASan+UBSan; todos pasan. Los enlaces
locales de ambos README modificados existen en la exportación. El header
`operating_state.h` está versionado y presente.

Inspección GCC 13 x64 `-O3`: `end()` mantiene las mismas instrucciones (solo
cambia el orden de un movimiento entre registros respecto a una carga).
No es una medición de RPS. Se evita añadir sincronización al envío o a la
recepción por esta corrección; la nueva publicación TLS usa el mutex de sesión
que la consulta de bytes pendientes ya utilizaba.

Resultado final: 42/42 grupos CTest, 140/140 ejecuciones TLS adicionales
(280 handshakes forzados entre TLS 1.2 y 1.3), y 1.000/1.000 ejecuciones del
grupo de concurrencia. Solo el nuevo caso persistente acumula 2.048.000 carreras;
se conserva además el caso original que detectó el fallo CI. ASan, UBSan y TSan
no notificaron errores durante las ejecuciones verificadas.

Tiempos de las suites completas: GCC 40,03 s; Linux ASan 41,97 s; Linux
UBSan 38,43 s; Linux TSan 44,59 s; MSVC Release 49,06 s; MSVC ASan 54,94 s;
Windows Clang UBSan 50,58 s. Los tiempos incluyen suites con comprobaciones
temporales; no son mediciones comparativas de rendimiento.

- [x] Revisión final del diff y comprobación de espacios con Git.
- [x] Pruebas unitarias, integración, resiliencia y metatests en las siete configuraciones.
- [x] Controles negativos: el handshake y la publicación antigua fallan con las nuevas regresiones.
- [x] Código, regresiones, documentación y este informe preparados en el índice.

Logs y scripts locales: `out/ci-fix/`. La validación local no confirma el estado
del siguiente run remoto: CI deberá ejecutar estos cambios cuando se publiquen.
