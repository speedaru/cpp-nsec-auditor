#pragma once
#include "pch.h"

namespace nsec::core {
    enum class RuleId : uint32_t {
        // 100-199 security
        SEC_BANNED_FUNCTIONS        = 101,

        // 200-249 logic
        LOGIC_TOO_MANY_NEST         = 201, // too much nesting
        
        // 250-299 complexity
        COMPLEXITY_CYCLOMATIC       = 251, // has too much stuff in function
        COMPLEXITY_NESTING_DEPTH    = 252, // too much nesting in a function, or nesting too complex

        // 300-399 memory management
        MEM_C_ALLOC_USAGE           = 301, // c allocators usage (malloc etc... not just calloc)
        MEM_RAW_NEW_DELETE          = 302, // new, delete
        MEM_NULLPTR_USAGE           = 305, // use nullptr instead of NULL
        MEM_UNINIT_PTR              = 310, // pointer not initialized
        MEM_C_STYLE_CAST            = 315  // avoid (type*) casting
    };
}
