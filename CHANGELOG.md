# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)  
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Endpoint scaffolding for upcoming modules (catalog, users, auth, orders).
- CI placeholder (workflow to be added).
- Basic test placeholder with CTest.

### Changed

- Improved developer UX in `Makefile` (`make dev`, clearer logs).
- Consistent CMake presets mapping (`dev-ninja` → `build-ninja` / `run-ninja`).

### Fixed

- N/A

### Security

- N/A

## [v0.1.0] - 2025-10-14

### Added

- Initial project bootstrap with **Vix.cpp** (C++20).
- `CMakeLists.txt` with `find_package(Vix REQUIRED)` and presets (`dev-ninja`, `dev-msvc`).
- Cross-platform `Makefile` (build/run/clean/release/tag/merge/changelog).
- Minimal app entry (`src/main.cpp`) with:
  - `GET /` → returns `{ "message": "Hello from Softadastra Backend (Vix.cpp)" }`
  - `GET /health` → returns `{ "status": "ok" }`
- `README.md` with quick start (configure, build, run).
- This `CHANGELOG.md` following Keep a Changelog.

### Changed

- `.gitignore` tailored for CMake/Ninja/MSVC builds and local artifacts.

### Fixed

- N/A

### Security

- N/A

<!--
Link references
Replace REPO_URL with your repository URL when available, e.g. https://github.com/softadastra/softadastra-backend-vix
-->

[Unreleased]: REPO_URL/compare/v0.1.0...HEAD
[v0.1.0]: REPO_URL/releases/tag/v0.1.0
