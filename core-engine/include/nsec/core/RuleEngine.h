#pragma once
#include "pch.h"
#include <nsec/core/ISecurityRule.h>

namespace nsec::core {
    class RuleEngine {
    public:
        RuleEngine() = default;

        /** registers a new rule into the engine */
        void AddRule(std::unique_ptr<ISecurityRule> rule);

        /**
         * @brief scans a list of specific files or a whole directory
         */
        void Run(const std::vector<fs::path>& targets, models::Report& report);

    private:
        void ProcessFile(const fs::path& filePath, models::Report& report) const;
        bool IsSupportedFile(const fs::path& path) const;

    private:
        std::vector<std::unique_ptr<ISecurityRule>> m_rules;
    };
}
