#!/usr/bin/env python3

import mysql.connector

def test_mysql_count_syntax():
    """测试MySQL中count(*,num)的行为"""
    try:
        # 连接到MySQL
        connection = mysql.connector.connect(
            host='localhost',
            user='root',
            password='hxl',
            autocommit=True,
            ssl_disabled=True
        )
        
        cursor = connection.cursor()
        
        # 创建测试数据库
        cursor.execute("CREATE DATABASE IF NOT EXISTS test_miniob")
        cursor.execute("USE test_miniob")
        
        # 创建测试表
        cursor.execute("DROP TABLE IF EXISTS aggregation_func")
        cursor.execute("CREATE TABLE aggregation_func(id int, num int, price float, addr char(4), birthday date)")
        cursor.execute("INSERT INTO aggregation_func VALUES (1, 10, 5.5, 'ABC', '2020-01-01')")
        cursor.execute("INSERT INTO aggregation_func VALUES (2, 20, 6.5, 'DEF', '2021-01-01')")
        
        print("=== 测试MySQL中的COUNT语法 ===")
        
        # 测试正确的COUNT语法
        print("\n1. 正确的COUNT语法:")
        cursor.execute("SELECT count(*) FROM aggregation_func")
        result = cursor.fetchall()
        print(f"SELECT count(*) FROM aggregation_func; -> {result}")
        
        cursor.execute("SELECT count(num) FROM aggregation_func")
        result = cursor.fetchall()
        print(f"SELECT count(num) FROM aggregation_func; -> {result}")
        
        # 测试错误的COUNT语法
        print("\n2. 错误的COUNT语法:")
        try:
            cursor.execute("SELECT count(*,num) FROM aggregation_func")
            result = cursor.fetchall()
            print(f"SELECT count(*,num) FROM aggregation_func; -> {result}")
        except mysql.connector.Error as err:
            print(f"SELECT count(*,num) FROM aggregation_func; -> ERROR: {err}")
        
        try:
            cursor.execute("SELECT count(num,price) FROM aggregation_func")
            result = cursor.fetchall()
            print(f"SELECT count(num,price) FROM aggregation_func; -> {result}")
        except mysql.connector.Error as err:
            print(f"SELECT count(num,price) FROM aggregation_func; -> ERROR: {err}")
            
        try:
            cursor.execute("SELECT count() FROM aggregation_func")
            result = cursor.fetchall()
            print(f"SELECT count() FROM aggregation_func; -> {result}")
        except mysql.connector.Error as err:
            print(f"SELECT count() FROM aggregation_func; -> ERROR: {err}")
        
        cursor.close()
        connection.close()
        
    except mysql.connector.Error as err:
        print(f"MySQL连接错误: {err}")

if __name__ == "__main__":
    test_mysql_count_syntax()
