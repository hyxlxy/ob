#include "common/type/boolean_type.h"
#include "common/value.h"
#include "common/log/log.h"
#include <algorithm>

int BooleanType::compare(const Value &left, const Value &right) const {
  bool l = left.get_boolean();
  bool r = right.get_boolean();
  if (l == r) return 0;
  return l ? 1 : -1;
}

RC BooleanType::cast_to(const Value &val, AttrType type, Value &result) const {
  switch (type) {
    case AttrType::BOOLEANS: {
      result.set_boolean(val.get_boolean());
      return RC::SUCCESS;
    }
    case AttrType::INTS: {
      result.set_int(val.get_boolean() ? 1 : 0);
      return RC::SUCCESS;
    }
    case AttrType::FLOATS: {
      result.set_float(val.get_boolean() ? 1.0f : 0.0f);
      return RC::SUCCESS;
    }
    case AttrType::CHARS: {
      result.set_string(val.get_boolean() ? "1" : "0");
      return RC::SUCCESS;
    }
    default:
      return RC::UNSUPPORTED;
  }
}

RC BooleanType::to_string(const Value &val, string &result) const {
  result = val.get_boolean() ? "1" : "0";
  return RC::SUCCESS;
}

RC BooleanType::set_value_from_str(Value &val, const string &data) const {
  if (data.empty()) {
    val.set_boolean(false);
    return RC::SUCCESS;
  }
  std::string s = data;
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  if (s == "1" || s == "true" || s == "t" || s == "yes" || s == "y") {
    val.set_boolean(true);
  } else if (s == "0" || s == "false" || s == "f" || s == "no" || s == "n") {
    val.set_boolean(false);
  } else {
    // Try numeric fallback
    try {
      float f = std::stof(s);
      val.set_boolean(!(f > -1e-6f && f < 1e-6f));
    } catch (...) {
      val.set_boolean(!s.empty());
    }
  }
  return RC::SUCCESS;
}

int BooleanType::cast_cost(AttrType type) {
  if (type == AttrType::BOOLEANS) return 0;
  if (type == AttrType::INTS || type == AttrType::FLOATS || type == AttrType::CHARS) return 1;
  return INT32_MAX;
}

