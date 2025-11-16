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
├─ut-001-main
│   └─build
│       └─test
.
```

