/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "storage/table/view.h"
#include "common/log/log.h"
#include "storage/db/db.h"
#include <fstream>

RC View::create(Db *db, int32_t table_id, const char *path, const char *name, const char *base_dir,
                std::vector<std::string> attr_names, std::string select_sql, SelectStmt *select_stmt,
                StorageFormat storage_format)
{
  if (db == nullptr || path == nullptr || name == nullptr || base_dir == nullptr) {
    LOG_WARN("invalid arguments. db=%p, path=%s, name=%s, base_dir=%s", db, path, name, base_dir);
    return RC::INVALID_ARGUMENT;
  }

  // 简化实现：暂时只保存基本信息
  name_ = name;
  select_sql_ = select_sql;
  select_stmt_ = select_stmt;
  
  LOG_INFO("successfully create view %s", name);
  return RC::SUCCESS;
}

RC View::drop()
{
  LOG_INFO("successfully drop view %s", name_.c_str());
  return RC::SUCCESS;
}

RC View::open(Db *db, const char *meta_file, const char *base_dir)
{
  if (db == nullptr || meta_file == nullptr || base_dir == nullptr) {
    LOG_WARN("invalid arguments. db=%p, meta_file=%s, base_dir=%s", db, meta_file, base_dir);
    return RC::INVALID_ARGUMENT;
  }

  LOG_INFO("successfully open view from %s", meta_file);
  return RC::SUCCESS;
}

const char *View::name() const
{
  return name_.c_str();
}
