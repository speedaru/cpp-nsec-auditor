#pragma once
#include "pch.h"
#include <nsec/core/ISecurityRule.h>
#include <nsec/core/RuleDefinitions.h>

namespace nsec::rules {
    class NestingDepthRule : public core::ISecurityRule {
    public:
        /**
         * @param maxAllowedDepth max depth allowed before warning
         * @param deepScopeDepth depth at which a we're considered in a "deep scope"
         * @param longScopeThreshold max lines allowed in a deep scope
         */
        NestingDepthRule(uint32_t maxAllowedDepth = 4,
                         uint32_t deepScopeDepth = 3,
                         uint32_t longScopeThreshold = 25)
            : m_maxAllowedDepth(maxAllowedDepth),
              m_deepScopeDepth(deepScopeDepth),
              m_longScopeThreshold(longScopeThreshold) {}

        core::RuleId GetId() const override { return core::RuleId::COMPLEXITY_NESTING_DEPTH; }

        void Execute(const fs::path& filePath, 
                     const std::string& content, 
                     models::Report& report) const override;

    private:
        struct ScopeInfo {
            uint32_t startLine;
            uint32_t depth;
        };

        /** context for when scanning a file (used to pass to function below) */
        struct ScanCtx {
            const fs::path& filePath;
            models::Report& report;

            std::vector<ScopeInfo> scopeStack{};
            uint32_t currentLineNum{};
            uint32_t currentDepth{};
            bool inGlobalScope{ true };
            bool reportedThisBranch; // avoid spamming report, only report once per individual deep branch
        };

        void EnterScope(ScanCtx& ctx) const;
        void ExitScope(ScanCtx& ctx) const;

        /** get scope where depth == m_deepScopeDepth */
        auto GetDeepScopeStart(ScanCtx& ctx) const -> ScopeInfo*;

        /** calc deep scope length (including subscopes) */
        uint32_t CalcDeepScopeLength(ScanCtx& ctx) const;


    private:
        uint32_t m_maxAllowedDepth;
        uint32_t m_deepScopeDepth;
        uint32_t m_longScopeThreshold;
    };

}
