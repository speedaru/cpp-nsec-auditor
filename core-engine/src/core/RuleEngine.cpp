#include "pch.h"
#include <nsec/core/RuleEngine.h>
#include <nsec/utils/Logger.h>

namespace nsec::core {
    void RuleEngine::AddRule(std::unique_ptr<ISecurityRule> rule) {
        if (rule) {
            m_rules.push_back(std::move(rule));
        }
    }

    void RuleEngine::Run(const std::vector<fs::path>& targets, models::Report& report) {
        std::vector<fs::path> filesToScan;

        for (const auto& target : targets) {
            if (fs::is_directory(target)) {
                for (const auto& entry : fs::recursive_directory_iterator(target)) {
                    if (IsSupportedFile(entry.path())) {
                        filesToScan.push_back(entry.path());
                    }
                }
            } else if (IsSupportedFile(target)) {
                filesToScan.push_back(fs::absolute(target));
            }
        }

        utils::Logger::Info("Starting analysis on " + std::to_string(filesToScan.size()) + " files...");

        std::vector<std::future<void>> futures;
        for (const auto& file : filesToScan) {
            futures.push_back(std::async(std::launch::async, [this, file, &report]() {
                this->ProcessFile(file, report);
            }));
        }

        for (auto& f : futures) f.get();
    }

    void RuleEngine::ProcessFile(const fs::path& filePath, models::Report& report) const {
        // read file content once
        std::ifstream file(filePath, std::ios::in | std::ios::binary);
        if (!file.is_open()) return;

        // read into string
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        // iterate through all registered rules
        for (const auto& rule : m_rules) {
            rule->Execute(filePath, content, report);
        }
    }

    bool RuleEngine::IsSupportedFile(const fs::path& path) const {
        if (!fs::is_regular_file(path)) return false;
        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return (ext == ".cpp" || ext == ".h" || ext == ".hpp" || ext == ".cc");
    }
}
