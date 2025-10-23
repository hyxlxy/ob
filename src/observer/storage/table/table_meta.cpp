/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Meiyi & Wangyunlai on 2021/5/12.
//

#include "common/lang/string.h"
#include "common/log/log.h"
#include "storage/table/table_meta.h"
#include "json/json.h"

static const Json::StaticString FIELD_TABLE_ID("table_id");
static const Json::StaticString FIELD_TABLE_NAME("table_name");
static const Json::StaticString FIELD_STORAGE_FORMAT("storage_format");
static const Json::StaticString FIELD_STORAGE_ENGINE("storage_engine");
static const Json::StaticString FIELD_FIELDS("fields");
static const Json::StaticString FIELD_INDEXES("indexes");
static const Json::StaticString FIELD_PRIMARY_KEYS("primary_keys");
static const Json::StaticString FIELD_TABLE_TYPE("table_type");
static const Json::StaticString FIELD_SELECT_SQL("select_sql");

TableMeta::TableMeta(const TableMeta &other)
    : table_id_(other.table_id_),
      name_(other.name_),
      fields_(other.fields_),
      indexes_(other.indexes_),
      storage_format_(other.storage_format_),
      storage_engine_(other.storage_engine_),
      table_type_(other.table_type_),
      select_sql_(other.select_sql_),
      record_size_(other.record_size_)
{}

void TableMeta::swap(TableMeta &other) noexcept
{
  name_.swap(other.name_);
  fields_.swap(other.fields_);
  indexes_.swap(other.indexes_);
  std::swap(record_size_, other.record_size_);
}

RC TableMeta::init(int32_t table_id, const char *name, const vector<FieldMeta> *trx_fields,
                   span<const AttrInfoSqlNode> attributes, const vector<string> &primary_keys, StorageFormat storage_format,
                   StorageEngine storage_engine)
{
  if (common::is_blank(name)) {
    LOG_ERROR("Name cannot be empty");
    return RC::INVALID_ARGUMENT;
  }

  // 允许 0 个用户字段（例如 DROP 掉最后一个用户列后，仍保留系统字段）

  RC rc = RC::SUCCESS;

  int field_offset  = 0;
  int trx_field_num = 0;

  if (trx_fields != nullptr) {
    trx_fields_ = *trx_fields;

    fields_.resize(attributes.size() + trx_fields->size());
    for (size_t i = 0; i < trx_fields->size(); i++) {
      const FieldMeta &field_meta = (*trx_fields)[i];
      fields_[i] = FieldMeta(field_meta.name(), field_meta.type(), field_offset, field_meta.len(), false /*visible*/, field_meta.field_id());
      field_offset += field_meta.len();
    }

    trx_field_num = static_cast<int>(trx_fields->size());
  } else {
    fields_.resize(attributes.size());
  }

  for (size_t i = 0; i < attributes.size(); i++) {
    const AttrInfoSqlNode &attr_info = attributes[i];
    // 统一规范 TEXT 字段的行内长度：offset(8)+length(4)+可空标志(可选1)
    int col_len = attr_info.length;
    if (attr_info.type == AttrType::TEXTS) {
      const int inline_len = static_cast<int>(sizeof(uint64_t) + sizeof(uint32_t));
      col_len = inline_len + (attr_info.nullable ? 1 : 0);
    }
    // `i` is the col_id of fields[i]
    rc = fields_[i + trx_field_num].init(
      attr_info.name.c_str(), attr_info.type, field_offset, col_len, true /*visible*/, i, attr_info.nullable);
    if (OB_FAIL(rc)) {
      LOG_ERROR("Failed to init field meta. table name=%s, field name: %s", name, attr_info.name.c_str());
      return rc;
    }

    field_offset += col_len;
  }

  primary_keys_ = primary_keys;
  record_size_ = field_offset;

  table_id_ = table_id;
  name_     = name;
  storage_format_ = storage_format;
  storage_engine_ = storage_engine;
  LOG_INFO("Sussessfully initialized table meta. table id=%d, name=%s", table_id, name);
  return RC::SUCCESS;
}

RC TableMeta::init_view(int32_t table_id, const char *name, const vector<string> &attr_names,
                        const string &select_sql)
{
  if (table_id < 0) {
    LOG_WARN("invalid table id. table id = %d, table name = %s", table_id, name);
    return RC::INVALID_ARGUMENT;
  }

  if (common::is_blank(name)) {
    LOG_WARN("Name cannot be empty");
    return RC::INVALID_ARGUMENT;
  }

  table_id_ = table_id;
  name_ = name;
  table_type_ = TableType::VIEW;
  select_sql_ = select_sql;

  // 为视图创建字段元数据
  // 注意：这里简化处理，实际应该从SELECT语句中推导字段类型
  for (size_t i = 0; i < attr_names.size(); i++) {
    FieldMeta field_meta;
    RC rc = field_meta.init(attr_names[i].c_str(), AttrType::CHARS, 0, 255, true, false);
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to init field meta. field name=%s", attr_names[i].c_str());
      return rc;
    }
    fields_.push_back(field_meta);
  }

  // 计算记录大小（视图不需要实际存储，但保持接口一致）
  record_size_ = 0;
  for (const FieldMeta &field : fields_) {
    record_size_ += field.len();
  }

  LOG_INFO("Successfully initialized view meta. table id=%d, name=%s", table_id, name);
  return RC::SUCCESS;
}

RC TableMeta::add_index(const IndexMeta &index)
{
  indexes_.push_back(index);
  return RC::SUCCESS;
}

RC TableMeta::drop_index(const char *index_name)
{
  if (common::is_blank(index_name)) {
    return RC::INVALID_ARGUMENT;
  }

  LOG_INFO("Trying to drop index: %s, total indexes: %d", index_name, (int)indexes_.size());
  
  for (auto it = indexes_.begin(); it != indexes_.end(); ++it) {
    LOG_INFO("Checking index: %s vs %s", it->name(), index_name);
    if (strcmp(it->name(), index_name) == 0) {
      LOG_INFO("Found matching index, removing it");
      indexes_.erase(it);
      return RC::SUCCESS;
    }
  }

  LOG_WARN("Index not found in table meta: %s", index_name);
  return RC::SCHEMA_INDEX_NOT_EXIST;
}

RC TableMeta::rename_column(const char *old_name, const char *new_name)
{
  if (common::is_blank(old_name) || common::is_blank(new_name)) {
    return RC::INVALID_ARGUMENT;
  }

  bool found = false;
  for (auto &field : fields_) {
    if (!field.visible()) { continue; }
    if (0 == strcmp(field.name(), old_name)) {
      FieldMeta repl;
      RC rc = repl.init(new_name, field.type(), field.offset(), field.len(), field.visible(), field.field_id(), field.nullable());
      if (rc != RC::SUCCESS) return rc;
      field = repl;
      found = true;
      break;
    }
  }
  if (!found) return RC::SCHEMA_FIELD_NOT_EXIST;

  // update single-field index meta if any
  for (auto &idx : indexes_) {
    if (!idx.fields().empty()) {
      // multi-field index: try to replace matching component
      auto names = idx.fields();
      for (auto &n : names) {
        if (n == old_name) { n = new_name; }
      }
      // rebuild IndexMeta
      vector<FieldMeta> fms;
      for (const auto &nm : names) {
        const FieldMeta *fm = field(nm.c_str());
        if (fm == nullptr) return RC::SCHEMA_FIELD_NOT_EXIST;
        fms.push_back(*fm);
      }
      IndexMeta rebuilt; rebuilt.init(idx.name(), fms, idx.is_unique());
      idx = rebuilt;
    } else if (idx.field() && 0 == strcmp(idx.field(), old_name)) {
      const FieldMeta *fm = field(new_name);
      if (fm == nullptr) return RC::SCHEMA_FIELD_NOT_EXIST;
      IndexMeta rebuilt; rebuilt.init(idx.name(), *fm, idx.is_unique());
      idx = rebuilt;
    }
  }

  return RC::SUCCESS;
}

const char *TableMeta::name() const { return name_.c_str(); }

const FieldMeta *TableMeta::trx_field() const { return &fields_[0]; }

span<const FieldMeta> TableMeta::trx_fields() const
{
  return span<const FieldMeta>(fields_.data(), sys_field_num());
}

const FieldMeta *TableMeta::field(int index) const { return &fields_[index]; }
const FieldMeta *TableMeta::field(const char *name) const
{
  if (nullptr == name) {
    return nullptr;
  }
  for (const FieldMeta &field : fields_) {
    if (0 == strcmp(field.name(), name)) {
      return &field;
    }
  }
  return nullptr;
}

const FieldMeta *TableMeta::find_field_by_offset(int offset) const
{
  for (const FieldMeta &field : fields_) {
    if (field.offset() == offset) {
      return &field;
    }
  }
  return nullptr;
}
int TableMeta::field_num() const { return fields_.size(); }

int TableMeta::sys_field_num() const { return static_cast<int>(trx_fields_.size()); }

const IndexMeta *TableMeta::index(const char *name) const
{
  for (const IndexMeta &index : indexes_) {
    if (0 == strcmp(index.name(), name)) {
      return &index;
    }
  }
  return nullptr;
}

const IndexMeta *TableMeta::find_index_by_field(const char *field) const
{
  for (const IndexMeta &index : indexes_) {
    if (0 == strcmp(index.field(), field)) {
      return &index;
    }
  }
  return nullptr;
}

const IndexMeta *TableMeta::index(int i) const { return &indexes_[i]; }

int TableMeta::index_num() const { return indexes_.size(); }

int TableMeta::record_size() const { return record_size_; }

int TableMeta::serialize(ostream &ss) const
{
  Json::Value table_value;
  table_value[FIELD_TABLE_ID]   = table_id_;
  table_value[FIELD_TABLE_NAME] = name_;
  table_value[FIELD_STORAGE_FORMAT] = static_cast<int>(storage_format_);
  table_value[FIELD_STORAGE_ENGINE] = static_cast<int>(storage_engine_);

  Json::Value fields_value;
  for (const FieldMeta &field : fields_) {
    Json::Value field_value;
    field.to_json(field_value);
    fields_value.append(std::move(field_value));
  }

  table_value[FIELD_FIELDS] = std::move(fields_value);

  Json::Value indexes_value;
  for (const auto &index : indexes_) {
    Json::Value index_value;
    index.to_json(index_value);
    indexes_value.append(std::move(index_value));
  }
  table_value[FIELD_INDEXES] = std::move(indexes_value);

  Json::Value primary_keys_value;
  for (const auto &field : primary_keys_) {
    primary_keys_value.append(field);
  }
  table_value[FIELD_PRIMARY_KEYS] = std::move(primary_keys_value);

  // 添加视图相关字段
  table_value[FIELD_TABLE_TYPE] = static_cast<int>(table_type_);
  if (table_type_ == TableType::VIEW) {
    table_value[FIELD_SELECT_SQL] = select_sql_;
  }

  Json::StreamWriterBuilder builder;
  Json::StreamWriter       *writer = builder.newStreamWriter();

  streampos old_pos = ss.tellp();
  writer->write(table_value, &ss);
  int ret = (int)(ss.tellp() - old_pos);

  delete writer;
  return ret;
}

int TableMeta::deserialize(istream &is)
{
  Json::Value             table_value;
  Json::CharReaderBuilder builder;
  string             errors;

  streampos old_pos = is.tellg();
  if (!Json::parseFromStream(builder, is, &table_value, &errors)) {
    LOG_ERROR("Failed to deserialize table meta. error=%s", errors.c_str());
    return -1;
  }

  const Json::Value &table_id_value = table_value[FIELD_TABLE_ID];
  if (!table_id_value.isInt()) {
    LOG_ERROR("Invalid table id. json value=%s", table_id_value.toStyledString().c_str());
    return -1;
  }

  int32_t table_id = table_id_value.asInt();

  const Json::Value &table_name_value = table_value[FIELD_TABLE_NAME];
  if (!table_name_value.isString()) {
    LOG_ERROR("Invalid table name. json value=%s", table_name_value.toStyledString().c_str());
    return -1;
  }

  string table_name = table_name_value.asString();

  const Json::Value &fields_value = table_value[FIELD_FIELDS];
  // 检查是否为视图，视图可能没有字段信息
  const Json::Value &table_type_value = table_value[FIELD_TABLE_TYPE];
  bool is_view = false;
  if (!table_type_value.empty() && table_type_value.isInt()) {
    is_view = (static_cast<TableType>(table_type_value.asInt()) == TableType::VIEW);
  }

  if (!is_view && (!fields_value.isArray() || fields_value.size() <= 0)) {
    LOG_ERROR("Invalid table meta. fields is not array, json value=%s", fields_value.toStyledString().c_str());
    return -1;
  }

  const Json::Value &storage_format_value = table_value[FIELD_STORAGE_FORMAT];
  if (!storage_format_value.isInt()) {
    LOG_ERROR("Invalid storage format. json value=%s", storage_format_value.toStyledString().c_str());
    return -1;
  }

  int32_t storage_format = storage_format_value.asInt();

  const Json::Value &storage_engine_value = table_value[FIELD_STORAGE_ENGINE];
  if (!storage_engine_value.isInt()) {
    LOG_ERROR("Invalid storage engine. json value=%s", storage_engine_value.toStyledString().c_str());
    return -1;
  }

  int32_t storage_engine = storage_engine_value.asInt();

  RC  rc        = RC::SUCCESS;
  int field_num = fields_value.isArray() ? fields_value.size() : 0;

  vector<FieldMeta> fields(field_num);
  for (int i = 0; i < field_num; i++) {
    FieldMeta &field = fields[i];

    const Json::Value &field_value = fields_value[i];
    rc                             = FieldMeta::from_json(field_value, field);
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to deserialize table meta. table name =%s", table_name.c_str());
      return -1;
    }
  }

  auto comparator = [](const FieldMeta &f1, const FieldMeta &f2) { return f1.offset() < f2.offset(); };
  sort(fields.begin(), fields.end(), comparator);

  table_id_ = table_id;
  storage_format_ = static_cast<StorageFormat>(storage_format);
  storage_engine_ = static_cast<StorageEngine>(storage_engine);
  name_.swap(table_name);
  fields_.swap(fields);

  // 视图没有字段时，record_size_设为0
  if (field_num > 0) {
    record_size_ = fields_.back().offset() + fields_.back().len() - fields_.begin()->offset();
  } else {
    record_size_ = 0;
  }

  for (const FieldMeta &field_meta : fields_) {
    if (!field_meta.visible()) {
      trx_fields_.push_back(field_meta); // 字段加上trx标识更好
    }
  }

  const Json::Value &indexes_value = table_value[FIELD_INDEXES];
  if (!indexes_value.empty()) {
    if (!indexes_value.isArray()) {
      LOG_ERROR("Invalid table meta. indexes is not array, json value=%s", fields_value.toStyledString().c_str());
      return -1;
    }
    const int              index_num = indexes_value.size();
    vector<IndexMeta> indexes(index_num);
    for (int i = 0; i < index_num; i++) {
      IndexMeta &index = indexes[i];

      const Json::Value &index_value = indexes_value[i];
      rc                             = IndexMeta::from_json(*this, index_value, index);
      if (rc != RC::SUCCESS) {
        LOG_ERROR("Failed to deserialize table meta. table name=%s", table_name.c_str());
        return -1;
      }
    }
    indexes_.swap(indexes);
  }

  const Json::Value &primary_keys_value = table_value[FIELD_PRIMARY_KEYS];
  if (!primary_keys_value.empty()) {
    if (!primary_keys_value.isArray()) {
      LOG_ERROR("Invalid table meta. primary keys is not array, json value=%s", fields_value.toStyledString().c_str());
      return -1;
    }
    const int              primary_key_num = primary_keys_value.size();
    vector<string> primary_keys(primary_key_num);
    for (int i = 0; i < primary_key_num; i++) {
      const Json::Value &field_name_value = primary_keys_value[i];
      if (!field_name_value.isString()) {
        LOG_ERROR("Invalid table meta. primary key name is not string, json value=%s",
                  field_name_value.toStyledString().c_str());
        return -1;
      }
      string field_name = field_name_value.asString();
      primary_keys.push_back(field_name);
    }
    primary_keys_.swap(primary_keys);
  }

  // 处理视图相关字段
  if (!table_type_value.empty()) {
    if (!table_type_value.isInt()) {
      LOG_ERROR("Invalid table meta. table type is not int, json value=%s", table_type_value.toStyledString().c_str());
      return -1;
    }
    table_type_ = static_cast<TableType>(table_type_value.asInt());
  } else {
    table_type_ = TableType::TABLE; // 默认为普通表
  }

  if (table_type_ == TableType::VIEW) {
    const Json::Value &select_sql_value = table_value[FIELD_SELECT_SQL];
    if (!select_sql_value.empty()) {
      if (!select_sql_value.isString()) {
        LOG_ERROR("Invalid table meta. select sql is not string, json value=%s", select_sql_value.toStyledString().c_str());
        return -1;
      }
      select_sql_ = select_sql_value.asString();
    }
  }

  return (int)(is.tellg() - old_pos);
}

int TableMeta::get_serial_size() const { return -1; }

void TableMeta::to_string(string &output) const {}

void TableMeta::desc(ostream &os) const
{
  os << name_ << '(' << endl;
  for (const auto &field : fields_) {
    os << '\t';
    field.desc(os);
    os << endl;
  }

  for (const auto &index : indexes_) {
    os << '\t';
    index.desc(os);
    os << endl;
  }
  os << ')' << endl;
}
