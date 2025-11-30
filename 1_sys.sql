-- Run this script in SYS tenant to enable Index Merge feature
-- Connect: obclient -h127.0.0.1 -P2881 -uroot@sys -A -e "source /root/ob/1_sys.sql"

-- 1) (Optional) show current tenant
SHOW TENANT;

-- 2) Enable Index Merge globally (all tenants)
ALTER SYSTEM SET _enable_index_merge = true;

-- 3) (Optional) Only enable for a specific tenant (uncomment and set your tenant name)
-- ALTER SYSTEM SET _enable_index_merge = true TENANT = 'test';

-- 4) Verify parameter
SHOW PARAMETERS LIKE '%_enable_index_merge%';

