#include "pch.h"
#include <nsec/utils/Logger.h>

namespace nsec::utils {
    void Logger::Info(const std::string& message) { Log(Level::Info, message); }
    void Logger::Warn(const std::string& message) { Log(Level::Warn, message); }
    void Logger::Critical(const std::string& message) { Log(Level::Critical, message); }

    void Logger::Log(Level level, const std::string& message) {
        switch (level) {
            case Level::Info:
                std::cout << "\033[36m[INFO]\033[0m " << message << std::endl;
                break;
            case Level::Warn:
                std::cout << "\033[33m[WARN]\033[0m " << message << std::endl;
                break;
            case Level::Critical:
                std::cerr << "\033[31m[CRITICAL]\033[0m " << message << std::endl;
                break;
        }
    }
}
