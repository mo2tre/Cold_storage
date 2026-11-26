-- V002: master data (farmers, products, cold_rooms, room_products)
-- Weights are DECIMAL(12,3) kg. Never FLOAT/DOUBLE for inventory or money.
-- Farmers/products/rooms are deactivated (is_active = 0), never deleted once they have history.

CREATE TABLE farmers (
    id         BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    code       VARCHAR(30)     NOT NULL,
    full_name  VARCHAR(150)    NOT NULL,
    phone      VARCHAR(30)     NULL,
    email      VARCHAR(150)    NULL,
    address    VARCHAR(255)    NULL,
    identifier VARCHAR(50)     NULL,                 -- approved identity document number (to confirm)
    notes      TEXT            NULL,
    is_active  TINYINT(1)      NOT NULL DEFAULT 1,
    created_at DATETIME(3)     NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
    updated_at DATETIME(3)     NOT NULL DEFAULT CURRENT_TIMESTAMP(3) ON UPDATE CURRENT_TIMESTAMP(3),
    created_by BIGINT UNSIGNED NULL,
    updated_by BIGINT UNSIGNED NULL,
    PRIMARY KEY (id),
    UNIQUE KEY uq_farmers_code (code),
    KEY idx_farmers_full_name (full_name),
    KEY idx_farmers_phone (phone),
    KEY idx_farmers_identifier (identifier),
    KEY idx_farmers_is_active (is_active),
    CONSTRAINT fk_farmers_created_by FOREIGN KEY (created_by) REFERENCES users (id),
    CONSTRAINT fk_farmers_updated_by FOREIGN KEY (updated_by) REFERENCES users (id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

CREATE TABLE products (
    id                BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    code              VARCHAR(30)     NOT NULL,
    name              VARCHAR(150)    NOT NULL,
    min_temp_c        DECIMAL(5,2)    NOT NULL,
    max_temp_c        DECIMAL(5,2)    NOT NULL,
    min_humidity_pct  DECIMAL(5,2)    NULL,          -- humidity is optional in the first version
    max_humidity_pct  DECIMAL(5,2)    NULL,
    max_storage_days  INT UNSIGNED    NULL,          -- maximum recommended storage duration
    is_active         TINYINT(1)      NOT NULL DEFAULT 1,
    created_at        DATETIME(3)     NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
    updated_at        DATETIME(3)     NOT NULL DEFAULT CURRENT_TIMESTAMP(3) ON UPDATE CURRENT_TIMESTAMP(3),
    created_by        BIGINT UNSIGNED NULL,
    updated_by        BIGINT UNSIGNED NULL,
    PRIMARY KEY (id),
    UNIQUE KEY uq_products_code (code),
    KEY idx_products_name (name),
    KEY idx_products_is_active (is_active),
    CONSTRAINT chk_products_temp CHECK (min_temp_c <= max_temp_c),
    CONSTRAINT chk_products_humidity CHECK (
        (min_humidity_pct IS NULL AND max_humidity_pct IS NULL)
        OR (min_humidity_pct IS NOT NULL AND max_humidity_pct IS NOT NULL
            AND min_humidity_pct <= max_humidity_pct)),
    CONSTRAINT fk_products_created_by FOREIGN KEY (created_by) REFERENCES users (id),
    CONSTRAINT fk_products_updated_by FOREIGN KEY (updated_by) REFERENCES users (id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

CREATE TABLE cold_rooms (
    id                BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    code              VARCHAR(30)     NOT NULL,
    name              VARCHAR(150)    NOT NULL,
    capacity_kg       DECIMAL(12,3)   NOT NULL,
    min_temp_c        DECIMAL(5,2)    NOT NULL,
    max_temp_c        DECIMAL(5,2)    NOT NULL,
    min_humidity_pct  DECIMAL(5,2)    NULL,
    max_humidity_pct  DECIMAL(5,2)    NULL,
    is_active         TINYINT(1)      NOT NULL DEFAULT 1,
    created_at        DATETIME(3)     NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
    updated_at        DATETIME(3)     NOT NULL DEFAULT CURRENT_TIMESTAMP(3) ON UPDATE CURRENT_TIMESTAMP(3),
    created_by        BIGINT UNSIGNED NULL,
    updated_by        BIGINT UNSIGNED NULL,
    PRIMARY KEY (id),
    UNIQUE KEY uq_cold_rooms_code (code),
    KEY idx_cold_rooms_is_active (is_active),
    CONSTRAINT chk_cold_rooms_capacity CHECK (capacity_kg > 0),
    CONSTRAINT chk_cold_rooms_temp CHECK (min_temp_c <= max_temp_c),
    CONSTRAINT chk_cold_rooms_humidity CHECK (
        (min_humidity_pct IS NULL AND max_humidity_pct IS NULL)
        OR (min_humidity_pct IS NOT NULL AND max_humidity_pct IS NOT NULL
            AND min_humidity_pct <= max_humidity_pct)),
    CONSTRAINT fk_cold_rooms_created_by FOREIGN KEY (created_by) REFERENCES users (id),
    CONSTRAINT fk_cold_rooms_updated_by FOREIGN KEY (updated_by) REFERENCES users (id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- Explicit room/product compatibility rules (validated by the backend on entry and transfer).
CREATE TABLE room_products (
    room_id    BIGINT UNSIGNED NOT NULL,
    product_id BIGINT UNSIGNED NOT NULL,
    PRIMARY KEY (room_id, product_id),
    KEY idx_room_products_product_id (product_id),
    CONSTRAINT fk_room_products_room    FOREIGN KEY (room_id)    REFERENCES cold_rooms (id) ON DELETE CASCADE,
    CONSTRAINT fk_room_products_product FOREIGN KEY (product_id) REFERENCES products (id)   ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
