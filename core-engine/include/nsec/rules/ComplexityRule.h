#pragma once
#include "pch.h"
#include <nsec/core/ISecurityRule.h>

namespace nsec::rules {
    class ComplexityRule : public core::ISecurityRule {
    public:
        ComplexityRule(uint32_t threshold = 10) : m_threshold(threshold) {}

        core::RuleId GetId() const override { return core::RuleId::COMPLEXITY_CYCLOMATIC; }

        void Execute(const fs::path& filePath, 
                     const std::string& content, 
                     models::Report& report) const override;

    private:
        uint32_t m_threshold;
    };
}
