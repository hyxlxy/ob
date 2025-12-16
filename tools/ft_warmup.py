#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
A lightweight warmup script for seekdb fulltext caches.
- Pre-warms term-stat cache by executing COUNT(*) aggregation on tokens extracted from query set
- Pre-warms Block-Max score cache by running small MATCH queries per token with LIMIT 1
- Optionally warms base_id popular groups to help storage cache

Usage:
  python3 tools/ft_warmup.py --queries /path/to/mldr-v1.0-en/test.jsonl --host 127.0.0.1 --port 2881 --user root --db test --limit 1 --top_base 16

Note:
  This script uses the obclient CLI; make sure deps/3rd/u01/obclient/bin/obclient exists or obclient is in PATH.
"""
import argparse
import json
import os
import subprocess

DEFAULT_OBCLIENT = os.environ.get("OBCLIENT", "./deps/3rd/u01/obclient/bin/obclient")

DDL_TEMPLATE = """
SET SESSION ob_query_timeout=200000000; -- 200s
"""

COUNT_SQL = "SELECT /*+ QUERY_TIMEOUT(200000000) */ COUNT(*) FROM items1 WHERE MATCH(fulltext_col) AGAINST({tok});"
PRE_MATCH_SQL = "SELECT /*+ QUERY_TIMEOUT(200000000) */ docid_col FROM items1 WHERE MATCH(fulltext_col) AGAINST({tok}) ORDER BY docid_col LIMIT {lim};"
TOP_BASE_SQL = "SELECT base_id, COUNT(*) c FROM items1 GROUP BY base_id ORDER BY c DESC LIMIT {n};"


def run_sql(sql:str, host:str, port:int, user:str, db:str):
    cmd = [DEFAULT_OBCLIENT, f"-h{host}", f"-P{port}", f"-u{user}", f"-D{db}", "-A", "-e", sql]
    # print("RUN:", " ".join(cmd))
    try:
        subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except subprocess.CalledProcessError:
        pass


def iter_queries(path):
    with open(path, 'r', encoding='utf-8') as f:
        for line in f:
            line=line.strip()
            if not line:
                continue
            try:
                obj=json.loads(line)
                text=obj.get('text') or obj.get('query') or obj.get('question')
                if text:
                    yield text
            except Exception:
                continue


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--queries', required=True)
    ap.add_argument('--host', default='127.0.0.1')
    ap.add_argument('--port', type=int, default=2881)
    ap.add_argument('--user', default='root')
    ap.add_argument('--db', default='test')
    ap.add_argument('--limit', type=int, default=1)
    ap.add_argument('--top_base', type=int, default=16)
    args=ap.parse_args()

    # warm top base groups
    run_sql(TOP_BASE_SQL.format(n=args.top_base), args.host, args.port, args.user, args.db)

    # iterate queries and warm per token implicitly via MATCH
    seen=set()
    for q in iter_queries(args.queries):
        q=q.strip()
        if not q or q in seen:
            continue
        seen.add(q)
        tok=f"'{q.replace("'","\\'")}'"
        run_sql(COUNT_SQL.format(tok=tok), args.host, args.port, args.user, args.db)
        run_sql(PRE_MATCH_SQL.format(tok=tok, lim=args.limit), args.host, args.port, args.user, args.db)

if __name__=='__main__':
    main()

