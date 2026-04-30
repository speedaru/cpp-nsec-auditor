#pragma once
#include "pch.h"
#include <nsec/models/Issue.h>

namespace nsec::models {
    class Report {
    public:
        /** add an issue */
        void Add(const Issue& issue);

        /** get a copy of issues */
        std::vector<Issue> GetIssues() const;
        
        /** get number of issues */
        size_t Size() const;

        bool IsEmpty() const;

        nlohmann::json ToJson() const;

    private:
        mutable std::mutex m_mutex;
        std::vector<Issue> m_issues;
    };
}
