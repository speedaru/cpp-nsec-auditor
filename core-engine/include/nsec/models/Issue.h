#pragma once
#include "pch.h"
#include <nsec/core/RuleDefinitions.h>

namespace nsec::models {
    enum class Severity : std::uint8_t {
        Info,
        Warning,
        Critical,
    };

    std::string_view ToString(Severity severity);

    struct Issue {
        core::RuleId ruleId;    // identifier of the rule that raised this issue
        Severity severity;      // severity level
        fs::path filePath;      // path of the analysed file
        uint32_t line;          // line number where the issue was found (1-based)
        std::string message;    // description of the issue
    };
}
