/* Utility for detecting NULL-like numerics used by MiniOB to represent exceptional results (e.g., division by zero) */
#pragma once

#include "common/value.h"
#include "common/lang/limits.h"

inline bool is_null_like_numeric(const Value &v) {
  if (v.attr_type() != AttrType::FLOATS) return false;
  return v.get_float() == numeric_limits<float>::max();
}

