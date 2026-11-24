# Cold Storage Management System

Windows desktop system for a cold-storage facility (LAN deployment).

- **Backend:** C++17 / Drogon REST API (`backend/`)
- **Frontend:** C++17 / Qt 6 Widgets (`frontend/`)
- **Database:** MySQL 8.4 LTS (`database/`)
- **Architecture:** modular monolith; the API is the only component that accesses MySQL.

## Prerequisites

- Visual Studio 2022 (Desktop development with C++, MSVC v143 x64)
- CMake >= 3.20, Git
- vcpkg (manifest mode)
- MySQL Server 8.4 (on Computer 1 only)
- Qt 6 (same release and MSVC 2022 64-bit kit for all Qt developers)
- Bruno (API testing)

## Build the backend

```powershell
cd backend
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

## Run and check

```powershell
copy config\config.example.json config\config.json   # config.json is NOT committed
.\build\Release\cold_storage_backend.exe config\config.json
curl http://localhost:8080/api/v1/health
```

Expected response:

```json
{ "success": true, "data": { "status": "ok", "service": "cold-storage-backend" }, "message": "Operation completed" }
```

## Git workflow

- `main` = stable release/demo, `develop` = integration, `feature/<name>` = individual work.
- Never push directly to `main`; use pull requests with at least one reviewer.
- Never commit passwords, tokens, keys, or `config.json`.

## Repository layout

```
backend/            Drogon API
frontend/           Qt desktop client
database/           migrations, seeds, diagrams
docs/               SRS, API contracts, decisions
tests/
api-collection/     Bruno collection
```
