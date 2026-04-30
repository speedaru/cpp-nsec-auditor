#pragma once
#include "pch.h"

namespace nsec::core {
    enum class RuleId : uint32_t {
        // 100-199 security
        SEC_BANNED_FUNCTIONS        = 101,

        // 200-249 logic
        LOGIC_TOO_MANY_NEST         = 201, // too much nesting
        
        // 250-299 complexity
        COMPLEXITY_CYCLOMATIC       = 251,

        // 300-399 memory management
        MEM_C_ALLOC_USAGE           = 301, // c allocators usage (malloc etc... not just calloc)
    };
}
