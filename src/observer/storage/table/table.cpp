/* Copyright (c) 2021 Xie Meiyi(xiemeiyi@hust.edu.cn) and OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Meiyi & Wangyunlai on 2021/5/13.
//

#include <limits.h>
#include <string.h>

#include "common/defs.h"
#include "common/lang/string.h"
#include "common/lang/span.h"
// #include "common/lang/algorithm.h"
#include "common/log/log.h"
#include "common/global_context.h"
#include "storage/db/db.h"
#include "storage/buffer/disk_buffer_pool.h"
#include "storage/common/condition_filter.h"
#include "storage/common/meta_util.h"
#include "storage/index/index.h"
#include "storage/record/record_manager.h"
#include "storage/record/heap_record_scanner.h"
#include "storage/table/table.h"
#include "storage/trx/trx.h"
// #include "storage/record/heap_record_scanner.h"
// #include "storage/record/lsm_record_scanner.h"
#include "storage/table/heap_table_engine.h"
#include "storage/table/lsm_table_engine.h"
#include "storage/record/record_manager.h"
#include <filesystem>

Table::~Table()
{
}

RC Table::create(Db *db, int32_t table_id, const char *path, const char *name, const char *base_dir,
    span<const AttrInfoSqlNode> attributes, const vector<string> &primary_keys, StorageFormat storage_format, StorageEngine storage_engine)
{
  if (table_id < 0) {
    LOG_WARN("invalid table id. table_id=%d, table_name=%s", table_id, name);
    return RC::INVALID_ARGUMENT;
  }

  if (common::is_blank(name)) {
    LOG_WARN("Name cannot be empty");
    return RC::INVALID_ARGUMENT;
  }
  LOG_INFO("Begin to create table %s:%s", base_dir, name);

  if (attributes.size() == 0) {
    LOG_WARN("Invalid arguments. table_name=%s, attribute_count=%d", name, attributes.size());
    return RC::INVALID_ARGUMENT;
  }

  RC rc = RC::SUCCESS;

  // 使用 table_name.table记录一个表的元数据
  // 判断表文件是否已经存在
  int fd = ::open(path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
  if (fd < 0) {
    if (EEXIST == errno) {
      LOG_ERROR("Failed to create table file, it has been created. %s, EEXIST, %s", path, strerror(errno));
      return RC::SCHEMA_TABLE_EXIST;
    }
    LOG_ERROR("Create table file failed. filename=%s, errmsg=%d:%s", path, errno, strerror(errno));
    return RC::IOERR_OPEN;
  }

  close(fd);

  // 创建文件
  const vector<FieldMeta> *trx_fields = db->trx_kit().trx_fields();
  if ((rc = table_meta_.init(table_id, name, trx_fields, attributes, primary_keys, storage_format, storage_engine)) != RC::SUCCESS) {
    LOG_ERROR("Failed to init table meta. name:%s, ret:%d", name, rc);
    return rc;  // delete table file
  }

  fstream fs;
  fs.open(path, ios_base::out | ios_base::binary);
  if (!fs.is_open()) {
    LOG_ERROR("Failed to open file for write. file name=%s, errmsg=%s", path, strerror(errno));
    return RC::IOERR_OPEN;
  }

  // 记录元数据到文件中
  table_meta_.serialize(fs);
  fs.close();

  db_       = db;

  string             data_file = table_data_file(base_dir, name);
  BufferPoolManager &bpm       = db->buffer_pool_manager();
  rc                           = bpm.create_file(data_file.c_str());
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to create disk buffer pool of data file. file name=%s", data_file.c_str());
    return rc;
  }

  // Ensure LOB file exists for TEXT columns
  bool has_text = false;
  for (int i = table_meta_.sys_field_num(); i < table_meta_.field_num(); ++i) {
    const FieldMeta *f = table_meta_.field(i);
    if (f && f->type() == AttrType::TEXTS) { has_text = true; break; }
  }
  if (has_text) {
    string lob_file = table_text_lob_file(base_dir, name);
    int lobfd = ::open(lob_file.c_str(), O_WRONLY | O_CREAT | O_CLOEXEC, 0600);
    if (lobfd < 0) {
      LOG_ERROR("Failed to create lob file. file name=%s, errmsg=%d:%s", lob_file.c_str(), errno, strerror(errno));
      return RC::IOERR_OPEN;
    }
    ::close(lobfd);
  }

  // rc = init_record_handler(base_dir);
  // if (rc != RC::SUCCESS) {
  //   LOG_ERROR("Failed to create table %s due to init record handler failed.", data_file.c_str());
  //   // don't need to remove the data_file
  //   return rc;
  // }

  if (table_meta_.storage_engine() == StorageEngine::HEAP) {
    engine_ = make_unique<HeapTableEngine>(&table_meta_, db_, this);
  } else if (table_meta_.storage_engine() == StorageEngine::LSM) {
    engine_ = make_unique<LsmTableEngine>(&table_meta_, db_, this);
  } else {
    rc = RC::UNSUPPORTED;
    LOG_WARN("Unsupported storage engine type: %d", table_meta_.storage_engine());
    return rc;
  }
  rc = engine_->open();
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to open table %s due to engine open failed.", data_file.c_str());
    return rc;
  }

  LOG_INFO("Successfully create table %s:%s", base_dir, name);
  return rc;
}

RC Table::create_view(Db *db, int32_t table_id, const char *path, const char *name,
                      const vector<string> &attr_names, const string &select_sql)
{
  if (table_id < 0) {
    LOG_WARN("invalid table id. table_id=%d, table_name=%s", table_id, name);
    return RC::INVALID_ARGUMENT;
  }

  if (common::is_blank(name)) {
    LOG_WARN("Name cannot be empty");
    return RC::INVALID_ARGUMENT;
  }

  LOG_INFO("Begin to create view %s", name);

  // 创建元数据文件
  int fd = ::open(path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
  if (fd < 0) {
    if (EEXIST == errno) {
      LOG_ERROR("Failed to create view file, it has been created. %s, EEXIST, %s", path, strerror(errno));
      return RC::SCHEMA_TABLE_EXIST;
    }
    LOG_ERROR("Create view file failed. filename=%s, errmsg=%d:%s", path, errno, strerror(errno));
    return RC::IOERR_OPEN;
  }
  close(fd);

  // 初始化视图元数据
  RC rc = table_meta_.init_view(table_id, name, attr_names, select_sql);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to init view meta. name:%s, ret:%d", name, rc);
    return rc;
  }

  // 保存元数据到文件
  fstream fs;
  fs.open(path, ios_base::out | ios_base::binary);
  if (!fs.is_open()) {
    LOG_ERROR("Failed to open file for write. file name=%s, errmsg=%s", path, strerror(errno));
    return RC::IOERR_OPEN;
  }

  table_meta_.serialize(fs);
  fs.close();

  db_ = db;

  LOG_INFO("Successfully created view %s", name);
  return RC::SUCCESS;
}

RC Table::open(Db *db, const char *meta_file, const char *base_dir)
{
  // 加载元数据文件
  fstream fs;
  string  meta_file_path = string(base_dir) + common::FILE_PATH_SPLIT_STR + meta_file;
  fs.open(meta_file_path, ios_base::in | ios_base::binary);
  if (!fs.is_open()) {
    LOG_ERROR("Failed to open meta file for read. file name=%s, errmsg=%s", meta_file_path.c_str(), strerror(errno));
    return RC::IOERR_OPEN;
  }
  if (table_meta_.deserialize(fs) < 0) {
    LOG_ERROR("Failed to deserialize table meta. file name=%s", meta_file_path.c_str());
    fs.close();
    return RC::INTERNAL;
  }
  fs.close();

  db_       = db;

  // // 加载数据文件
  // RC rc = init_record_handler(base_dir);
  // if (rc != RC::SUCCESS) {
  //   LOG_ERROR("Failed to open table %s due to init record handler failed.", base_dir);
  //   // don't need to remove the data_file
  //   return rc;
  // }
  RC rc = RC::SUCCESS;

  // 视图不需要存储引擎
  if (!table_meta_.is_view()) {
    if (table_meta_.storage_engine() == StorageEngine::HEAP) {
      engine_ = make_unique<HeapTableEngine>(&table_meta_, db_, this);
    }  else if (table_meta_.storage_engine() == StorageEngine::LSM) {
      engine_ = make_unique<LsmTableEngine>(&table_meta_, db_, this);
    } else {
      rc = RC::UNSUPPORTED;
      LOG_ERROR("Unsupported storage engine type: %d", table_meta_.storage_engine());
      return rc;
    }

    rc = engine_->open();
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to open table %s due to engine open failed.", base_dir);
      return rc;
    }
  }

  return rc;
}

RC Table::append_text_lob(const char *data, uint32_t len, uint64_t &offset)
{
  if (is_view()) {
    return RC::UNIMPLEMENTED;
  }
  if (len > 65535) {
    return RC::INVALID_ARGUMENT;
  }
  string lob_file = table_text_lob_file(db_->path().c_str(), table_meta_.name());
  int fd = ::open(lob_file.c_str(), O_WRONLY | O_CREAT, 0600);
  if (fd < 0) {
    LOG_WARN("open lob file failed %s", lob_file.c_str());
    return RC::IOERR_OPEN;
  }
  // append: seek to end
  off_t end = ::lseek(fd, 0, SEEK_END);
  if (end < 0) {
    ::close(fd);
    return RC::IOERR_SEEK;
  }
  uint32_t be_len = len; // store as native endian simple
  // write header fully
  size_t wrote = 0;
  const char *hp = reinterpret_cast<const char *>(&be_len);
  while (wrote < sizeof(uint32_t)) {
    ssize_t w = ::write(fd, hp + wrote, sizeof(uint32_t) - wrote);
    if (w <= 0) { ::close(fd); return RC::IOERR_WRITE; }
    wrote += (size_t)w;
  }
  // write payload fully
  size_t total = 0;
  while (total < len) {
    ssize_t w = ::write(fd, data + total, len - total);
    if (w <= 0) { ::close(fd); return RC::IOERR_WRITE; }
    total += (size_t)w;
  }
  ::fsync(fd);
  ::close(fd);
  offset = static_cast<uint64_t>(end);
  return RC::SUCCESS;
}

RC Table::read_text_lob(uint64_t offset, uint32_t len, std::string &out) const
{
  if (is_view()) {
    return RC::UNIMPLEMENTED;
  }
  string lob_file = table_text_lob_file(db_->path().c_str(), table_meta_.name());
  int fd = ::open(lob_file.c_str(), O_RDONLY);
  if (fd < 0) {
    return RC::IOERR_OPEN;
  }
  if (::lseek(fd, (off_t)offset, SEEK_SET) < 0) {
    ::close(fd);
    return RC::IOERR_SEEK;
  }
  uint32_t stored_len = 0;
  size_t got = 0;
  char *hp = reinterpret_cast<char *>(&stored_len);
  while (got < sizeof(uint32_t)) {
    ssize_t r = ::read(fd, hp + got, sizeof(uint32_t) - got);
    if (r <= 0) { ::close(fd); return RC::IOERR_READ; }
    got += (size_t)r;
  }
  // Always use stored length to read full payload for robustness
  out.resize(stored_len);
  size_t have = 0;
  while (have < stored_len) {
    ssize_t r = ::read(fd, out.data() + have, stored_len - have);
    if (r <= 0) { ::close(fd); return RC::IOERR_READ; }
    have += (size_t)r;
  }
  ::close(fd);
  return RC::SUCCESS;
}

RC Table::insert_record(Record &record)
{
  if (is_view()) {
    LOG_WARN("Cannot insert into view: %s", name());
    return RC::UNIMPLEMENTED;
  }
  return engine_->insert_record(record);
}

RC Table::visit_record(const RID &rid, function<bool(Record &)> visitor)
{
  if (is_view()) {
    LOG_WARN("Cannot visit record in view: %s", name());
    return RC::UNIMPLEMENTED;
  }
  return engine_->visit_record(rid, visitor);
}

RC Table::insert_record_with_trx(Record &record, Trx *trx)
{
  return engine_->insert_record_with_trx(record, trx);
}
RC Table::delete_record_with_trx(const Record &record, Trx *trx)
{
  return engine_->delete_record_with_trx(record, trx);
}

RC Table::update_record_with_trx(const Record &old_record, const Record &new_record, Trx* trx)
{
  return engine_->update_record_with_trx(old_record, new_record, trx);
}

RC Table::get_record(const RID &rid, Record &record)
{
  if (is_view()) {
    LOG_WARN("Cannot get record from view: %s", name());
    return RC::UNIMPLEMENTED;
  }
  return engine_->get_record(rid, record);
}

const char *Table::name() const { return table_meta_.name(); }

const TableMeta &Table::table_meta() const { return table_meta_; }

RC Table::make_record(int value_num, const Value *values, Record &record)
{
  RC rc = RC::SUCCESS;
  // 检查字段类型是否一致
  if (value_num + table_meta_.sys_field_num() != table_meta_.field_num()) {
    LOG_WARN("Input values don't match the table's schema, table name:%s", table_meta_.name());
    return RC::SCHEMA_FIELD_MISSING;
  }

  const int normal_field_start_index = table_meta_.sys_field_num();
  // 复制所有字段的值
  int   record_size = table_meta_.record_size();
  char *record_data = (char *)malloc(record_size);
  memset(record_data, 0, record_size);

  for (int i = 0; i < value_num && OB_SUCC(rc); i++) {
    const FieldMeta *field = table_meta_.field(i + normal_field_start_index);
    const Value &    value = values[i];

    // Handle NULL values
    if (value.attr_type() == AttrType::UNDEFINED) {
      if (!field->nullable()) {
        LOG_WARN("Cannot insert NULL into NOT NULL field. table name:%s,field name:%s",
            table_meta_.name(), field->name());
        rc = RC::INVALID_ARGUMENT;
        break;
      }
      // For NULL values, just set the record data to 0 (which represents NULL)
      rc = set_value_to_record(record_data, value, field);
    } else if (field->type() != value.attr_type()) {
      Value real_value;
      rc = Value::cast_to(value, field->type(), real_value);
      if (OB_FAIL(rc)) {
        LOG_WARN("failed to cast value. table name:%s,field name:%s,value:%s ",
            table_meta_.name(), field->name(), value.to_string().c_str());
        break;
      }
      rc = set_value_to_record(record_data, real_value, field);
    } else {
      rc = set_value_to_record(record_data, value, field);
    }
  }
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to make record. table name:%s", table_meta_.name());
    free(record_data);
    return rc;
  }

  record.set_data_owner(record_data, record_size);
  return RC::SUCCESS;
}

RC Table::set_value_to_record(char *record_data, const Value &value, const FieldMeta *field)
{
  // Handle NULL values
  if (value.attr_type() == AttrType::UNDEFINED || value.is_null()) {
    if (field->nullable()) {
      // For nullable fields, set the field data to zeros and set NULL flag in last byte
      memset(record_data + field->offset(), 0, field->len());
      record_data[field->offset() + field->len() - 1] = '1';  // Set NULL flag
      return RC::SUCCESS;
    }
    // For non-nullable fields, reject NULL assignment
    LOG_WARN("Cannot set NULL to NOT NULL field %s", field->name());
    return RC::INVALID_ARGUMENT;
  }

  size_t       copy_len = field->len();
  size_t       data_len = value.length();

  if (field->type() == AttrType::CHARS) {
    if (field->nullable()) {
      copy_len = field->len() - 1;
    }
    if (copy_len > data_len) {
      copy_len = data_len;
    }
    // 清理 CHAR 字段剩余区域，防止更新为更短字符串时残留旧字符
    size_t chars_area_len = field->len() - (field->nullable() ? 1 : 0);
    memset(record_data + field->offset(), 0, chars_area_len);
  } else if (field->type() == AttrType::TEXTS) {
    // For TEXT: write to LOB and store offset+length
    if (value.is_null()) {
      // handled by NULL path above
    } else {
      printf("DEBUG: [TEXT] write begin field=%s raw_len=%zu ptr_null=%d\n",
             field->name(), data_len, value.data() == nullptr);
      // Fallback: some UPDATE constant folding可能未正确设置 Value.length()
      if (value.attr_type() == AttrType::CHARS && data_len == 0 && value.data() != nullptr) {
        data_len = strlen(value.data());
        printf("DEBUG: [TEXT] write corrected_len via strlen=%zu\n", data_len);
      }
      if (data_len > 65535) {
        LOG_WARN("TEXT too long: %zu > 65535", data_len);
        return RC::INVALID_ARGUMENT;
      }
      uint64_t lob_off = 0;
      RC lrc = append_text_lob(value.data(), static_cast<uint32_t>(data_len), lob_off);
      if (lrc != RC::SUCCESS) {
        return lrc;
      }
      // in-row layout: [offset(8)][length(4)][optional null flag]
      bool with_null_flag = field->nullable();
      // zero area
      memset(record_data + field->offset(), 0, field->len());
      memcpy(record_data + field->offset(), &lob_off, sizeof(uint64_t));
      uint32_t ulen = static_cast<uint32_t>(data_len);
      memcpy(record_data + field->offset() + sizeof(uint64_t), &ulen, sizeof(uint32_t));
      if (with_null_flag) {
        record_data[field->offset() + field->len() - 1] = '0';
      }
      printf("DEBUG: [TEXT] write done field=%s off=%llu len=%u nullflag_set=%d\n",
             field->name(), (unsigned long long)lob_off, ulen, with_null_flag ? 1 : 0);
      return RC::SUCCESS;
    }
  } else if (field->type() == AttrType::VECTORS) {
    // For VECTORS type, handle nullable fields specially
    if (field->nullable()) {
      copy_len = field->len() - 1;  // Reserve last byte for NULL flag
    }
    LOG_INFO("DEBUG: set_value_to_record VECTOR field=%s value_len=%zu field_len=%d copy_limit=%zu",
             field->name(), data_len, field->len(), copy_len);
    // 对于向量类型，需要检查数据长度是否超过字段定义的长度
    if (data_len > copy_len) {
      LOG_WARN("Vector data length (%zu) exceeds field length (%zu) for field %s",
               data_len, copy_len, field->name());
      return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    }
    // 当模式为显式 VECTOR(n) 时，字段长度(扣除可空标志位)应为 sizeof(int)+n*sizeof(float)
    // 强制校验实际维度与定义维度完全一致，防止插入短向量到高维列
    int expected_data_len = static_cast<int>(copy_len);
    if (expected_data_len > static_cast<int>(sizeof(int))) {  // 仅当定义了具体维度时进行严格校验
      int expected_dim = (expected_data_len - static_cast<int>(sizeof(int))) / static_cast<int>(sizeof(float));
      if (value.data() == nullptr || data_len < sizeof(int)) {
        LOG_WARN("Invalid vector payload for field %s", field->name());
        return RC::INVALID_ARGUMENT;
      }
      int actual_dim = *(const int *)(value.data());
      int actual_data_len = static_cast<int>(sizeof(int) + actual_dim * sizeof(float));
      if (actual_data_len != expected_data_len) {
        LOG_WARN("Vector dimension mismatch for field %s: expected %d, got %d", field->name(), expected_dim, actual_dim);
        return RC::SCHEMA_FIELD_TYPE_MISMATCH;
      }
      LOG_INFO("DEBUG: VECTOR dim ok field=%s expected_dim=%d actual_dim=%d out_len=%d",
               field->name(), expected_dim, actual_dim, (int)data_len);
    }
    // 对于向量类型，使用实际数据长度
    copy_len = data_len;
  } else {
    // For fixed-length types (INT, FLOAT, DATE, etc.), always use full data length
    // The NULL flag for nullable fields will be handled separately
    copy_len = data_len;
  }

  memcpy(record_data + field->offset(), value.data(), copy_len);

  // For nullable fields, set NULL flag to '0' (not NULL)
  if (field->nullable()) {
    if (field->type() == AttrType::CHARS || field->type() == AttrType::TEXTS || field->type() == AttrType::VECTORS) {
      // For variable-length types, NULL flag is at the end
      record_data[field->offset() + field->len() - 1] = '0';
    } else {
      // For fixed-length types, we need to place NULL flag after the data
      // but within the field's allocated space
      if (copy_len < field->len()) {
        record_data[field->offset() + field->len() - 1] = '0';
      }
    }
  }

  return RC::SUCCESS;
}

RC Table::get_record_scanner(RecordScanner *&scanner, Trx *trx, ReadWriteMode mode)
{
  if (is_view()) {
    // 视图应该通过其SELECT语句查询，而不是直接扫描记录
    // 这里返回一个空scanner作为临时方案，防止崩溃
    // TODO: 正确的实现应该在SelectStmt阶段展开视图
    LOG_WARN("get_record_scanner called on view %s, which should not happen. Returning empty scanner.", name());
    scanner = nullptr;  // 返回nullptr表示不支持
    return RC::UNIMPLEMENTED;
  }
  return engine_->get_record_scanner(scanner, trx, mode);
}

RC Table::get_chunk_scanner(ChunkFileScanner &scanner, Trx *trx, ReadWriteMode mode)
{
  return engine_->get_chunk_scanner(scanner, trx, mode);
}

RC Table::create_index(Trx *trx, const FieldMeta *field_meta, const char *index_name)
{
  return engine_->create_index(trx, field_meta, index_name);
}

RC Table::create_index(Trx *trx, const vector<const FieldMeta *> &field_metas, const char *index_name)
{
  return engine_->create_index(trx, field_metas, index_name);
}

RC Table::create_index(Trx *trx, const FieldMeta *field_meta, const char *index_name, bool is_unique)
{
  return engine_->create_index(trx, field_meta, index_name, is_unique);
}

RC Table::create_index(Trx *trx, const vector<const FieldMeta *> &field_metas, const char *index_name, bool is_unique)
{
  return engine_->create_index(trx, field_metas, index_name, is_unique);
}

RC Table::drop_index(Trx *trx, const char *index_name)
{
  return engine_->drop_index(trx, index_name);
}

RC Table::delete_record(const Record &record)
{
  return engine_->delete_record(record);
}

Index *Table::find_index(const char *index_name) const
{
  return engine_->find_index(index_name);
}
Index *Table::find_index_by_field(const char *field_name) const
{
  return engine_->find_index_by_field(field_name);
}

RC Table::sync()
{
  // 视图不需要同步数据文件
  if (is_view()) {
    return RC::SUCCESS;
  }
  return engine_->sync();
}

RC Table::alter_change_column_rename(const char *old_name, const char *new_name)
{
  if (is_view()) return RC::UNIMPLEMENTED;
  if (common::is_blank(old_name) || common::is_blank(new_name)) return RC::INVALID_ARGUMENT;

  // build new meta and atomically persist
  TableMeta new_meta(table_meta_);
  RC rc = new_meta.rename_column(old_name, new_name);
  if (rc != RC::SUCCESS) return rc;

  string base = db_->path();
  string tmp_file = table_meta_file(base.c_str(), table_meta_.name()) + ".tmp";
  {
    fstream fs;
    fs.open(tmp_file, ios_base::out | ios_base::binary | ios_base::trunc);
    if (!fs.is_open()) return RC::IOERR_OPEN;
    new_meta.serialize(fs);
    fs.close();
  }
  string meta_file = table_meta_file(base.c_str(), table_meta_.name());
  if (std::rename(tmp_file.c_str(), meta_file.c_str()) != 0) {
    return RC::IOERR_WRITE;
  }

  table_meta_.swap(new_meta);
  return RC::SUCCESS;
}

RC Table::alter_add_column_int(const char *new_name)
{
  if (is_view()) return RC::UNIMPLEMENTED;
  if (common::is_blank(new_name)) return RC::INVALID_ARGUMENT;

  // 仅支持在现有全部用户字段均为 INT 的前提下重写
  for (int i = table_meta_.sys_field_num(); i < table_meta_.field_num(); ++i) {
    const FieldMeta *f = table_meta_.field(i);
    if (f->type() != AttrType::INTS) {
      return RC::UNSUPPORTED;
    }
  }

  // 1) 提取旧数据
  vector<vector<int>> rows;
  {
    RecordScanner *scanner = nullptr;
    RC rc = engine_->get_record_scanner(scanner, nullptr, ReadWriteMode::READ_ONLY);
    if (rc != RC::SUCCESS) return rc;
    Record record;
    while (OB_SUCC(scanner->next(record))) {
      vector<int> row;
      for (int i = table_meta_.sys_field_num(); i < table_meta_.field_num(); ++i) {
        const FieldMeta *f = table_meta_.field(i);
        int v = 0;
        memcpy(&v, record.data() + f->offset(), sizeof(int));
        row.push_back(v);
      }
      rows.push_back(std::move(row));
    }
    delete scanner;
  }

  // 2) 生成新元数据（与 MySQL 一致：新增列默认允许 NULL，且未指定 DEFAULT 时，历史行值为 NULL）
  vector<AttrInfoSqlNode> attrs;
  for (int i = table_meta_.sys_field_num(); i < table_meta_.field_num(); ++i) {
    const FieldMeta *f = table_meta_.field(i);
    AttrInfoSqlNode a; a.name = f->name(); a.type = f->type(); a.length = f->len(); a.nullable = f->nullable();
    attrs.push_back(a);
  }
  {
    // INT 可空：长度按现有规范需额外 1 字节 NULL 标志
    AttrInfoSqlNode a; a.name = new_name; a.type = AttrType::INTS; a.length = 5; a.nullable = true;
    attrs.push_back(a);
  }

  vector<FieldMeta> trx_copy(table_meta_.trx_fields().begin(), table_meta_.trx_fields().end());
  TableMeta new_meta;
  RC rc = new_meta.init(table_meta_.table_id(), table_meta_.name(), &trx_copy, attrs, table_meta_.primary_keys(), table_meta_.storage_format(), table_meta_.storage_engine());
  if (rc != RC::SUCCESS) return rc;

  // 3) 原子写入新元数据
  string base = db_->path();
  string tmp_meta = table_meta_file(base.c_str(), table_meta_.name()) + ".tmp";
  {
    fstream fs;
    fs.open(tmp_meta, ios_base::out | ios_base::binary | ios_base::trunc);
    if (!fs.is_open()) return RC::IOERR_OPEN;
    new_meta.serialize(fs);
    fs.close();
  }
  string meta_file_path = table_meta_file(base.c_str(), table_meta_.name());
  if (std::rename(tmp_meta.c_str(), meta_file_path.c_str()) != 0) return RC::IOERR_WRITE;

  // 4) 重建数据文件
  // 关闭现引擎，删除旧数据文件，创建新文件
  engine_.reset();
  string data_file = table_data_file(base.c_str(), table_meta_.name());
  std::error_code fec;
  std::filesystem::remove(data_file, fec);
  BufferPoolManager &bpm = db_->buffer_pool_manager();
  rc = bpm.create_file(data_file.c_str());
  if (rc != RC::SUCCESS) return rc;

  // 切换内存中的元数据
  table_meta_.swap(new_meta);

  // 重新打开引擎
  if (table_meta_.storage_engine() == StorageEngine::HEAP) {
    engine_ = make_unique<HeapTableEngine>(&table_meta_, db_, this);
  } else if (table_meta_.storage_engine() == StorageEngine::LSM) {
    engine_ = make_unique<LsmTableEngine>(&table_meta_, db_, this);
  } else {
    return RC::UNSUPPORTED;
  }
  rc = engine_->open();
  if (rc != RC::SUCCESS) return rc;

  // 插入数据（新增列默认 NULL）
  for (const auto &row : rows) {
    vector<Value> vals;
    vals.reserve(row.size() + 1);
    for (int v : row) { vals.emplace_back(Value(v)); }
    Value nullv; nullv.set_type(AttrType::UNDEFINED); nullv.set_null(true);
    vals.emplace_back(nullv);
    Record rec;
    rc = make_record(static_cast<int>(vals.size()), vals.data(), rec);
    if (rc != RC::SUCCESS) return rc;
    rc = insert_record(rec);
    if (rc != RC::SUCCESS) return rc;
  }

  return RC::SUCCESS;
}

RC Table::alter_drop_column(const char *name)
{
  if (is_view()) return RC::UNIMPLEMENTED;
  if (common::is_blank(name)) return RC::INVALID_ARGUMENT;

  // 仅支持所有用户字段为 INT
  for (int i = table_meta_.sys_field_num(); i < table_meta_.field_num(); ++i) {
    const FieldMeta *f = table_meta_.field(i);
    if (f->type() != AttrType::INTS) {
      return RC::UNSUPPORTED;
    }
  }

  // 找到要删除的列序
  int drop_idx = -1;
  for (int i = table_meta_.sys_field_num(); i < table_meta_.field_num(); ++i) {
    if (0 == strcmp(table_meta_.field(i)->name(), name)) { drop_idx = i; break; }
  }
  if (drop_idx < 0) return RC::SCHEMA_FIELD_NOT_EXIST;

  // 如有索引，先删除
  const IndexMeta *idx = table_meta_.find_index_by_field(name);
  if (idx != nullptr) {
    (void)drop_index(nullptr, idx->name());
  }

  // 1) 提取旧数据
  vector<vector<int>> rows;
  {
    RecordScanner *scanner = nullptr;
    RC rc = engine_->get_record_scanner(scanner, nullptr, ReadWriteMode::READ_ONLY);
    if (rc != RC::SUCCESS) return rc;
    Record record;
    while (OB_SUCC(scanner->next(record))) {
      vector<int> row;
      for (int i = table_meta_.sys_field_num(); i < table_meta_.field_num(); ++i) {
        const FieldMeta *f = table_meta_.field(i);
        int v = 0;
        memcpy(&v, record.data() + f->offset(), sizeof(int));
        row.push_back(v);
      }
      rows.push_back(std::move(row));
    }
    delete scanner;
  }

  // 2) 生成新元数据（移除该列）
  vector<AttrInfoSqlNode> attrs;
  for (int i = table_meta_.sys_field_num(); i < table_meta_.field_num(); ++i) {
    const FieldMeta *f = table_meta_.field(i);
    if (0 == strcmp(f->name(), name)) continue;
    AttrInfoSqlNode a; a.name = f->name(); a.type = f->type(); a.length = f->len(); a.nullable = f->nullable();
    attrs.push_back(a);
  }
  vector<FieldMeta> trx_copy(table_meta_.trx_fields().begin(), table_meta_.trx_fields().end());
  TableMeta new_meta;
  RC rc = new_meta.init(table_meta_.table_id(), table_meta_.name(), &trx_copy, attrs, table_meta_.primary_keys(), table_meta_.storage_format(), table_meta_.storage_engine());
  if (rc != RC::SUCCESS) return rc;

  // 3) 原子写入新元数据
  string base = db_->path();
  string tmp_meta = table_meta_file(base.c_str(), table_meta_.name()) + ".tmp";
  {
    fstream fs;
    fs.open(tmp_meta, ios_base::out | ios_base::binary | ios_base::trunc);
    if (!fs.is_open()) return RC::IOERR_OPEN;
    new_meta.serialize(fs);
    fs.close();
  }
  string meta_file_path = table_meta_file(base.c_str(), table_meta_.name());
  if (std::rename(tmp_meta.c_str(), meta_file_path.c_str()) != 0) return RC::IOERR_WRITE;

  // 4) 重建数据文件
  engine_.reset();
  string data_file = table_data_file(base.c_str(), table_meta_.name());
  std::error_code fec;
  std::filesystem::remove(data_file, fec);
  BufferPoolManager &bpm = db_->buffer_pool_manager();
  rc = bpm.create_file(data_file.c_str());
  if (rc != RC::SUCCESS) return rc;

  table_meta_.swap(new_meta);

  if (table_meta_.storage_engine() == StorageEngine::HEAP) {
    engine_ = make_unique<HeapTableEngine>(&table_meta_, db_, this);
  } else if (table_meta_.storage_engine() == StorageEngine::LSM) {
    engine_ = make_unique<LsmTableEngine>(&table_meta_, db_, this);
  } else {
    return RC::UNSUPPORTED;
  }
  rc = engine_->open();
  if (rc != RC::SUCCESS) return rc;

  // 插入数据（跳过该列）
  int drop_user_idx = drop_idx - table_meta_.sys_field_num();
  for (const auto &row : rows) {
    vector<Value> vals;
    vals.reserve(row.size() - 1);
    for (size_t j = 0; j < row.size(); ++j) {
      if ((int)j == drop_user_idx) continue;
      vals.emplace_back(Value((int)row[j]));
    }
    Record rec;
    rc = make_record(static_cast<int>(vals.size()), vals.data(), rec);
    if (rc != RC::SUCCESS) return rc;
    rc = insert_record(rec);
    if (rc != RC::SUCCESS) return rc;
  }

  return RC::SUCCESS;
}
