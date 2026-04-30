#pragma once
#include "pch.h"
#include <nsec/core/ISecurityRule.h>

namespace nsec::rules {
    class BannedFunctionRule : public core::ISecurityRule {
    public:
        BannedFunctionRule(std::string functionName, 
                           models::Severity severity, 
                           std::string message);

        core::RuleId GetId() const override { return core::RuleId::SEC_BANNED_FUNCTIONS; }
        
        void Execute(const fs::path& filePath, 
                     const std::string& content, 
                     models::Report& report) const override;

    private:
        std::string m_functionName;
        models::Severity m_severity;
        std::string m_message;
        std::regex m_regex;
    };
}
