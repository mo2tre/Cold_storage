# ERD v1: auth and master data

```mermaid
erDiagram
    roles ||--o{ users : "has"
    roles ||--o{ role_permissions : "grants"
    permissions ||--o{ role_permissions : "granted in"
    users ||--o{ farmers : "created_by/updated_by"
    users ||--o{ products : "created_by/updated_by"
    users ||--o{ cold_rooms : "created_by/updated_by"
    cold_rooms ||--o{ room_products : "accepts"
    products ||--o{ room_products : "stored in"

    roles { bigint id PK
            varchar name UK }
    permissions { bigint id PK
                  varchar code UK }
    role_permissions { bigint role_id PK,FK
                       bigint permission_id PK,FK }
    users { bigint id PK
            varchar username UK
            varchar password_hash
            bigint role_id FK
            tinyint is_active }
    farmers { bigint id PK
              varchar code UK
              varchar full_name
              varchar phone
              tinyint is_active }
    products { bigint id PK
               varchar code UK
               varchar name
               decimal min_temp_c
               decimal max_temp_c
               int max_storage_days }
    cold_rooms { bigint id PK
                 varchar code UK
                 decimal capacity_kg
                 decimal min_temp_c
                 decimal max_temp_c }
    room_products { bigint room_id PK,FK
                    bigint product_id PK,FK }
```

Next migrations (40-50%+): entries, entry_details, stock_lots, stock_allocations, stock_movements,
transfers, exits, exit_details, exit_allocations, invoices, payments, temperature_logs, inspections,
damaged_stock_cases, notifications, audit_logs, facility_plans.
