#include "pch.h"
#include <nsec/models/Report.h>

namespace nsec::models {
    void Report::Add(const Issue& issue) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_issues.push_back(issue); // copy issue
    }

    std::vector<Issue> Report::GetIssues() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_issues;
    }

    size_t Report::Size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_issues.size();
    }

    bool Report::IsEmpty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_issues.empty();
    }

    nlohmann::json Report::ToJson() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        nlohmann::json reportJson;
        reportJson["summary"] = {
            {"total_issues", m_issues.size()},
            {"status", m_issues.empty() ? "Secure" : "Vulnerable"}
        };

        reportJson["issues"] = nlohmann::json::array();
        
        for (const auto& issue : m_issues) {
            nlohmann::json issueJson = {
                {"ruleId", issue.ruleId},
                {"severity", ToString(issue.severity)},
                {"file", issue.filePath.string()},
                {"line", issue.line},
                {"message", issue.message}
            };
            reportJson["issues"].push_back(issueJson);
        }

        return reportJson;
    }

}
