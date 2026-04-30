#include "pch.h"
#include <nsec/rules/ComplexityRule.h>
#include <regex>

namespace nsec::rules {
    void ComplexityRule::Execute(const fs::path& filePath, const std::string& content, models::Report& report) const {
        // decision point patterns
        static const std::vector<std::string> patterns = {
            "\\bif\\b", "\\belse\\b", "\\bfor\\b", "\\bwhile\\b", 
            "\\bswitch\\b", "\\bcase\\b", "&&", "\\|\\|"
        };

        uint32_t totalComplexity = 0;

        for (const auto& pattern : patterns) {
            std::regex re(pattern);
            auto begin = std::sregex_iterator(content.begin(), content.end(), re);
            auto end = std::sregex_iterator();
            totalComplexity += std::distance(begin, end);
        }

        if (totalComplexity > m_threshold) {
            models::Issue issue;
            issue.ruleId = GetId();
            issue.severity = models::Severity::Warning;
            issue.filePath = filePath;
            issue.line = 0; // file-level issue
            issue.message = "High Cyclomatic Complexity detected (" + 
                            std::to_string(totalComplexity) + 
                            "). This file may be difficult to audit and prone to bugs.";

            report.Add(issue);
        }
    }
}
