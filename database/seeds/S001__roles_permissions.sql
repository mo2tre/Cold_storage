-- S001: roles and permissions (non-sensitive). Roles are the proposed ones in the plan (to confirm).
-- No user and no password is seeded here: the first Administrator is created by the backend setup step.

INSERT INTO roles (name, description) VALUES
    ('Administrator',    'Users, roles, permissions, configuration, all operational modules'),
    ('Manager',          'Dashboard/reports, authorized exceptional corrections, room/product settings'),
    ('Operator',         'Entries, transfers, exits, temperature readings, routine records'),
    ('Quality Inspector','Inspections, damaged quantities, quarantine, quality follow-up');

INSERT INTO permissions (code, description) VALUES
    ('users.manage',       'Create/deactivate users, change roles'),
    ('config.manage',      'Change system configuration'),
    ('farmers.read',       'View farmers'),
    ('farmers.write',      'Create/edit farmers'),
    ('products.read',      'View products'),
    ('products.write',     'Create/edit products'),
    ('rooms.read',         'View cold rooms'),
    ('rooms.write',        'Create/edit cold rooms'),
    ('entries.write',      'Create entries (bon d''entree)'),
    ('inventory.read',     'View inventory and lots'),
    ('transfers.write',    'Create/confirm transfers'),
    ('exits.write',        'Create exits'),
    ('invoices.read',      'View invoices'),
    ('payments.write',     'Record payments'),
    ('temperature.write',  'Record temperature readings'),
    ('inspections.write',  'Record inspections, damaged cases, quarantine'),
    ('reports.read',       'View reports and dashboard'),
    ('audit.read',         'View audit trail');

-- Administrator: everything
INSERT INTO role_permissions (role_id, permission_id)
SELECT r.id, p.id FROM roles r CROSS JOIN permissions p WHERE r.name = 'Administrator';

-- Manager
INSERT INTO role_permissions (role_id, permission_id)
SELECT r.id, p.id FROM roles r JOIN permissions p
  ON p.code IN ('farmers.read','farmers.write','products.read','products.write','rooms.read','rooms.write',
                'inventory.read','invoices.read','reports.read','audit.read')
WHERE r.name = 'Manager';

-- Operator
INSERT INTO role_permissions (role_id, permission_id)
SELECT r.id, p.id FROM roles r JOIN permissions p
  ON p.code IN ('farmers.read','farmers.write','products.read','rooms.read','entries.write','inventory.read',
                'transfers.write','exits.write','invoices.read','payments.write','temperature.write')
WHERE r.name = 'Operator';

-- Quality Inspector
INSERT INTO role_permissions (role_id, permission_id)
SELECT r.id, p.id FROM roles r JOIN permissions p
  ON p.code IN ('farmers.read','products.read','rooms.read','inventory.read','temperature.write','inspections.write')
WHERE r.name = 'Quality Inspector';
