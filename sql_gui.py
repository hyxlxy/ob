#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox, filedialog
import subprocess
import os
import locale
import sys

# 设置中文环境
try:
    locale.setlocale(locale.LC_ALL, 'zh_CN.UTF-8')
except:
    try:
        locale.setlocale(locale.LC_ALL, 'C.UTF-8')
    except:
        pass

class SQLRunner:
    def __init__(self, root):
        self.root = root
        self.root.title("SQL测试文件运行器 - 中文版")
        self.root.geometry("900x700")
        
        # 创建主框架
        main_frame = ttk.Frame(root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # 文件选择
        file_frame = ttk.LabelFrame(main_frame, text="SQL文件", padding="5")
        file_frame.grid(row=0, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(0, 10))
        
        self.file_path = tk.StringVar(value="test/case/test/test.test")
        ttk.Entry(file_frame, textvariable=self.file_path, width=50).grid(row=0, column=0, padx=(0, 5))
        ttk.Button(file_frame, text="选择文件", command=self.select_file).grid(row=0, column=1)
        
        # 数据库连接设置
        db_frame = ttk.LabelFrame(main_frame, text="数据库连接", padding="5")
        db_frame.grid(row=1, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(0, 10))
        
        ttk.Label(db_frame, text="主机:").grid(row=0, column=0, sticky=tk.W, padx=(0, 5))
        self.host = tk.StringVar(value="localhost")
        ttk.Entry(db_frame, textvariable=self.host, width=15).grid(row=0, column=1, padx=(0, 10))
        
        ttk.Label(db_frame, text="端口:").grid(row=0, column=2, sticky=tk.W, padx=(0, 5))
        self.port = tk.StringVar(value="3306")
        ttk.Entry(db_frame, textvariable=self.port, width=10).grid(row=0, column=3, padx=(0, 10))
        
        ttk.Label(db_frame, text="用户名:").grid(row=1, column=0, sticky=tk.W, padx=(0, 5))
        self.username = tk.StringVar(value="root")
        ttk.Entry(db_frame, textvariable=self.username, width=15).grid(row=1, column=1, padx=(0, 10))
        
        ttk.Label(db_frame, text="密码:").grid(row=1, column=2, sticky=tk.W, padx=(0, 5))
        self.password = tk.StringVar(value="hxl")
        ttk.Entry(db_frame, textvariable=self.password, show="*", width=15).grid(row=1, column=3)
        
        ttk.Label(db_frame, text="数据库:").grid(row=2, column=0, sticky=tk.W, padx=(0, 5))
        self.database = tk.StringVar(value="test")
        ttk.Entry(db_frame, textvariable=self.database, width=15).grid(row=2, column=1, padx=(0, 10))
        
        # 操作按钮
        button_frame = ttk.Frame(main_frame)
        button_frame.grid(row=2, column=0, columnspan=2, pady=(0, 10))
        
        ttk.Button(button_frame, text="执行SQL", command=self.execute_sql).pack(side=tk.LEFT, padx=(0, 5))
        ttk.Button(button_frame, text="查看SQL", command=self.view_sql).pack(side=tk.LEFT, padx=(0, 5))
        ttk.Button(button_frame, text="打开DBeaver", command=self.open_dbeaver).pack(side=tk.LEFT, padx=(0, 5))
        ttk.Button(button_frame, text="打开Navicat", command=self.open_navicat).pack(side=tk.LEFT)
        
        # 结果显示区域
        result_frame = ttk.LabelFrame(main_frame, text="执行结果", padding="5")
        result_frame.grid(row=3, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(0, 10))
        
        self.result_text = scrolledtext.ScrolledText(result_frame, height=15, width=80)
        self.result_text.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # 配置网格权重
        root.columnconfigure(0, weight=1)
        root.rowconfigure(0, weight=1)
        main_frame.columnconfigure(0, weight=1)
        main_frame.rowconfigure(3, weight=1)
        result_frame.columnconfigure(0, weight=1)
        result_frame.rowconfigure(0, weight=1)
    
    def select_file(self):
        filename = filedialog.askopenfilename(
            title="选择SQL文件",
            filetypes=[("SQL files", "*.sql *.test"), ("All files", "*.*")]
        )
        if filename:
            self.file_path.set(filename)
    
    def view_sql(self):
        try:
            with open(self.file_path.get(), 'r', encoding='utf-8') as f:
                content = f.read()
            self.result_text.delete(1.0, tk.END)
            self.result_text.insert(1.0, content)
        except Exception as e:
            messagebox.showerror("错误", f"无法读取文件: {e}")
    
    def execute_sql(self):
        try:
            host = self.host.get()
            port = self.port.get()
            username = self.username.get()
            password = self.password.get()
            database = self.database.get()
            
            cmd = [
                'mysql',
                f'-h{host}',
                f'-P{port}',
                f'-u{username}',
                f'-p{password}',
                f'{database}'
            ]
            
            self.result_text.delete(1.0, tk.END)
            self.result_text.insert(1.0, f"正在执行SQL文件: {self.file_path.get()}\n")
            self.result_text.insert(tk.END, f"连接: {username}@{host}:{port}\n")
            self.result_text.insert(tk.END, f"数据库: {database}\n\n")
            
            with open(self.file_path.get(), 'r') as f:
                result = subprocess.run(cmd, stdin=f, capture_output=True, text=True)
            
            if result.returncode == 0:
                self.result_text.insert(tk.END, "执行成功!\n")
                if result.stdout:
                    self.result_text.insert(tk.END, f"输出:\n{result.stdout}\n")
            else:
                self.result_text.insert(tk.END, f"执行失败:\n{result.stderr}\n")
                
        except Exception as e:
            messagebox.showerror("错误", f"执行失败: {e}")
    
    def open_dbeaver(self):
        try:
            subprocess.Popen(['dbeaver'])
            self.result_text.insert(tk.END, "DBeaver已启动，请手动导入SQL文件\n")
        except Exception as e:
            messagebox.showerror("错误", f"无法启动DBeaver: {e}")
    
    def open_navicat(self):
        try:
            navicat_path = os.path.expanduser("~/Desktop/navicat17-mysql-cs-x86_64.AppImage")
            if os.path.exists(navicat_path):
                subprocess.Popen([navicat_path])
                self.result_text.insert(tk.END, "Navicat已启动，请手动导入SQL文件\n")
            else:
                messagebox.showerror("错误", "Navicat文件未找到")
        except Exception as e:
            messagebox.showerror("错误", f"无法启动Navicat: {e}")

if __name__ == "__main__":
    root = tk.Tk()
    app = SQLRunner(root)
    root.mainloop()
