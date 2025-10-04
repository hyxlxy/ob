# GDB script to debug subquery crash
set pagination off
set logging file /tmp/miniob_gdb.log
set logging on

# 设置断点
break filter_stmt.cpp:433
break select_stmt.cpp:192
break sql_task_handler.cpp:96

# 捕获所有退出信号
catch signal SIGABRT
catch signal SIGSEGV
catch throw

# 运行程序
run -f /tmp/miniob/miniob/conf/observer.ini

# 当程序停止时自动执行的命令
commands
  backtrace
  info locals
  info args
  continue
end

