#include "pch.h"
#include <nsec/ui/MonitorUI.h>
#include <nsec/utils/Logger.h>
using namespace nsec::ui;

#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
    #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
        #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
    #endif
#endif

static void SetupTerminal() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

static GlobalStats AggregateStats(const std::string& reportsDir) {
    GlobalStats stats;
    if (!fs::exists(reportsDir)) return stats;

    for (const auto& entry : fs::directory_iterator(reportsDir)) {
        if (entry.path().extension() != ".json") {
            continue;
        }

        try {
            std::ifstream file(entry.path());
            nlohmann::json report = nlohmann::json::parse(file);

            stats.totalScans++;

            if (report.contains("issues") && report["issues"].is_array()) {
                for (const auto& issue : report["issues"]) {
                    std::string sev = issue.value("severity", "");
                    if (sev == "Critical") stats.criticalIssues++;
                    else if (sev == "Warning") stats.warningIssues++;
                }
            }
        } catch (...) {
            // skip bad json files
        }
    }

    // scoring logic: 100 - (C*20) - (W*5)
    int penalty = (stats.criticalIssues * 20) + (stats.warningIssues * 5);
    stats.securityGrade = std::max(0, 100 - penalty);

    return stats;
}

static char GetGradeLetter(int score) {
    if (score >= 90) return 'A';
    if (score >= 75) return 'B';
    if (score >= 50) return 'C';
    if (score >= 25) return 'D';
    return 'F';
}

static const char* GetGradeColor(int score) {
    if (score >= 75) return "\033[32m"; // green
    if (score >= 40) return "\033[33m"; // yellow
    return "\033[31m"; // red
}

static void Render(const GlobalStats& stats) {
    // ANSI clear screen and home cursor
    std::cout << "\033[H\033[J";

    std::cout << "\033[1;36m==================================================\033[0m\n";
    std::cout << "\033[1;37m          NSEC SECURITY COMMAND CENTER            \033[0m\n";
    std::cout << "\033[1;36m==================================================\033[0m\n\n";

    std::cout << "  [Live Status]      " << (stats.totalScans > 0 ? "\033[32mACTIVE\033[0m" : "\033[31mIDLE\033[0m") << "\n";
    std::cout << "  [Total Scans]      " << stats.totalScans << "\n";
    std::cout << "  [Issues Blocked]   \033[31m" << stats.criticalIssues << " Critical\033[0m, \033[33m" 
        << stats.warningIssues << " Warning\033[0m\n\n";

    std::cout << "  [Project Health]   " << GetGradeColor(stats.securityGrade) 
        << "GRADE " << GetGradeLetter(stats.securityGrade) 
        << " (" << stats.securityGrade << "/100)\033[0m\n\n";

    std::cout << "\033[1;36m--------------------------------------------------\033[0m\n";
    std::cout << "  Press Ctrl+C to exit monitoring mode.\n";
    std::cout << "\033[1;36m--------------------------------------------------\033[0m\n";
    std::cout.flush();
}

void nsec::ui::MonitorUI::Start(const std::string& reportsDir) {
    SetupTerminal();
    
    while (true) {
        GlobalStats stats = AggregateStats(reportsDir);
        Render(stats);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}
