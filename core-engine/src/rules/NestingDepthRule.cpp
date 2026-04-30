#include "pch.h"
#include <nsec/rules/NestingDepthRule.h>

namespace nsec::rules {
    void NestingDepthRule::Execute(const fs::path& filePath, const std::string& content, models::Report& report) const {
        std::istringstream stream(content);
        std::string line;
        
        // context to track the nesting level and pass easily to helper functions
        ScanCtx ctx{
            .filePath = filePath,
            .report = report
        };

        while (std::getline(stream, line)) {
            ctx.currentLineNum++;

            for (char c : line) {
                if (c == '{') {
                    EnterScope(ctx);
                } 
                else if (c == '}') {
                    ExitScope(ctx);
                }
            }
        }
    }

    void NestingDepthRule::EnterScope(ScanCtx& ctx) const {
        // don't add initial scope to depth counter
        if (ctx.inGlobalScope) {
            ctx.inGlobalScope = false;
            return;
        }

        ctx.currentDepth++;
        ctx.scopeStack.push_back({ .startLine = ctx.currentLineNum, .depth = ctx.currentDepth });

        // check for immediate depth violation
        if (ctx.currentDepth > m_maxAllowedDepth) {
            models::Issue issue;
            issue.ruleId = core::RuleId::COMPLEXITY_NESTING_DEPTH;
            issue.severity = models::Severity::Warning;
            issue.filePath = ctx.filePath;
            issue.line = ctx.scopeStack.back().startLine;
            issue.message = "Deep nesting detected (Level " + std::to_string(ctx.currentDepth) + 
                "). Suggestion: Use early returns or refactor into smaller functions.";
            ctx.report.Add(issue);
        }
    }

    void NestingDepthRule::ExitScope(ScanCtx& ctx) const {
        // lambda to decrement depth since we use it in 2 places
        auto decDepth = [&]() {
            if (ctx.currentDepth > 0) ctx.currentDepth--;
            if (ctx.currentDepth == 0) ctx.inGlobalScope = true; // back in global scope
        };

        if (ctx.scopeStack.empty()) {
            decDepth();
            return;
        }

        const ScopeInfo& info = ctx.scopeStack.back();

        // check advanced metric: deep scope (> m_deepScopeDepth) and very long (> m_longScopeThreshold)
        if (ctx.currentDepth >= m_deepScopeDepth && !ctx.reportedThisBranch) {
            uint32_t deepScopeStart = GetDeepScopeStart(ctx)->startLine;
            uint32_t deepScopeLength = CalcDeepScopeLength(ctx);

            if (deepScopeLength > m_longScopeThreshold) {
                models::Issue issue;
                issue.ruleId = core::RuleId::COMPLEXITY_NESTING_DEPTH;
                issue.severity = models::Severity::Warning;
                issue.filePath = ctx.filePath;
                issue.line = deepScopeStart;
                issue.message = "Maintainability Risk: Deeply nested scope (Level " + std::to_string(info.depth) + 
                    ") is too long (" + std::to_string(deepScopeLength) + " lines). " +
                    "This makes the logic hard to follow.";

                ctx.report.Add(issue);
                ctx.reportedThisBranch = true; // mark deep branch as reported
            }
        }
        else {
            ctx.reportedThisBranch = false; // reset flag once we left deep branch
        }

        ctx.scopeStack.pop_back();
        decDepth();
    }

    auto NestingDepthRule::GetDeepScopeStart(ScanCtx& ctx) const -> ScopeInfo* {
        // not in deep scope
        if (ctx.currentDepth < m_deepScopeDepth) {
            return nullptr;
        }

        // get scope where depth == m_deepScopeDepth
        auto it = std::find_if(ctx.scopeStack.rbegin(), ctx.scopeStack.rend(), [&](const ScopeInfo& info){
            return info.depth == m_deepScopeDepth;
        });

        assert(it != ctx.scopeStack.rend());
        return &*it;
    }

    uint32_t NestingDepthRule::CalcDeepScopeLength(ScanCtx& ctx) const {
        ScopeInfo* deepScopeStart = GetDeepScopeStart(ctx);
        if (!deepScopeStart) {
            return 0u;
        }
        
        // calc deep scope length (including subscopes)
        assert(ctx.currentLineNum >= deepScopeStart->startLine);
        return ctx.currentLineNum - deepScopeStart->startLine;
    }
}
