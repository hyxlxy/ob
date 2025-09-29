#!/bin/bash

# SQL测试文件运行脚本
# 支持多种数据库连接方式

echo "=== SQL测试文件运行工具 ==="
echo "1. 使用MySQL命令行客户端"
echo "2. 使用DBeaver (图形化界面)"
echo "3. 使用Navicat (图形化界面)"
echo "4. 查看SQL文件内容"
echo "5. 退出"

read -p "请选择运行方式 (1-5): " choice

case $choice in
    1)
        echo "使用MySQL命令行客户端..."
        if command -v mysql &> /dev/null; then
            echo "请输入MySQL连接信息："
            read -p "主机 (默认: localhost): " host
            read -p "端口 (默认: 3306): " port
            read -p "用户名 (默认: root): " username
            read -s -p "密码 (默认: hxl): " password
            echo
            
            host=${host:-localhost}
            port=${port:-3306}
            username=${username:-root}
            password=${password:-hxl}
            
            mysql -h "$host" -P "$port" -u "$username" -p"$password" < test/case/test/test.test
        else
            echo "错误: MySQL客户端未安装"
        fi
        ;;
    2)
        echo "启动DBeaver..."
        if command -v dbeaver &> /dev/null; then
            dbeaver &
            echo "DBeaver已启动，请手动导入SQL文件: test/case/test/test.test"
        else
            echo "错误: DBeaver未安装"
        fi
        ;;
    3)
        echo "启动Navicat..."
        if [ -f ~/Desktop/navicat17-mysql-cs-x86_64.AppImage ]; then
            ~/Desktop/navicat17-mysql-cs-x86_64.AppImage &
            echo "Navicat已启动，请手动导入SQL文件: test/case/test/test.test"
        else
            echo "错误: Navicat未找到"
        fi
        ;;
    4)
        echo "=== SQL文件内容 ==="
        cat test/case/test/test.test
        ;;
    5)
        echo "退出"
        exit 0
        ;;
    *)
        echo "无效选择"
        ;;
esac
