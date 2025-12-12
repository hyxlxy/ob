#pragma once
#include <sstream>
#include <string>

namespace adlog {

// 启用/禁用 DEBUG 日志（默认关闭）
void enable_debug(bool enable = true);
bool is_debug_enabled();

// 内部实现声明（定义在 .cpp）
void log_debug_impl(const std::string &msg);
void log_debug_once_impl(int interval, const std::string &msg);

// ====== 保留你原来的调用方式 ======
template <typename... Args> void DEBUG(Args &&...args) {
  if (is_debug_enabled()) {
    std::ostringstream oss;
    (oss << ... << args);
    log_debug_impl(oss.str());
  }
}

template <typename... Args> void DEBUG_ONCE(int out_cnt, Args &&...args) {
  if (is_debug_enabled()) {
    std::ostringstream oss;
    (oss << ... << args);
    log_debug_once_impl(out_cnt, oss.str());
  }
}

} // namespace adlog