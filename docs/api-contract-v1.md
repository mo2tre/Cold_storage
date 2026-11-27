# API contract v1

Status: **draft for the Qt developer (Member 3)**. Changes go through the backend lead (Member 1) and are
recorded here before either side depends on them.

Base URL: `http://<server-ip>:8081/api/v1` (port and host are configurable in the Qt client; never hard-code them).

## Conventions

- JSON only: `Content-Type: application/json`, UTF-8.
- Names are `snake_case`. Dates/times are ISO 8601 in **UTC** (`2026-11-27T06:16:03.603Z`); the client displays facility-local time.
- Weights are decimal kg and money is decimal; never floats in the database. (Later endpoints will send them as JSON numbers with up to 3 decimals.)
- **Authentication is enforced.** Every endpoint except `GET /health` and `POST /auth/login` requires the header `Authorization: Bearer <token>` (token from `/auth/login`, valid 8 hours). The Qt `ApiClient` adds it to every request and sends the user back to the login screen on `401`.
- Permissions are checked by the server. The login response lists the user's `permissions`; the client uses them only to show/hide buttons (never as security).

### Success envelope
```json
{ "success": true, "data": { }, "message": "Operation completed" }
```

### Error envelope
```json
{ "success": false,
  "error": { "code": "VALIDATION_ERROR", "message": "Invalid farmer data",
             "details": [ { "field": "code", "message": "is required" } ] } }
```

| HTTP | `error.code`       | When |
|------|--------------------|------|
| 400  | `BAD_REQUEST`      | Malformed JSON, invalid id or query parameter |
| 401  | `UNAUTHORIZED`     | Missing, invalid, expired or revoked token; wrong login |
| 403  | `FORBIDDEN`        | Logged in but lacking the required permission |
| 404  | `NOT_FOUND`        | Unknown endpoint or resource |
| 409  | `CONFLICT`         | Duplicate unique value (e.g. farmer `code`) |
| 422  | `VALIDATION_ERROR` | Field validation failed; see `error.details[]` |
| 500  | `INTERNAL_ERROR`   | Server problem; no internal details are exposed |

The client must show `error.message`, and highlight `details[].field` on forms. Validation is also done by the server even if the form validates first.

### Pagination (list endpoints)
Query: `page` (default 1), `page_size` (default 20, max 100). Response `data`:
```json
{ "items": [ ], "page": 1, "page_size": 20, "total": 57 }
```

## System

### GET /health
`200` -> `{ "success": true, "data": { "status": "ok", "service": "cold-storage-backend" }, "message": "Operation completed" }`

Use it for the "connection status" indicator on the login screen.

## Auth

### POST /auth/login
Body: `{ "username": "admin", "password": "..." }`

`200` ->
```json
{ "success": true, "message": "Login successful",
  "data": { "token": "<64 hex chars>", "token_type": "Bearer", "expires_at": "2026-11-27T14:00:00.000Z",
            "user": { "id": 1, "username": "admin", "full_name": "Administrator", "role": "Administrator",
                      "permissions": ["farmers.read", "farmers.write", "..."] } } }
```
Errors: `401 UNAUTHORIZED` "Invalid username or password" (same message for unknown user, wrong password, or deactivated account), `422` missing fields, `400` bad JSON.
Never store the password; keep the token in memory only.

### GET /auth/me
`200` -> `data` = the user object above plus `expires_at`. `401` if the token is missing, invalid, expired or revoked.

### POST /auth/logout
Revokes the current token. `200` -> `"Logged out"`. The client then discards the token.

### Permissions used so far
`farmers.read` (GET), `farmers.write` (POST, PUT). A user without it gets `403 FORBIDDEN`.

## Farmers

Farmers are never deleted once they exist: deactivate with `is_active: false`. (Farmers are updated with `PUT`, the full object; products and cold rooms with `PATCH`, partial.)

### Farmer object
| Field        | Type            | Notes |
|--------------|-----------------|-------|
| `id`         | integer         | Read-only |
| `code`       | string (<= 30)  | Required, unique |
| `full_name`  | string (<= 150) | Required |
| `phone`      | string or null  | <= 30 |
| `email`      | string or null  | <= 150, must contain `@` |
| `address`    | string or null  | <= 255 |
| `identifier` | string or null  | <= 50 (identity document number) |
| `notes`      | string or null  | <= 5000 |
| `is_active`  | boolean         | Default `true` |
| `created_at`, `updated_at` | string | Read-only, UTC |

Optional text fields: send `null`, omit them, or send `""`; the server stores `null`. Text is trimmed.

### GET /farmers
Query: `q` (search in name, code and phone), `is_active` (`true`/`false`), `page`, `page_size`. Sorted by name.
`200` -> paginated list (see above), `items` are farmer objects.

### POST /farmers
Body: farmer fields (`code` and `full_name` required).
`201` -> `data` = created farmer, `message` = "Farmer created". Errors: `409`, `422`, `400`.

Example:
```json
{ "code": "F001", "full_name": "Ahmed Ben Ali", "phone": "20123456" }
```

### GET /farmers/{id}
`200` -> farmer object. `404` if it does not exist, `400` if `{id}` is not a positive integer.

### PUT /farmers/{id}
Replaces all editable fields (send the full object, as the edit form holds all fields). Same validation as POST.
`200` -> updated farmer. Errors: `404`, `409`, `422`, `400`.

## Products

Permissions: `products.read` (GET), `products.write` (POST, PATCH). Products are deactivated (`is_active: false`), not deleted.

| Field | Type | Notes |
|-------|------|-------|
| `id` | integer | Read-only |
| `code` | string (<= 30) | Required, unique |
| `name` | string (<= 150) | Required |
| `min_temp_c`, `max_temp_c` | number | Required, -100..100, min <= max (degrees Celsius) |
| `min_humidity_pct`, `max_humidity_pct` | number or null | Optional but both or neither, 0..100, min <= max |
| `max_storage_days` | integer or null | 1..36500, maximum recommended storage duration |
| `is_active` | boolean | Default `true` |
| `created_at`, `updated_at` | string | Read-only, UTC |

- `GET /products`: query `q` (name or code), `is_active`, `page`, `page_size`. Paginated, sorted by name.
- `POST /products`: `201` with the created product. Errors `409` (duplicate code), `422`.
- `GET /products/{id}`: `200` or `404`.
- `PATCH /products/{id}`: **partial update**: send only the fields to change; the server validates the complete result. `200`, or `404`, `409`, `422`.

## Cold rooms

Permissions: `rooms.read` (GET), `rooms.write` (POST, PATCH, PUT compatibility).

| Field | Type | Notes |
|-------|------|-------|
| `id` | integer | Read-only |
| `code` | string (<= 30) | Required, unique |
| `name` | string (<= 150) | Required |
| `capacity_kg` | number | Required, > 0, up to 3 decimals |
| `min_temp_c`, `max_temp_c` | number | Required, min <= max |
| `min_humidity_pct`, `max_humidity_pct` | number or null | Same rule as products |
| `is_active` | boolean | Default `true` |
| `created_at`, `updated_at` | string | Read-only, UTC |

- `GET /cold-rooms`, `POST /cold-rooms`, `GET /cold-rooms/{id}`, `PATCH /cold-rooms/{id}`: same behaviour as products (sorted by code).
- `GET /cold-rooms/{id}/products` -> `{ "product_ids": [1, 2] }` (products that may be stored in the room).
- `PUT /cold-rooms/{id}/products` with `{ "product_ids": [1, 2] }` replaces the whole set. `422` if an id is not an existing product (nothing changes). An empty array clears it.
- Occupancy (`/cold-rooms/{id}/occupancy`) comes with the entries/stock milestone.

## Not available yet (planned)
Users management, entries, inventory, transfers, exits, invoices.
Each will be added here before implementation.
