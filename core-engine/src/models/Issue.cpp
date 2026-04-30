#include "pch.h"
#include <nsec/models/Issue.h>

namespace nsec::models {
    std::string_view ToString(Severity severity) {
        switch (severity) {
            case Severity::Info:
                return "Info";
            case Severity::Warning:
                return "Warning";
            case Severity::Critical:
                return "Critical";
        }
        return "Unknown";
    }
}
