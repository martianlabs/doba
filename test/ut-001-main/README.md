<div align="center">

  ![doba](../../resources/doba_logo.png)

</div>

# OVERVIEW

# BUILD

To build a **Debug** runtime configuration use:
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

To build a **Release** runtime configuration use:
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

# RUN

After building the binary you should get something like the following:
```bash
.
├─**gateway**
│     └─build
│         ├─gateway        Microservice binary file.
│         ├─ca-root.pem    Needed certificate in order to consume Azure based resources (you can find it in the */setup/microsoft/* folder).
│         └─registry.json  Needed configuration file if using locally provided registry services (automatically copied by default from */libraries/registry/* folder).
.
```

