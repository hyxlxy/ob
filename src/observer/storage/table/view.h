/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once

#include "storage/table/table.h"
#include "sql/operator/physical_operator.h"

class SelectStmt;

class View
{
public:
  View()           = default;
  ~View() = default;

  /**
   * 创建一个视图
   * @param path 元数据保存的文件(完整路径)
   * @param name 视图名
   * @param base_dir 视图数据存放的路径
   * @param attr_names 字段
   * @param select_sql 查询 sql
   * @param select_stmt 查询 stmt
   * @param storage_format 存储格式，与基表一致
   */
  RC create(Db *db, int32_t table_id, const char *path, const char *name, const char *base_dir,
      std::vector<std::string> attr_names, std::string select_sql, SelectStmt *select_stmt,
      StorageFormat storage_format);

  RC drop();

  /**
   * 打开一个视图
   * @param meta_file 保存视图元数据的文件完整路径
   * @param base_dir 视图所在的文件夹
   */
  RC open(Db *db, const char *meta_file, const char *base_dir);

  RC init_data();

  RC init_member();

  /**
   * 获取视图的查询SQL
   */
  const std::string &select_sql() const { return select_sql_; }

  /**
   * 获取视图的查询语句
   */
  SelectStmt *select_stmt() const { return select_stmt_; }

  /**
   * 设置视图的查询语句
   */
  void set_select_stmt(SelectStmt *stmt) { select_stmt_ = stmt; }

  /**
   * 获取视图名称
   */
  const char *name() const;

private:
  std::string  name_;         ///< 视图名称
  std::string  select_sql_;   ///< 视图的查询SQL
  SelectStmt  *select_stmt_ = nullptr; ///< 视图的查询语句
};
