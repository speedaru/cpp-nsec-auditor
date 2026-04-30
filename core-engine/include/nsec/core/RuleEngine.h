#pragma once
#include "pch.h"
#include <nsec/core/ISecurityRule.h>

namespace nsec::core {
    class RuleEngine {
    public:
        RuleEngine() = default;

        /** registers a new rule into the engine */
        void AddRule(std::unique_ptr<ISecurityRule> rule);

        /** recursively scans the target path and runs all registered rules */
        void Run(const fs::path& targetPath, models::Report& report);

    private:
        void ProcessFile(const fs::path& filePath, models::Report& report) const;

    private:
        std::vector<std::unique_ptr<ISecurityRule>> m_rules;
    };
}
