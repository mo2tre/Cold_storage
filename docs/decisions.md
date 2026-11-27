# Technical decisions

| # | Decision | Reason / notes |
|---|----------|----------------|
| 1 | Backend: C++17, Drogon 1.9.x (vcpkg manifest mode) | Agreed in the master plan. |
| 2 | Compiler: Visual Studio, MSVC x64. The lead builds with VS 2026 (MSVC 14.52); the plan says VS 2022 (v143) | **To confirm:** all four members must use the same toolchain, especially for Qt. |
| 3 | MySQL 8.4 needs `mysql_native_password=ON` in `my.ini` (`[mysqld]`) | The vcpkg `libmariadb` build cannot load the `caching_sha2_password` plugin on Windows (error 1045 "Plugin caching_sha2_password could not be loaded"). The application account `cold_app` is created `WITH mysql_native_password`. Only the server computer (Computer 1) is affected; Qt clients never connect to MySQL. Revisit if the MySQL version changes (the plugin is deprecated). |
| 4 | MySQL `default-time-zone='+00:00'` | Timestamps are stored in UTC; the client converts to facility-local time. |
| 5 | Application DB account is least-privilege (`SELECT, INSERT, UPDATE, DELETE`) | Migrations are applied with a separate privileged account. |
| 6 | API port 8080 in the plan is only an example; the lead uses 8081 locally | Qt client must read the API URL from settings. |
| 7 | `config/config.json` (with DB password) is never committed; `config.example.json` is | See `.gitignore`. |

| 8 | Passwords: PBKDF2-HMAC-SHA256, 600,000 iterations, random 16-byte salt (OpenSSL), stored as `pbkdf2_sha256$iter$salt$hash` | Modern salted hash, no extra library. Can be replaced by Argon2 later; the stored format carries the scheme. |
| 9 | Sessions: opaque random 256-bit bearer tokens; only the SHA-256 hash is stored in `sessions`; 8-hour expiry; logout revokes | Supports expiry and logout (plan, section 13) without a JWT library. |
| 10 | The first Administrator is created with `cold_storage_backend.exe --hash-password` + one SQL `INSERT` (see `database/README.md`) | No default/seeded password exists anywhere in the repository. |

Open points from the plan still to confirm: billing rate/currency/rounding, partial-payment policy, quarantine
room vs. marked zone, full list of roles/permissions.
