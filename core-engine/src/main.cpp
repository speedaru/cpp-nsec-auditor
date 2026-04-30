#include "pch.h"
#include <nsec/models/Report.h>
#include <nsec/core/RuleEngine.h>
#include <nsec/rules/BannedFunctionRule.h>

/** @brief prints the tool usage to the console */
void PrintUsage() {
    std::cout << "nsec-audit: High-Performance Security Static Analysis Tool\n";
    std::cout << "Usage: nsec-audit <target_path>\n";
    std::cout << "Example: nsec-audit ./src\n";
}

int main(int argc, char* argv[]) {
    // cli argument parsing
    if (argc < 2) {
        PrintUsage();
        return 1;
    }

    fs::path targetPath = argv[1];
    if (!fs::exists(targetPath)) {
        std::cerr << "Error: Path '" << targetPath << "' does not exist.\n";
        return 1;
    }

    // initialization
    nsec::models::Report report;
    nsec::core::RuleEngine engine;

    // rule registration
    engine.AddRule(std::make_unique<nsec::rules::BannedFunctionRule>(
        "strcpy", nsec::models::Severity::Critical, "Banned function 'strcpy' detected (Buffer Overflow risk). Use 'strncpy' or 'std::string' instead."));

    engine.AddRule(std::make_unique<nsec::rules::BannedFunctionRule>(
        "sprintf", nsec::models::Severity::Warning, "Function 'sprintf' is prone to buffer overflows. Use 'snprintf' or 'std::format' (C++20)."));

    // execution with timing
    std::cout << ">>> Initializing security scan on: " << fs::absolute(targetPath) << "\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    engine.Run(targetPath, report);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    // console output summary
    size_t issueCount = report.Size();
    
    std::cout << "\n--------------------------------------------------\n";
    std::cout << "              SCAN SUMMARY RESULTS                \n";
    std::cout << "--------------------------------------------------\n";
    std::cout << std::left << std::setw(25) << "Status:" << (issueCount == 0 ? "CLEAN" : "VULNERABILITIES FOUND") << "\n";
    std::cout << std::left << std::setw(25) << "Total Issues:" << issueCount << "\n";
    std::cout << std::left << std::setw(25) << "Execution Time:" << std::fixed << std::setprecision(3) << elapsed.count() << " seconds\n";
    std::cout << "--------------------------------------------------\n";

    if (issueCount > 0) {
        std::cout << "Detailed findings have been exported to 'audit_report.json'.\n";
    } else {
        std::cout << "No security violations were detected in the analyzed files.\n";
    }

    // json persistence
    try {
        std::ofstream outFile("audit_report.json");
        if (outFile.is_open()) {
            outFile << report.ToJson().dump(4);
            outFile.close();
        } else {
            std::cerr << "Error: Could not create 'audit_report.json' for export.\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during JSON export: " << e.what() << "\n";
    }

    return (issueCount > 0) ? 0 : 0; // return 0 as the engine completed successfully
}
