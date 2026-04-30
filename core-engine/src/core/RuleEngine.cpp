#include "pch.h"
#include <nsec/core/RuleEngine.h>

namespace nsec::core {
    void RuleEngine::AddRule(std::unique_ptr<ISecurityRule> rule) {
        if (rule) {
            m_rules.push_back(std::move(rule));
        }
    }

    void RuleEngine::Run(const fs::path& targetPath, models::Report& report) {
        if (!fs::exists(targetPath)) {
            return;
        }

        std::vector<std::future<void>> futures;

        for (const auto& entry : fs::recursive_directory_iterator(targetPath)) {
            if (!entry.is_regular_file()) continue;

            const auto& path = entry.path();
            auto ext = path.extension().string();
            
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            if (ext == ".cpp" || ext == ".h" || ext == ".hpp" || ext == ".cc" || ext == ".cxx") {
                // launch analysis of each file in parallel
                futures.push_back(std::async(std::launch::async, [this, path, &report]() {
                    this->ProcessFile(path, report);
                }));
            }
        }

        // wait for all threads to finish
        for (auto& f : futures) {
            f.get();
        }
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
}
