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
// Created by Wangyunlai on 2021/5/12.
//

#pragma once

#include "common/lang/serializable.h"
#include "common/sys/rc.h"
#include "common/types.h"
#include "common/lang/span.h"
#include "storage/field/field_meta.h"
#include "storage/index/index_meta.h"

/**
 * @brief 表类型枚举
 */
enum class TableType
{
  TABLE,  ///< 普通表
  VIEW    ///< 视图
};

/**
 * @brief 表元数据
 *
 */
class TableMeta : public common::Serializable
{
public:
  TableMeta()          = default;
  virtual ~TableMeta() = default;

  TableMeta(const TableMeta &other);

  void swap(TableMeta &other) noexcept;

  RC init(int32_t table_id, const char *name, const vector<FieldMeta> *trx_fields,
      span<const AttrInfoSqlNode> attributes, const vector<string> &primary_keys, StorageFormat storage_format,
      StorageEngine storage_engine);

  /**
   * @brief 初始化视图元数据
   */
  RC init_view(int32_t table_id, const char *name, const vector<string> &attr_names,
               const string &select_sql);

  RC add_index(const IndexMeta &index);
  RC drop_index(const char *index_name);

public:
  int32_t             table_id() const { return table_id_; }
  const char         *name() const;
  const FieldMeta    *trx_field() const;
  const FieldMeta    *field(int index) const;
  const FieldMeta    *field(const char *name) const;
  const FieldMeta    *find_field_by_offset(int offset) const;
  auto                field_metas() const -> const vector<FieldMeta>                *{ return &fields_; }
  auto                trx_fields() const -> span<const FieldMeta>;
  const StorageFormat storage_format() const { return storage_format_; }
  const StorageEngine storage_engine() const { return storage_engine_; }

  int field_num() const;  // sys field included
  int sys_field_num() const;

  const IndexMeta *index(const char *name) const;
  const IndexMeta *find_index_by_field(const char *field) const;
  const IndexMeta *index(int i) const;
  int              index_num() const;

  const vector<string> &primary_keys() const { return primary_keys_; }

  int record_size() const;

  // 视图相关方法
  TableType table_type() const { return table_type_; }
  bool is_view() const { return table_type_ == TableType::VIEW; }
  const string &select_sql() const { return select_sql_; }

public:
  int  serialize(ostream &os) const override;
  int  deserialize(istream &is) override;
  int  get_serial_size() const override;
  void to_string(string &output) const override;
  void desc(ostream &os) const;

protected:
  int32_t           table_id_ = -1;
  string            name_;
  vector<FieldMeta> trx_fields_;
  vector<FieldMeta> fields_;  // 包含sys_fields
  vector<IndexMeta> indexes_;
  vector<string>    primary_keys_;
  StorageFormat     storage_format_;
  StorageEngine     storage_engine_;
  TableType         table_type_ = TableType::TABLE;  ///< 表类型
  string            select_sql_;                      ///< 视图的SELECT语句（仅视图使用）

  int record_size_ = 0;
};
