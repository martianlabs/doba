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
├─<strong>gateway</strong>
│&nbsp;&nbsp;└─build
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;├─gateway        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Microservice binary file.
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;├─ca-root.pem    &nbsp;&nbsp;&nbsp;&nbsp;Needed certificate in order to consume Azure based resources (you can find it in the <em>/setup/microsoft/</em> folder).
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;└─registry.json  &nbsp;&nbsp;Needed configuration file if using locally provided registry services (automatically copied by default from <em>/libraries/registry/</em> folder).
.
```
