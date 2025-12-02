#pragma once
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <unordered_map>

namespace adlog {

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }


    template<typename... Args>
    void debug(Args&&... args) {
        std::ostringstream oss;
        (oss << ... << args);
        log("DEBUG", oss.str());
    }

    template<typename... Args>
    void debug_once(int out_cnt, Args&&... args) {
        std::ostringstream oss;
        (oss << ... << args);
        std::string formatted_message = oss.str();
        
        std::lock_guard<std::mutex> lock(mutex_);
        auto& counter = debug_once_counters_[formatted_message];
        counter.count++;
        
        if (counter.count % out_cnt == 0) {
            log("DEBUG", "[COUNT:" + std::to_string(counter.count) + "] " + formatted_message);
        }
    }


private:
    Logger() {
        std::lock_guard<std::mutex> lock(mutex_);
        log_file_.open("/root/ad/ad2/ob/log/adlog.log", std::ios::app);
        if (!log_file_.is_open()) {
            system("mkdir -p /root/ad/ad2/ob/log");
            log_file_.open("/root/ad/ad2/ob/log/adlog.log", std::ios::app);
        }

    }

    ~Logger() {
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

    void log(const std::string& level, const std::string& message) {

        std::lock_guard<std::mutex> lock(mutex_);
        
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        log_file_ << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
                  << "." << std::setfill('0') << std::setw(3) << ms.count()
                  << " " << level << "] " << message << std::endl;
        log_file_.flush();
    }

    struct CounterInfo {
        int count = 0;
    };

    std::ofstream log_file_;
    std::mutex mutex_;
    std::unordered_map<std::string, CounterInfo> debug_once_counters_;
};

// 便捷函数

    template<typename... Args>
    void DEBUG(Args&&... args) {
        Logger::getInstance().debug(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void DEBUG_ONCE(int out_cnt, Args&&... args) {
        Logger::getInstance().debug_once(out_cnt, std::forward<Args>(args)...);
    }


} // namespace adlog