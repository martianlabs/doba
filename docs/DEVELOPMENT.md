# Desarrollo

[Indice](HANDOFF.md)

## Contenido

- [Requisitos y organizacion](#requisitos-y-organizacion)
- [Compilacion y pruebas](#compilacion-y-pruebas)
- [Consumo CMake](#consumo-cmake)
- [Estilo del repositorio](#estilo-del-repositorio)
- [Validacion de cambios](#validacion-de-cambios)

## Requisitos y organizacion

- C++20; backends disponibles para Windows y Linux.
- CMake 3.20 o superior para configuracion manual.
- CMake 3.25 o superior para los presets MSVC de esquema 6.
- Ninja y un entorno de desarrollo MSVC inicializado para los presets Windows.

La biblioteca reside en `include/`. Los tests de componente replican la ruta
del header bajo `tests/unit`, `tests/integration` o `tests/package`.
Los helpers y `CMakeLists.txt` compartidos permanecen en la raiz de cada suite.
Consultar el [mapa de componentes](ARCHITECTURE.md#arquitectura-actual) y
los [ejemplos](../examples/README.md).

## Compilacion y pruebas

Ejecutar los comandos desde la raiz del proyecto. Con un generador de
configuracion unica y un compilador C++20 disponible:

```sh
cmake -S . -B out/build/debug -DCMAKE_BUILD_TYPE=Debug -DDOBA_ENABLE_STRICT_WARNINGS=ON
cmake --build out/build/debug
ctest --test-dir out/build/debug --output-on-failure
```

Para Release, usar otro directorio y `-DCMAKE_BUILD_TYPE=Release`.
En Windows, desde un entorno de desarrollo MSVC:

```sh
cmake --preset msvc-debug -DDOBA_ENABLE_STRICT_WARNINGS=ON
cmake --build --preset build-debug
ctest --test-dir out/build/msvc-debug --output-on-failure

cmake --preset msvc-release -DDOBA_ENABLE_STRICT_WARNINGS=ON
cmake --build --preset build-release
ctest --test-dir out/build/msvc-release --output-on-failure
```

Las opciones de [CMakeLists.txt](../CMakeLists.txt) son:

| Opcion | Valor por defecto | Efecto |
| --- | --- | --- |
| `DOBA_BUILD_EXAMPLES` | ON como proyecto principal; OFF como subproyecto | Compila los ejemplos. |
| `DOBA_BUILD_TESTS` | ON como proyecto principal; OFF como subproyecto | Habilita las suites propias. |
| `DOBA_ENABLE_STRICT_WARNINGS` | OFF | Activa warnings estrictos en el arbol de doba. |

La [matriz de calidad](QUALITY.md#integracion-continua) detalla los compiladores
y sanitizers configurados. Sus comandos exactos estan en el
[workflow CI](../.github/workflows/ci.yml).

## Consumo CMake

La instalacion y los consumidores se documentan en el
[README](../README.md). El paquete se busca con
`find_package(doba CONFIG REQUIRED)` y se enlaza con `martianlabs::doba`.

El target exporta los includes, C++20 y Threads. Los flags internos de warnings
y sanitizers no forman parte de su interfaz instalada. Al usar
`add_subdirectory`, los tests y ejemplos de doba quedan desactivados por
defecto, incluso si el proyecto padre activa su propio testing.

## Estilo del repositorio

Los ficheros equivalentes del mismo modulo son la referencia de estilo.
Conservar nombres, orden de declaraciones, includes, namespaces, firmas,
indentacion y estructura de clases y tests.

- UTF-8 sin BOM, contenido ASCII y finales CRLF en el working tree.
- Newline final obligatorio y sin espacios al final de linea.
- Lineas C++ de hasta 80 columnas.
- Cada fichero C++ nuevo usa la cabecera Apache/doba de un equivalente.
- Los cambios locales conservan encoding, formato y cabeceras existentes.
- Los comentarios explican restricciones necesarias y son breves.
- Las referencias RFC usan `S`, por ejemplo `RFC 9110 S7.6.1`.
- En comentarios enmarcados, conservar el ancho y la alineacion de cierre.

CMake pasa `/utf-8` a MSVC para fijar los juegos de caracteres de origen y
ejecucion. Las convenciones anteriores deben comprobarse tambien en tests y
documentacion, con independencia de la configuracion local del editor.

## Validacion de cambios

1. Inspeccionar el componente, sus equivalentes y sus tests.
2. Identificar la causa y el contrato aplicable; citar la seccion RFC cuando
   determine un comportamiento HTTP.
3. Mantener el cambio localizado, preservando API, ownership y frontera
   protocolo-transporte.
4. Para un bug, anadir una regresion que falle antes y pase despues, con los
   limites relevantes del caso.
5. Ejecutar las pruebas focalizadas y la suite afectada. Usar la matriz CI
   para validar las plataformas y configuraciones pertinentes.
6. Revisar el diff, los enlaces documentales y el formato. Ejecutar
   `git diff --check`; comprobar CRLF con `git ls-files --eol -- <fichero>`
   y verificar ASCII, ausencia de BOM y newline final.
7. Registrar que pruebas se ejecutaron, sus resultados y cualquier limitacion.

Los cambios de protocolo conservan la distincion entre sintaxis y semantica.
Los cambios de rendimiento requieren medidas reproducibles y no deben
debilitar la correccion. Los cambios documentales se validan mediante
contenido, referencias y formato; no requieren ejecutar CTest por si solos.
