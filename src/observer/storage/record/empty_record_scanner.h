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

#include "storage/record/record_scanner.h"

/**
 * @brief 空记录扫描器，用于视图等不需要扫描记录的场景
 * @ingroup RecordManager
 */
class EmptyRecordScanner : public RecordScanner
{
public:
  EmptyRecordScanner() = default;
  ~EmptyRecordScanner() override = default;

  RC open_scan() override { return RC::SUCCESS; }
  RC close_scan() override { return RC::SUCCESS; }
  RC next(Record &record) override { return RC::RECORD_EOF; }
};

