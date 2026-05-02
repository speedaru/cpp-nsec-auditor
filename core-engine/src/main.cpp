#include "pch.h"
#include <nsec/models/Report.h>
#include <nsec/utils/Logger.h>
#include <nsec/core/RuleEngine.h>
#include <nsec/ui/MonitorUI.h>

// rules
#include <nsec/rules/BannedFunctionRule.h>
#include <nsec/rules/NestingDepthRule.h>

using namespace nsec;

/**
 * @brief registers default security rules into the engine
 */
static void RegisterRules(core::RuleEngine& engine) {
    engine.AddRule(std::make_unique<rules::BannedFunctionRule>(
        "strcpy", models::Severity::Critical, "Banned 'strcpy' (Overflow risk)."));
    
    engine.AddRule(std::make_unique<rules::BannedFunctionRule>(
        "sprintf", models::Severity::Warning, "'sprintf' is unsafe. Use 'snprintf'."));
    
    engine.AddRule(std::make_unique<rules::NestingDepthRule>());
}

/**
 * @brief exports report to a json file
 */
void ExportReport(const models::Report& report, const std::string& path) {
    try {
        fs::path outPath(path);
        if (outPath.has_parent_path() && !fs::exists(outPath.parent_path())) {
            fs::create_directories(outPath.parent_path());
        }

        std::ofstream outFile(path);
        if (outFile.is_open()) {
            outFile << report.ToJson().dump(4);
            utils::Logger::Info("Report exported to: " + path);
        } else {
            utils::Logger::Critical("Failed to write report to: " + path);
        }
    } catch (const std::exception& e) {
        utils::Logger::Critical("Export error: " + std::string(e.what()));
    }
}

/**
 * @brief prints a summary of the scan results
 */
void PrintSummary(size_t issueCount, double elapsedSeconds) {
    std::cout << "\n--------------------------------------------------\n";
    std::cout << "              SCAN SUMMARY RESULTS                \n";
    std::cout << "--------------------------------------------------\n";
    
    utils::Logger::Info("Scan Completed in " + std::to_string(elapsedSeconds) + "s");
    utils::Logger::Info("Total Issues Found: " + std::to_string(issueCount));
    
    if (issueCount > 0) {
        utils::Logger::Warn("Security Status: VULNERABLE");
    } else {
        utils::Logger::Info("Security Status: CLEAN");
    }
    std::cout << "--------------------------------------------------\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage:\n";
        std::cout << "  nsec-audit <paths...> [-o output.json]      Run a security scan\n";
        std::cout << "  nsec-audit --monitor [reports_dir]          Start Command Center UI (default: reports)\n";
        return 1;
    }

    // check for monitoring mode
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--monitor") {
            std::string monitorDir = "reports";
            
            // check if the next argument exists and isnt another flag
            if (i + 1 < argc && argv[i+1][0] != '-') {
                monitorDir = argv[++i];
            }
            
            utils::Logger::Info("Starting monitor on directory: " + monitorDir);
            ui::MonitorUI::Start(monitorDir);
            return 0; // exit after monitoring finished
        }
    }

    // normal scan mode argument parsing
    std::vector<fs::path> inputPaths;
    std::string outputPath = "reports/audit_report.json";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            outputPath = argv[++i];
        } else {
            if (fs::exists(arg)) {
                inputPaths.push_back(arg);
            } else {
                utils::Logger::Warn("Path ignored (not found): " + arg);
            }
        }
    }

    if (inputPaths.empty()) {
        utils::Logger::Critical("No valid input paths provided for analysis.");
        return 1;
    }

    // engine execution
    models::Report report;
    core::RuleEngine engine;
    
    RegisterRules(engine);

    auto start = std::chrono::high_resolution_clock::now();
    
    engine.Run(inputPaths, report);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    // output report to file
    PrintSummary(report.Size(), elapsed.count());
    ExportReport(report, outputPath);
}

