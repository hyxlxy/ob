/* Copyright (c) 2025 */
#pragma once

#include "sql/optimizer/rewrite_rule.h"

class VectorIndexScanRewrite : public RewriteRule {
public:
  RC rewrite(std::unique_ptr<LogicalOperator> &oper, bool &change_made) override;
};


