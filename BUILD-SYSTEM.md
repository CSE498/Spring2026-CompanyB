# Build System

There are two parts of the build system:

- **Native** — C++ and Qt6, supports the pure logic and Qt source
- **Emscripten** — compiles Emscripten

Docker can be used to build and test both of these parts. Local builds/tests are supported as well. Dependencies are required to be installed.

---

## Prerequisites

### Docker

> [!IMPORTANT]  
> Docker is required for Docker-based builds.

Install [Docker Desktop](https://www.docker.com/products/docker-desktop/) for your platform and make sure it is running before using any `make docker-*` target.

### Qt6

> [!IMPORTANT]  
> Required for local builds if using Qt.

If you want to build and run the native Qt application directly on your machine:

**macOS**
```bash
brew install qt
```

**Windows**

Download and run the [Qt Online Installer](https://www.qt.io/download-qt-installer). Select Qt 6.x for Desktop during setup. After installing, set `CMAKE_PREFIX_PATH` as a user environment variable pointing to your Qt installation:
```ps1
CMAKE_PREFIX_PATH=C:\Qt\6.x.x\msvc2022_64 # Select the version for your compiler (Ex: MSVC MinGW)
```

**Linux**

Use your distro's package manager. Peak at the Dockerfile to see what I am installing for the container.

On Debian/Ubuntu:
```bash
sudo apt install qt6-base-dev libgl-dev
```
Other distros vary — you know your system.

---

## Docker Builds

All Docker targets build inside a container with the full toolchain pre-installed. No local compiler or Qt installation required.

### Emscripten (WebAssembly)

| Command | Description |
|---|---|
| `make docker-build-emscripten` | Build the project to `/build/emscripten/` |
| `make docker-test-emscripten` | Build and run Emscripten Catch2 tests |
| `make docker-serve` | Build and serve on `http://localhost:8080` |

### Native (Qt)

> [!WARNING]
> `docker-build-native` produces a **Linux binary**. It will not run on macOS or Windows and possibly various linux distros. For local development with the GUI, build on your host machine instead (see below).

| Command | Description |
|---|---|
| `make docker-build-native` | Build the native Qt application to `/build/native/` |
| `make docker-test-native` | Build and run native Catch2 tests (headless) |

### Other

| Command | Description |
|---|---|
| `make docker-dev` / `make docker-shell` | Interactive shell inside the container |
| `make docker-image` | Build the Docker image |
| `make docker-rebuild` | Rebuild the Docker image from scratch (no cache) |
| `make docker-clean` | Remove the `build/` directory |

---

## Local Builds (Native + Qt only)

These run on your host machine and require Qt6 installed (see prereqs above).

| Command | Description |
|---|---|
| `make` / `make build` | Configure and build |
| `make debug` | Build with debug symbols |
| `make opt` | Optimized release build |
| `make quick` | Fast compile, minimal checks |
| `make grumpy` | Build with extra warnings (`-Wconversion -Weffc++`) |
| `make test` | Build and run Catch2 tests |
| `make all` | Build + run tests |
| `make clean` | Remove build artifacts |

Output binary is at `/build/native/app`. Test binary is at `/build/tests/tests`.

You can also forward targets directly to `/source/` or `/tests/`:
```bash
make src-debug     # runs 'make debug' in source/
make test-build    # runs 'make build' in tests/
make test-list     # list source, test, and object files cmake picked up
```

Run `make help` for a full list.

---

## CI

All pushes to any branch run two jobs in a CI workflow:

- **native-test** — installs Qt6 and runs `make test` on Ubuntu (headless)
- **emscripten-test** — installs emsdk 5.0.0 and runs the Emscripten Catch2 tests

---

## Support

For build system issues, reach out to **Maksim Savich** in the `#ci-cd-support` Discord channel.
