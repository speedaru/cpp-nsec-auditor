#pragma once
#include "pch.h"
#include <nsec/models/Report.h>
#include <nsec/core/RuleDefinitions.h>

namespace nsec::core {
    /** abstract base class for all security rules */
    class ISecurityRule {
    public:
        virtual ~ISecurityRule() = default;

        /**
         * @return the unique numeric identifier for this rule
         */
        virtual RuleId GetId() const = 0;

        /**
         * @brief Executes the rule logic on a specific file's content.
         * 
         * @param filePath The filesystem path to the file being analyzed.
         * @param content The pre-loaded string content of the file.
         * @param report The thread-safe report object where findings are stored.
         */
        virtual void Execute(const fs::path& filePath, const std::string& content, models::Report& report) const = 0;
    };
}
