# Documentacion de doba

Doba es un framework de servidor C++20 header-only con backends IOCP en
Windows y epoll en Linux.

## Guia de lectura

| Documento | Contenido y fuente de verdad |
| --- | --- |
| [Arquitectura](ARCHITECTURE.md) | Componentes, contratos, ownership y flujo de datos actuales. |
| [Desarrollo](DEVELOPMENT.md) | Requisitos, build, estilo y validacion de cambios. |
| [Calidad](QUALITY.md) | Pruebas disponibles, CI y evidencia de correcciones completadas. |
| [Backlog](BACKLOG.md) | Todos los pendientes, dependencias y criterios de release. |

La implementacion dispone de pruebas unitarias, integracion sobre sockets
reales y CI multiplataforma con sanitizers. El objetivo de la proxima beta
y sus condiciones de salida se mantienen en el
[backlog de release](BACKLOG.md#objetivo-de-release).

El [README del proyecto](../README.md) presenta la biblioteca y su consumo.
Los [ejemplos](../examples/README.md) documentan usos concretos de la API.

## Mantenimiento

Actualizar cada dato en su documento de referencia. Al cerrar un pendiente,
trasladar la evidencia relevante a calidad y el contrato implementado a
arquitectura; registrar su identificador como referencia historica y
mantener actualizados el inventario y los enlaces del backlog.
Los ejemplos y las instrucciones especificas de herramientas permanecen junto
a sus componentes y se enlazan desde esta documentacion.
