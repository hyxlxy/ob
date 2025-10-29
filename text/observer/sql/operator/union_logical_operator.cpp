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
// Created by AI Assistant on 2024/12/19.
//

#include "sql/operator/union_logical_operator.h"
#include "common/log/log.h"

using namespace std;

UnionLogicalOperator::UnionLogicalOperator(const vector<bool> &distinct_flags,
                                          const vector<AttrType> &result_types,
                                          const vector<string> &result_names)
    : distinct_flags_(distinct_flags), result_types_(result_types), result_names_(result_names)
{
}

// find_log_prop removed: not used in current planning pipeline
