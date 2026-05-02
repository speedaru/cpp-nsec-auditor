#pragma once
#include "pch.h"

namespace nsec::ui {
    struct GlobalStats {
        uint32_t totalScans = 0;
        uint32_t criticalIssues = 0;
        uint32_t warningIssues = 0;
        int securityGrade = 100;
        std::string lastScanTime = "N/A";
    };

    namespace MonitorUI {
        /**
         * @brief starts the monitoring loop and blocks the calling thread
         */
        void Start(const std::string& reportsDir);
    };
}
