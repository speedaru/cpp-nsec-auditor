#pragma once
#include "pch.h"

namespace nsec::utils {
    class Logger {
    public:
        enum class Level {
            Info,
            Warn,
            Critical
        };

        static void Info(const std::string& message);
        static void Warn(const std::string& message);
        static void Critical(const std::string& message);

    private:
        static void Log(Level level, const std::string& message);
    };
}
