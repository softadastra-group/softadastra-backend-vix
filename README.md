# 🧩 softadastra-backend-vix — Example project using [Vix.cpp](https://github.com/vixcpp/vix)

High-performance backend for the Softadastra marketplace, built entirely with Vix.cpp, a modern C++20 web framework inspired by FastAPI and Express.js. Designed to showcase the speed, modularity, and developer experience of Vix.cpp in a real-world production API.

---

## 🚀 Features

- Simple **HTTP server** powered by `Vix::App`
- Cross-platform build system (Linux / macOS / Windows)
- Modern **C++20** codebase
- Configurable via CMake presets (`dev-ninja`, `dev-msvc`)
- Optional sanitizers for debug builds
- Integrated logging (via Vix logger)

---

## 🏗️ Project Structure

```
softadastra-backend-vix/
├── CMakeLists.txt        # Main build configuration
├── CMakePresets.json     # Cross-platform presets
├── Makefile              # Simplified build helper
├── README.md             # Project documentation
└── src/
    └── main.cpp          # Application entry point
```

---

## ⚙️ Requirements

- **CMake ≥ 3.20**
- **C++20 compiler**
  - Linux/macOS: Clang ≥ 15 or GCC ≥ 11
  - Windows: Visual Studio 2022 (MSVC 19.3+)
- **Ninja** (optional, for fast builds)
- **Vix.cpp installed** under `/usr/local` or built locally

---

## 🔧 Build and Run

### 🐧 Linux / macOS

```bash
make build      # Configure + build (via preset)
make run        # Build and execute
```

### or manually with CMake:

```bash
cmake --preset dev-ninja
cmake --build --preset dev-ninja
```

### 🪟 Windows (Visual Studio 2022)

```powershell
cmake --preset dev-msvc
cmake --build --preset dev-msvc
```

> The `run` target is already defined in the CMake file — it will execute the compiled binary automatically.

---

## 🧰 Useful Commands

| Command                      | Description                             |
| ---------------------------- | --------------------------------------- |
| `make build`                 | Configure and build with default preset |
| `make run`                   | Build and run the app                   |
| `make clean`                 | Remove all build folders                |
| `make rebuild`               | Clean and rebuild everything            |
| `make PRESET=dev-msvc build` | Build with custom preset (e.g. Windows) |

---

## ⚡ Example Output

When built successfully, you’ll see logs like:

```bash
[2025-10-12 13:41:23.220] [vixLogger] [info] Using configuration file: /home/user/vixcpp/vix/config/config.json
[2025-10-12 13:41:23.221] [vixLogger] [info] Acceptor initialized on port 8080
[2025-10-12 13:41:23.221] [vixLogger] [info] Server request timeout set to 5000 ms
```

Visit **http://localhost:8080/** to test.

---

## 🧩 About Vix.cpp

[Vix.cpp](https://github.com/vixcpp/vix) is a high-performance, modular C++ web framework inspired by **FastAPI**, **Express.js**, and **Vue.js**.

It offers:

- Extreme performance (**40k+ requests/sec**)
- Clean syntax (`App app; app.get("/", ...);`)
- Modular architecture (`core`, `orm`, `cli`, `json`, `utils`, etc.)
- Simple CMake integration for external apps

---

## 🪪 License

MIT © [Vix.cpp Authors](https://github.com/vixcpp)
