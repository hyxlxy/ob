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
// Created by wangyunlai.wyl on 2021/5/19.
//

#include "storage/index/bplus_tree_index.h"
#include "common/log/log.h"
#include "storage/table/table.h"
#include "storage/db/db.h"

BplusTreeIndex::~BplusTreeIndex() noexcept { close(); }

RC BplusTreeIndex::create(Table *table, const char *file_name, const IndexMeta &index_meta, const FieldMeta &field_meta)
{
  if (inited_) {
    LOG_WARN("Failed to create index due to the index has been created before. file_name:%s, index:%s, field:%s",
        file_name, index_meta.name(), index_meta.field());
    return RC::RECORD_OPENNED;
  }

  Index::init(index_meta, field_meta);

  BufferPoolManager &bpm = table->db()->buffer_pool_manager();
  RC rc = index_handler_.create(table->db()->log_handler(), bpm, file_name, field_meta.type(), field_meta.len());
  if (RC::SUCCESS != rc) {
    LOG_WARN("Failed to create index_handler, file_name:%s, index:%s, field:%s, rc:%s",
        file_name, index_meta.name(), index_meta.field(), strrc(rc));
    return rc;
  }

  inited_ = true;
  table_  = table;
  LOG_INFO("Successfully create index, file_name:%s, index:%s, field:%s",
    file_name, index_meta.name(), index_meta.field());
  return RC::SUCCESS;
}

RC BplusTreeIndex::create(Table *table, const char *file_name, const IndexMeta &index_meta, const vector<FieldMeta> &field_metas)
{
  if (inited_) {
    return RC::RECORD_OPENNED;
  }

  if (field_metas.empty()) {
    return RC::INVALID_ARGUMENT;
  }

  // 保存所有字段
  field_metas_ = field_metas;
  
  // 计算组合键长度
  int total_len = 0;
  for (const FieldMeta &fm : field_metas) {
    total_len += fm.len();
  }

  // 使用第一个字段初始化
  Index::init(index_meta, field_metas[0]);
  
  BufferPoolManager &bpm = table->db()->buffer_pool_manager();
  // 使用多列比较器创建索引
  RC rc = index_handler_.create(table->db()->log_handler(), bpm, file_name, field_metas, total_len);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to create multi-field index_handler, file_name:%s, index:%s, rc:%s",
        file_name, index_meta.name(), strrc(rc));
    return rc;
  }

  inited_ = true;
  table_  = table;
  return RC::SUCCESS;
}

RC BplusTreeIndex::open(Table *table, const char *file_name, const IndexMeta &index_meta, const FieldMeta &field_meta)
{
  if (inited_) {
    LOG_WARN("Failed to open index due to the index has been initedd before. file_name:%s, index:%s, field:%s",
        file_name, index_meta.name(), index_meta.field());
    return RC::RECORD_OPENNED;
  }

  Index::init(index_meta, field_meta);

  BufferPoolManager &bpm = table->db()->buffer_pool_manager();
  RC rc = index_handler_.open(table->db()->log_handler(), bpm, file_name);
  if (RC::SUCCESS != rc) {
    LOG_WARN("Failed to open index_handler, file_name:%s, index:%s, field:%s, rc:%s",
        file_name, index_meta.name(), index_meta.field(), strrc(rc));
    return rc;
  }

  inited_ = true;
  table_  = table;
  LOG_INFO("Successfully open index, file_name:%s, index:%s, field:%s",
    file_name, index_meta.name(), index_meta.field());
  return RC::SUCCESS;
}

RC BplusTreeIndex::close()
{
  if (inited_) {
    LOG_INFO("Begin to close index, index:%s, field:%s", index_meta_.name(), index_meta_.field());
    index_handler_.close();
    inited_ = false;
  }
  LOG_INFO("Successfully close index.");
  return RC::SUCCESS;
}

RC BplusTreeIndex::insert_entry(const char *record, const RID *rid)
{
  // Build multi-field key for multi-column indexes
  vector<char> combined_key;
  const char *key_data;
  int key_len;
  
  if (!field_metas_.empty()) {
    // Multi-field index: build combined key
    for (const FieldMeta &field_meta : field_metas_) {
      const char *field_data = record + field_meta.offset();
      combined_key.insert(combined_key.end(), field_data, field_data + field_meta.len());
    }
    key_data = combined_key.data();
    key_len = combined_key.size();
    
    // Debug output for multi-field key construction
    LOG_INFO("=== MULTI-FIELD INDEX DEBUG ===");
    LOG_INFO("Index: %s, RID: %s", index_meta_.name(), rid->to_string().c_str());
    LOG_INFO("Field count: %zu", field_metas_.size());
    for (size_t i = 0; i < field_metas_.size(); i++) {
      const FieldMeta &fm = field_metas_[i];
      const char *field_data = record + fm.offset();
      LOG_INFO("Field[%zu]: name=%s, type=%d, len=%d, offset=%d", 
               i, fm.name(), (int)fm.type(), fm.len(), fm.offset());
      
      // Print field data as hex
      string hex_data;
      for (int j = 0; j < fm.len(); j++) {
        char hex[4];
        snprintf(hex, sizeof(hex), "%02x", (unsigned char)field_data[j]);
        hex_data += hex;
        if (j < fm.len() - 1) hex_data += " ";
      }
      LOG_INFO("Field[%zu] data (hex): %s", i, hex_data.c_str());
    }
    LOG_INFO("Combined key length: %zu", combined_key.size());
    string combined_hex;
    for (size_t i = 0; i < combined_key.size(); i++) {
      char hex[4];
      snprintf(hex, sizeof(hex), "%02x", (unsigned char)combined_key[i]);
      combined_hex += hex;
      if (i < combined_key.size() - 1) combined_hex += " ";
    }
    LOG_INFO("Combined key (hex): %s", combined_hex.c_str());
    LOG_INFO("=== END MULTI-FIELD INDEX DEBUG ===");
  } else {
    // Single-field index: use original logic
    key_data = record + field_meta_.offset();
    key_len = field_meta_.len();
  }

  // Check for uniqueness if this is a unique index
  if (index_meta_.is_unique()) {
    // MySQL semantics: if any indexed column is NULL, it does NOT participate in
    // uniqueness checks (i.e., multiple NULLs are allowed). We detect NULL flags
    // directly from the record bytes. For nullable fields, the last byte is the
    // NULL flag: '1' means NULL, '0' means NOT NULL.

    auto field_is_null = [&](const FieldMeta &fm) -> bool {
      if (!fm.nullable()) {
        return false;
      }
      const int null_flag_pos = fm.offset() + fm.len() - 1;
      return *(record + null_flag_pos) == '1';
    };

    bool any_null_in_index_key = false;
    if (!field_metas_.empty()) {
      // Multi-column index: if ANY component is NULL, skip uniqueness check
      for (const FieldMeta &fm : field_metas_) {
        if (field_is_null(fm)) {
          any_null_in_index_key = true;
          break;
        }
      }
    } else {
      // Single-column index
      if (field_is_null(field_meta_)) {
        any_null_in_index_key = true;
      }
    }

    if (!any_null_in_index_key) {
    // If the index is empty, no need to check for duplicates
    if (!index_handler_.is_empty()) {
      // Check if the key already exists
      BplusTreeScanner scanner(index_handler_);
      RC rc = scanner.open(key_data, key_len, true, key_data, key_len, true);
      if (rc == RC::SUCCESS) {
        RID existing_rid;
        rc = scanner.next_entry(existing_rid);
        scanner.close();

        if (rc == RC::SUCCESS) {
          // Key already exists, this violates uniqueness constraint
          LOG_WARN("Unique constraint violation: key already exists in index %s", index_meta_.name());
          return RC::RECORD_DUPLICATE_KEY;
        }
      } else {
        // If scanner.open failed for non-empty index, this is an error
        LOG_WARN("Scanner open failed for uniqueness check in non-empty index %s, rc=%s", index_meta_.name(), strrc(rc));
        return rc;
      }
    }
    } else {
      LOG_TRACE("Skip unique check for NULL key on index %s", index_meta_.name());
    }
  }

  RC insert_rc = index_handler_.insert_entry(key_data, rid);
  if (insert_rc != RC::SUCCESS) {
    LOG_WARN("Failed to insert entry into index %s, rc=%s", index_meta_.name(), strrc(insert_rc));
  }
  return insert_rc;
}

RC BplusTreeIndex::delete_entry(const char *record, const RID *rid)
{
  // Build multi-field key for multi-column indexes
  vector<char> combined_key;
  const char *key_data;
  
  if (!field_metas_.empty()) {
    // Multi-field index: build combined key
    for (const FieldMeta &field_meta : field_metas_) {
      const char *field_data = record + field_meta.offset();
      combined_key.insert(combined_key.end(), field_data, field_data + field_meta.len());
    }
    key_data = combined_key.data();
    
    // Debug output for multi-field key deletion
    LOG_INFO("=== MULTI-FIELD INDEX DELETE DEBUG ===");
    LOG_INFO("Index: %s, RID: %s", index_meta_.name(), rid->to_string().c_str());
    LOG_INFO("Field count: %zu", field_metas_.size());
    for (size_t i = 0; i < field_metas_.size(); i++) {
      const FieldMeta &fm = field_metas_[i];
      const char *field_data = record + fm.offset();
      LOG_INFO("Field[%zu]: name=%s, type=%d, len=%d, offset=%d", 
               i, fm.name(), (int)fm.type(), fm.len(), fm.offset());
      
      // Print field data as hex
      string hex_data;
      for (int j = 0; j < fm.len(); j++) {
        char hex[4];
        snprintf(hex, sizeof(hex), "%02x", (unsigned char)field_data[j]);
        hex_data += hex;
        if (j < fm.len() - 1) hex_data += " ";
      }
      LOG_INFO("Field[%zu] data (hex): %s", i, hex_data.c_str());
    }
    LOG_INFO("Combined key length: %zu", combined_key.size());
    string combined_hex;
    for (size_t i = 0; i < combined_key.size(); i++) {
      char hex[4];
      snprintf(hex, sizeof(hex), "%02x", (unsigned char)combined_key[i]);
      combined_hex += hex;
      if (i < combined_key.size() - 1) combined_hex += " ";
    }
    LOG_INFO("Combined key (hex): %s", combined_hex.c_str());
    LOG_INFO("=== END MULTI-FIELD INDEX DELETE DEBUG ===");
  } else {
    // Single-field index: use original logic
    key_data = record + field_meta_.offset();
  }

  return index_handler_.delete_entry(key_data, rid);
}

IndexScanner *BplusTreeIndex::create_scanner(
    const char *left_key, int left_len, bool left_inclusive, const char *right_key, int right_len, bool right_inclusive)
{
  BplusTreeIndexScanner *index_scanner = new BplusTreeIndexScanner(index_handler_);
  RC rc = index_scanner->open(left_key, left_len, left_inclusive, right_key, right_len, right_inclusive);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to open index scanner. rc=%d:%s", rc, strrc(rc));
    delete index_scanner;
    return nullptr;
  }
  return index_scanner;
}

RC BplusTreeIndex::sync() { return index_handler_.sync(); }

////////////////////////////////////////////////////////////////////////////////
BplusTreeIndexScanner::BplusTreeIndexScanner(BplusTreeHandler &tree_handler) : tree_scanner_(tree_handler) {}

BplusTreeIndexScanner::~BplusTreeIndexScanner() noexcept { tree_scanner_.close(); }

RC BplusTreeIndexScanner::open(
    const char *left_key, int left_len, bool left_inclusive, const char *right_key, int right_len, bool right_inclusive)
{
  return tree_scanner_.open(left_key, left_len, left_inclusive, right_key, right_len, right_inclusive);
}

RC BplusTreeIndexScanner::next_entry(RID *rid) { return tree_scanner_.next_entry(*rid); }

RC BplusTreeIndexScanner::destroy()
{
  delete this;
  return RC::SUCCESS;
}
