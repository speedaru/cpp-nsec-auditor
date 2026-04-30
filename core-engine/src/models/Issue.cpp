#include "pch.h"
#include <nsec/models/Issue.h>

std::string_view nsec::models::ToString(Severity severity) {
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
