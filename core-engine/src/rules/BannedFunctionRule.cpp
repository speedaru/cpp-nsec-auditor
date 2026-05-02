#include "pch.h"
#include <nsec/rules/BannedFunctionRule.h>

namespace nsec::rules {
    // helpers for state detection
    namespace {
        inline bool IsSingleLineCommentStart(const std::string& s, size_t i) {
            return i + 1 < s.length() && s[i] == '/' && s[i + 1] == '/';
        }

        inline bool IsMultiLineCommentStart(const std::string& s, size_t i) {
            return i + 1 < s.length() && s[i] == '/' && s[i + 1] == '*';
        }

        inline bool IsMultiLineCommentEnd(const std::string& s, size_t i) {
            return i + 1 < s.length() && s[i] == '*' && s[i + 1] == '/';
        }

        inline bool IsStringDelimiter(const std::string& s, size_t i) {
            return s[i] == '"';
        }

        inline bool IsEscapedChar(const std::string& s, size_t i) {
            return s[i] == '\\' && i + 1 < s.length();
        }
    }

    BannedFunctionRule::BannedFunctionRule(std::string functionName, models::Severity severity, std::string message)
        : m_functionName(std::move(functionName)), m_severity(severity), m_message(std::move(message)) 
    {
        // match the function name followed by optional whitespace and an opening parenthesis
        // this distinguishes a call from a mention or variable name
        m_regex = std::regex("\\b" + m_functionName + "\\s*\\(");
    }

    /**
     * @brief Maps sections of code (comments/strings) that should be ignored by rules.
     */
    auto BannedFunctionRule::GetForbiddenRanges(const std::string& content) const -> RangeList {
        RangeList ranges;
        bool inString = false, inSingleComment = false, inMultiComment = false;
        size_t startIdx = 0;

        for (size_t i = 0; i < content.length(); ++i) {
            if (inMultiComment) {
                if (IsMultiLineCommentEnd(content, i)) {
                    ranges.push_back({startIdx, i + 2});
                    inMultiComment = false;
                    i++;
                }
                continue;
            }

            if (inSingleComment) {
                if (content[i] == '\n') {
                    ranges.push_back({startIdx, i});
                    inSingleComment = false;
                }
                continue;
            }

            if (inString) {
                if (IsEscapedChar(content, i)) { i++; continue; } 
                if (IsStringDelimiter(content, i)) {
                    ranges.push_back({startIdx, i + 1});
                    inString = false;
                }
                continue;
            }

            // Entry Point Detection
            if (IsSingleLineCommentStart(content, i)) {
                inSingleComment = true;
                startIdx = i++;
            } else if (IsMultiLineCommentStart(content, i)) {
                inMultiComment = true;
                startIdx = i++;
            } else if (IsStringDelimiter(content, i)) {
                inString = true;
                startIdx = i;
            }
        }
        
        if (inSingleComment || inMultiComment || inString) {
            ranges.push_back({startIdx, content.length()});
        }

        return ranges;
    }

    void BannedFunctionRule::Execute(const fs::path& filePath, const std::string& content, models::Report& report) const {
        RangeList forbidden = GetForbiddenRanges(content);

        auto words_begin = std::sregex_iterator(content.begin(), content.end(), m_regex);
        auto words_end = std::sregex_iterator();

        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            std::smatch match = *i;
            size_t pos = match.position();

            // Binary Search using the RangeList typedef
            auto it = std::lower_bound(forbidden.begin(), forbidden.end(), SourceRange{pos, pos}, 
                [](const SourceRange& range, const SourceRange& val) {
                    return range.second <= val.first;
                });

            bool isForbidden = (it != forbidden.end() && it->first <= pos && pos < it->second);
            if (isForbidden) continue;

            uint32_t lineCount = 1;
            for (size_t j = 0; j < pos; ++j) {
                if (content[j] == '\n') lineCount++;
            }

            models::Issue issue;
            issue.ruleId = GetId();
            issue.severity = m_severity;
            issue.filePath = filePath;
            issue.line = lineCount;
            issue.message = m_message + " (Call detected: " + m_functionName + ")";

            report.Add(issue);
        }
    }
}
