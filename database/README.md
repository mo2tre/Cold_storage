# Database

MySQL 8.4 LTS. Versioned, forward-only migrations: never edit a migration that was already applied
(or pushed to `develop`); add a new `V00N__name.sql` instead.

- `migrations/V*.sql`: schema
- `seeds/S*.sql`: non-sensitive reference data (roles, permissions)
- `diagrams/`: ERD
- `apply_migrations.ps1`: runs pending files once each and records them in `schema_migrations`

## Setup on Computer 1

1. Create the database and a least-privilege application account (run as root in Workbench; choose your own password, never commit it):
   ```sql
   CREATE DATABASE cold_storage CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci;
   CREATE USER 'cold_app'@'localhost' IDENTIFIED BY '<choose-a-password>';
   GRANT SELECT, INSERT, UPDATE, DELETE ON cold_storage.* TO 'cold_app'@'localhost';
   ```
2. Keep timestamps in UTC: in `my.ini` under `[mysqld]` add `default-time-zone='+00:00'`, then restart MySQL.
3. Apply migrations and seeds with a user that can create tables (root for now):
   ```powershell
   .\database\apply_migrations.ps1 -MysqlExe "C:\Program Files\MySQL\MySQL Server 8.4\bin\mysql.exe" -Database cold_storage -User root -Seeds
   ```

## Create the first Administrator (once)

No user is seeded, so nobody can log in until you create one.

1. Generate a password hash (the password is typed hidden and is not stored anywhere):
   ```powershell
   cd backend
   .\build\Release\cold_storage_backend.exe --hash-password
   ```
   It prints one line starting with `pbkdf2_sha256$...`. Copy it.
2. In Workbench (database `cold_storage`), paste your hash:
   ```sql
   INSERT INTO users (username, full_name, password_hash, role_id)
   SELECT 'admin', 'Administrator', 'PASTE_THE_HASH_HERE', id FROM roles WHERE name = 'Administrator';
   ```
3. Log in with `POST /api/v1/auth/login` (see `docs/api-contract-v1.md`).
