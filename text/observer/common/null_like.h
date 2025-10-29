/* Utility for detecting NULL-like numerics used by MiniOB to represent exceptional results (e.g., division by zero) */
#pragma once

#include "common/value.h"
#include "common/lang/limits.h"

inline bool is_null_like_numeric(const Value &v) {
  // Check for NULL (UNDEFINED type) - this is the standard SQL NULL
  if (v.attr_type() == AttrType::UNDEFINED) return true;

  // Legacy check for special float values (kept for compatibility)
  if (v.attr_type() == AttrType::FLOATS) {
    return v.get_float() == numeric_limits<float>::max();
  }

  return false;
}

