#pragma once

#include "common/type/data_type.h"

class BooleanType : public DataType {
public:
  BooleanType() : DataType(AttrType::BOOLEANS) {}
  virtual ~BooleanType() = default;

  int compare(const Value &left, const Value &right) const override;

  RC add(const Value &, const Value &, Value &) const override { return RC::UNSUPPORTED; }
  RC subtract(const Value &, const Value &, Value &) const override { return RC::UNSUPPORTED; }
  RC multiply(const Value &, const Value &, Value &) const override { return RC::UNSUPPORTED; }
  RC divide(const Value &, const Value &, Value &) const override { return RC::UNSUPPORTED; }
  RC negative(const Value &, Value &) const override { return RC::UNSUPPORTED; }

  RC cast_to(const Value &val, AttrType type, Value &result) const override;
  RC to_string(const Value &val, string &result) const override;
  RC set_value_from_str(Value &val, const string &data) const override;

  int cast_cost(AttrType type) override;
};

