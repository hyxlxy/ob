#include "ad_log.h"
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <unordered_map>

namespace {
bool g_debug_enabled = fals;
std::mutex g_log_mutex;
std::unordered_map<std::string, int> g_debug_once_counters;
std::ofstream g_log_file;

void ensure_log_file_open() {
  if (!g_log_file.is_open()) {
    std::system("mkdir -p /root/ad/ad2/ob/log");
    g_log_file.open("/root/ad/ad2/ob/log/adlog.log", std::ios::app);
  }
}

void write_log_line(const std::string &level, const std::string &message) {
  ensure_log_file_open();
  std::lock_guard<std::mutex> lock(g_log_mutex);
  
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) %
            1000;

  g_log_file << "["
             << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
             << "." << std::setfill('0') << std::setw(3) << ms.count() << " "
             << level << "] " << message << std::endl;
  g_log_file.flush();
}
} // namespace

namespace adlog {

void enable_debug(bool enable) { g_debug_enabled = enable; }

bool is_debug_enabled() { return g_debug_enabled; }

void log_debug_impl(const std::string &msg) { write_log_line("DEBUG", msg); }

void log_debug_once_impl(int interval, const std::string &msg) {
  std::lock_guard<std::mutex> lock(g_log_mutex);
  int &counter = g_debug_once_counters[msg];
  counter++;
  if (counter % interval == 0) {
    write_log_line("DEBUG", "[COUNT:" + std::to_string(counter) + "] " + msg);
  }
}

} // namespace adlog