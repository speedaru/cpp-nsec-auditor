#include "pch.h"
#include <nsec/rules/BannedFunctionRule.h>

namespace nsec::rules {
    BannedFunctionRule::BannedFunctionRule(std::string functionName, models::Severity severity, std::string message)
        : m_functionName(std::move(functionName)), m_severity(severity), m_message(std::move(message)) 
    {
        // use word boundaries to match exact function names
        m_regex = std::regex("\\b" + m_functionName + "\\b");
    }

    void BannedFunctionRule::Execute(const fs::path& filePath, const std::string& content, models::Report& report) const {
        auto words_begin = std::sregex_iterator(content.begin(), content.end(), m_regex);
        auto words_end = std::sregex_iterator();

        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            std::smatch match = *i;
            size_t pos = match.position();

            // calc line number
            uint32_t lineCount = 1;
            for (size_t j = 0; j < pos; ++j) {
                if (content[j] == '\n') lineCount++;
            }

            models::Issue issue;
            issue.ruleId = GetId();
            issue.severity = m_severity;
            issue.filePath = filePath;
            issue.line = lineCount;
            issue.message = m_message + " (Found: " + m_functionName + ")";

            report.Add(issue);
        }
    }
}
