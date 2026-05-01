#include "pch.h"
#include <nsec/models/Report.h>
#include <nsec/core/RuleEngine.h>
#include <nsec/utils/Logger.h>

// rules
#include <nsec/rules/BannedFunctionRule.h>
#include <nsec/rules/NestingDepthRule.h>

using namespace nsec;

void RegisterRules(core::RuleEngine& engine) {
    engine.AddRule(std::make_unique<rules::BannedFunctionRule>(
        "strcpy", models::Severity::Critical, "Banned 'strcpy' (Overflow risk)."));
    
    engine.AddRule(std::make_unique<rules::BannedFunctionRule>(
        "sprintf", models::Severity::Warning, "'sprintf' is unsafe. Use 'snprintf'."));
    
    engine.AddRule(std::make_unique<rules::NestingDepthRule>());
}

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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: nsec-audit <paths...> [-o output.json]\n";
        return 1;
    }

    std::vector<fs::path> inputPaths;
    std::string outputPath = "reports/audit_report.json";

    // Simple Argument Parser
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
        utils::Logger::Critical("No valid input paths provided.");
        return 1;
    }

    models::Report report;
    core::RuleEngine engine;
    
    RegisterRules(engine);

    auto start = std::chrono::high_resolution_clock::now();
    
    engine.Run(inputPaths, report);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    // Summary output
    std::cout << "\n--------------------------------------------------\n";
    utils::Logger::Info("Scan Completed in " + std::to_string(elapsed.count()) + "s");
    utils::Logger::Info("Total Issues Found: " + std::to_string(report.Size()));
    
    if (report.Size() > 0) {
        utils::Logger::Warn("Security status: VULNERABLE");
    } else {
        utils::Logger::Info("Security status: CLEAN");
    }
    std::cout << "--------------------------------------------------\n";

    ExportReport(report, outputPath);
}
