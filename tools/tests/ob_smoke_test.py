#!/usr/bin/env python3

import os
import sys
import time
import argparse

try:
    import mysql.connector  # type: ignore
except Exception as exc:  # pragma: no cover
    sys.stderr.write(
        "mysql-connector-python is required. Install with: pip install mysql-connector-python\n"
    )
    raise


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="OceanBase smoke test via MySQL protocol"
    )
    parser.add_argument("--host", default=os.getenv("OB_HOST", "127.0.0.1"))
    parser.add_argument("--port", type=int, default=int(os.getenv("OB_PORT", "10000")))
    parser.add_argument("--user", default=os.getenv("OB_USER", "root"))
    parser.add_argument("--password", default=os.getenv("OB_PASSWORD", ""))
    parser.add_argument("--database", default=os.getenv("OB_DATABASE", "oceanbase"))
    parser.add_argument("--timeout", type=int, default=20, help="connect timeout seconds")
    return parser.parse_args()


def connect_with_retry(cfg):
    deadline = time.time() + cfg.timeout
    last_err = None
    while time.time() < deadline:
        try:
            conn = mysql.connector.connect(
                host=cfg.host,
                port=cfg.port,
                user=cfg.user,
                password=cfg.password,
                database=cfg.database,
                autocommit=True,
                connection_timeout=5,
            )
            return conn
        except Exception as err:  # pragma: no cover
            last_err = err
            time.sleep(1)
    raise RuntimeError(f"Failed to connect to OceanBase at {cfg.host}:{cfg.port}: {last_err}")


def main() -> int:
    cfg = parse_args()
    print(
        f"Connecting to OceanBase mysql://{cfg.user}@{cfg.host}:{cfg.port}/{cfg.database} ..."
    )
    conn = connect_with_retry(cfg)
    print("Connected.")

    cur = conn.cursor()
    cur.execute("select version();")
    version = cur.fetchone()[0]
    print(f"Server version: {version}")

    cur.execute("create database if not exists ob_smoke;")
    cur.execute("use ob_smoke;")
    cur.execute(
        "create table if not exists t_smoke (id bigint primary key, name varchar(64), created_at timestamp default current_timestamp);"
    )
    cur.execute("truncate table t_smoke;")

    rows_to_insert = [(1, "alice"), (2, "bob"), (3, "charlie")]
    cur.executemany("insert into t_smoke(id, name) values (%s, %s);", rows_to_insert)

    cur.execute("select id, name from t_smoke order by id;")
    rows = cur.fetchall()
    print("Selected rows:")
    for r in rows:
        print(f"  id={r[0]}, name={r[1]}")

    # simple update and delete
    cur.execute("update t_smoke set name = 'alice_updated' where id = 1;")
    cur.execute("delete from t_smoke where id = 3;")

    cur.execute("select count(*) from t_smoke;")
    cnt = cur.fetchone()[0]
    print(f"Row count after update/delete: {cnt}")

    # cleanup is optional: keep database for inspection
    cur.close()
    conn.close()
    print("Done.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())


