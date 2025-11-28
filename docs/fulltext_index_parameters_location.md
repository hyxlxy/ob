# 全文索引查询效率参数设置位置

本文档说明影响全文索引查询效率的参数在哪些文件中定义、设置和使用。

## 一、参数定义文件

### 主要定义文件
**`src/share/parameter/ob_parameter_seed.ipp`**

这是所有系统参数的统一定义文件，使用宏定义方式声明参数。所有参数都在此文件中定义。

#### 参数定义格式
```cpp
DEF_INT(parameter_name, OB_CLUSTER_PARAMETER, "default_value", "[range]", 
        "description",
        ObParameterAttr(Section::SECTION_NAME, Source::DEFAULT, EditLevel::DYNAMIC_EFFECTIVE));
```

### 各参数的具体位置

#### 1. 缓存相关参数（第1020-1038行）

```cpp
// src/share/parameter/ob_parameter_seed.ipp:1026
DEF_INT(index_block_cache_priority, OB_CLUSTER_PARAMETER, "10", "[1,)", 
        "index cache priority. Range:[1, )",
        ObParameterAttr(Section::CACHE, Source::DEFAULT, EditLevel::DYNAMIC_EFFECTIVE));

// src/share/parameter/ob_parameter_seed.ipp:1028
DEF_INT(user_block_cache_priority, OB_CLUSTER_PARAMETER, "1", "[1,)", 
        "user block cache priority. Range:[1, )",
        ObParameterAttr(Section::CACHE, Source::DEFAULT, EditLevel::DYNAMIC_EFFECTIVE));

// src/share/parameter/ob_parameter_seed.ipp:1030
DEF_INT(user_row_cache_priority, OB_CLUSTER_PARAMETER, "1", "[1,)", 
        "user row cache priority. Range:[1, )",
        ObParameterAttr(Section::CACHE, Source::DEFAULT, EditLevel::DYNAMIC_EFFECTIVE));
```

#### 2. 共享存储缓存参数（第52-58行）

```cpp
// src/share/parameter/ob_parameter_seed.ipp:53
DEF_INT(ss_cache_max_percentage, OB_CLUSTER_PARAMETER, "30", "(0,100]",
        "the maximum percentage of local cache disk space to total data in shared storage mode. Range: (0,100] in integer",
        ObParameterAttr(Section::SSTABLE, Source::DEFAULT, EditLevel::DYNAMIC_EFFECTIVE));

// src/share/parameter/ob_parameter_seed.ipp:56
DEF_CAP(ss_cache_maxsize_percpu, OB_CLUSTER_PARAMETER, "128G", "(0M,)",
        "the maximum allowed local cache disk size per CPU per server in shared storage mode. Range: (0, +∞)",
        ObParameterAttr(Section::SSTABLE, Source::DEFAULT, EditLevel::DYNAMIC_EFFECTIVE));
```

#### 3. 缓存淘汰阈值（第247-249行）

```cpp
// src/share/parameter/ob_parameter_seed.ipp:247
DEF_CAP(cache_wash_threshold, OB_CLUSTER_PARAMETER, "64M", "[0B,]",
        "size of remaining memory at which cache eviction will be triggered. Range: [0,+∞)",
        ObParameterAttr(Section::OBSERVER, Source::DEFAULT, EditLevel::DYNAMIC_EFFECTIVE));
```

#### 4. IO相关参数（第1222-1226行）

```cpp
// src/share/parameter/ob_parameter_seed.ipp:1222
DEF_CAP(_io_read_batch_size, OB_CLUSTER_PARAMETER, "0K", "[0K,16M]", 
        "Maximum batch size in one read io request. Range:[0K,16M]",
        ObParameterAttr(Section::SSTABLE, Source::DEFAULT, EditLevel::DYNAMIC_EFFECTIVE));

// src/share/parameter/ob_parameter_seed.ipp:1224
DEF_INT(_io_read_redundant_limit_percentage, OB_CLUSTER_PARAMETER, "0", "[0, 99]",
        "Maximum percentage of redundant size in one read io request, redundant data means blocks in the middle of the batch that hit in cache or filtered by skipping index but must be read. Range:[0,99]",
        ObParameterAttr(Section::SSTABLE, Source::DEFAULT, EditLevel::DYNAMIC_EFFECTIVE));

// src/share/parameter/ob_parameter_seed.ipp:1085
DEF_INT(_io_callback_thread_count, OB_CLUSTER_PARAMETER, "0", "[0,128]",
        "The number of io callback threads. The default value is 0. Range: [0,128] in integer. If not specified, The number of threads is dynamically configured according to the memory size",
        ObParameterAttr(Section::OBSERVER, Source::DEFAULT, EditLevel::DYNAMIC_EFFECTIVE));
```

#### 5. 大查询相关参数（第371-378行）

```cpp
// src/share/parameter/ob_parameter_seed.ipp:371
DEF_DBL(large_query_worker_percentage, OB_CLUSTER_PARAMETER, "30", "[0,100]",
        "the percentage of the workers reserved to serve large query request. Range: [0, 100] in percentage",
        ObParameterAttr(Section::TENANT, Source::DEFAULT, EditLevel::DYNAMIC_EFFECTIVE))

// src/share/parameter/ob_parameter_seed.ipp:375
DEF_TIME(large_query_threshold, OB_CLUSTER_PARAMETER, "5s", "[0ms,)",
         "threshold for execution time beyond which a request may be paused and rescheduled as a 'large request', 0ms means disable 'large request'. Range: [0ms, +∞)",
         ObParameterAttr(Section::TENANT, Source::DEFAULT, EditLevel::DYNAMIC_EFFECTIVE));
```

#### 6. 存储流RPC缓冲区（需要查找具体位置）

#### 7. 查询Token数量限制（编译期常量）

```cpp
// src/sql/das/iter/ob_das_text_retrieval_merge_iter.h:36
static const int64_t OB_MAX_TEXT_RETRIEVAL_TOKEN_CNT = 256;
```

**注意**：这是一个编译期常量，不能通过配置修改，需要修改源代码并重新编译。

## 二、参数使用文件

### 1. 参数读取和初始化

**`src/observer/ob_server.cpp`**

#### 缓存优先级参数读取（第2318-2324行）
```cpp
// cache
storage_env_.index_block_cache_priority_ = config_.index_block_cache_priority;
storage_env_.user_block_cache_priority_ = config_.user_block_cache_priority;
storage_env_.user_row_cache_priority_ = config_.user_row_cache_priority;
storage_env_.fuse_row_cache_priority_ = config_.fuse_row_cache_priority;
storage_env_.bf_cache_priority_ = config_.bf_cache_priority;
storage_env_.storage_meta_cache_priority_ = config_.storage_meta_cache_priority;
storage_env_.bf_cache_miss_count_threshold_ = config_.bf_cache_miss_count_threshold;
```

#### 缓存初始化（第2913-2919行）
```cpp
if (OB_FAIL(OB_STORE_CACHE.init(storage_env_.index_block_cache_priority_,
                                storage_env_.user_block_cache_priority_,
                                storage_env_.user_row_cache_priority_,
                                storage_env_.fuse_row_cache_priority_,
                                storage_env_.bf_cache_priority_,
                                storage_env_.bf_cache_miss_count_threshold_,
                                storage_env_.storage_meta_cache_priority_))) {
  LOG_WARN("Fail to init OB_STORE_CACHE, ", KR(ret), K(storage_env_.data_dir_));
}
```

### 2. 缓存套件实现

**`src/storage/blocksstable/ob_storage_cache_suite.cpp`**

#### 缓存初始化（第50-91行）
```cpp
int ObStorageCacheSuite::init(
    const int64_t index_block_cache_priority,
    const int64_t user_block_cache_priority,
    const int64_t user_row_cache_priority,
    const int64_t fuse_row_cache_priority,
    const int64_t bf_cache_priority,
    const int64_t bf_cache_miss_count_threshold,
    const int64_t storage_meta_cache_priority)
{
  // ... 使用这些优先级参数初始化各种缓存
}
```

#### 缓存优先级重置（第94-100行）
```cpp
int ObStorageCacheSuite::reset_priority(
    const int64_t index_block_cache_priority,
    const int64_t user_block_cache_priority,
    const int64_t user_row_cache_priority,
    const int64_t fuse_row_cache_priority,
    const int64_t bf_cache_priority,
    const int64_t storage_meta_cache_priority)
```

### 3. 全文索引查询实现

**`src/sql/das/ob_text_retrieval_op.cpp`**

#### Token数量检查（第151-154行）
```cpp
} else if (OB_UNLIKELY(query_tokens_.count() > OB_MAX_TEXT_RETRIEVAL_TOKEN_CNT)) {
  ret = OB_NOT_SUPPORTED;
  LOG_WARN("too many query tokens in a single query not supported", K(ret), K_(query_tokens));
  LOG_USER_ERROR(OB_NOT_SUPPORTED, "text retrieval query with token count exceed limit");
}
```

## 三、参数设置方式

### 1. 通过SQL语句设置（推荐）

对于 `EditLevel::DYNAMIC_EFFECTIVE` 的参数，可以通过SQL动态修改：

```sql
-- 设置索引块缓存优先级
ALTER SYSTEM SET index_block_cache_priority = 20;

-- 设置用户块缓存优先级
ALTER SYSTEM SET user_block_cache_priority = 5;

-- 设置用户行缓存优先级
ALTER SYSTEM SET user_row_cache_priority = 5;

-- 设置共享存储缓存百分比
ALTER SYSTEM SET ss_cache_max_percentage = 50;

-- 设置IO读取批次大小
ALTER SYSTEM SET _io_read_batch_size = '128K';

-- 设置IO读取冗余限制百分比
ALTER SYSTEM SET _io_read_redundant_limit_percentage = 50;

-- 设置IO回调线程数
ALTER SYSTEM SET _io_callback_thread_count = 8;

-- 设置缓存淘汰阈值
ALTER SYSTEM SET cache_wash_threshold = '128M';

-- 设置大查询工作线程百分比
ALTER SYSTEM SET large_query_worker_percentage = 40;

-- 设置大查询阈值
ALTER SYSTEM SET large_query_threshold = '10s';
```

### 2. 通过配置文件设置

#### OBD部署配置文件
**`tools/deploy/obd/observer.include.yaml`**

示例配置：
```yaml
config:
  index_block_cache_priority: 20
  user_block_cache_priority: 5
  user_row_cache_priority: 5
  ss_cache_max_percentage: 50
  _io_read_batch_size: '128K'
  cache_wash_threshold: '128M'
  large_query_worker_percentage: 40
  large_query_threshold: '10s'
```

### 3. 通过配置服务器（ConfigServer）

如果使用OceanBase的配置服务器（obconfig），可以通过配置服务器统一管理参数。

### 4. 编译期常量修改

对于 `OB_MAX_TEXT_RETRIEVAL_TOKEN_CNT`，需要修改源代码：

**`src/sql/das/iter/ob_das_text_retrieval_merge_iter.h:36`**
```cpp
// 修改前
static const int64_t OB_MAX_TEXT_RETRIEVAL_TOKEN_CNT = 256;

// 修改后（例如改为512）
static const int64_t OB_MAX_TEXT_RETRIEVAL_TOKEN_CNT = 512;
```

修改后需要重新编译整个项目。

## 四、参数生效级别

### EditLevel说明

1. **DYNAMIC_EFFECTIVE**：动态生效，可以通过 `ALTER SYSTEM SET` 修改，立即生效
2. **STATIC_EFFECTIVE**：静态生效，需要重启observer才能生效
3. **READONLY**：只读参数，只能在启动时通过配置文件设置

### 参数类型说明

1. **OB_CLUSTER_PARAMETER**：集群级参数，所有observer共享
2. **OB_TENANT_PARAMETER**：租户级参数，每个租户独立设置

## 五、查看当前参数值

### 通过SQL查看
```sql
-- 查看所有参数
SELECT * FROM oceanbase.__all_virtual_sys_parameter_stat 
WHERE name LIKE '%cache%' OR name LIKE '%io%';

-- 查看特定参数
SELECT * FROM oceanbase.__all_virtual_sys_parameter_stat 
WHERE name = 'index_block_cache_priority';
```

### 通过系统视图查看
```sql
-- 查看系统参数
SHOW PARAMETERS LIKE '%cache%';
SHOW PARAMETERS LIKE '%io%';
```

## 六、参数修改建议

### 修改前检查
1. 确认参数的有效范围
2. 确认参数是否需要重启（EditLevel）
3. 确认参数是集群级还是租户级

### 修改步骤
1. 查看当前值：`SHOW PARAMETERS LIKE 'parameter_name';`
2. 修改参数：`ALTER SYSTEM SET parameter_name = value;`
3. 验证修改：`SHOW PARAMETERS LIKE 'parameter_name';`
4. 观察性能变化

### 注意事项
1. 某些参数修改可能影响系统稳定性，建议在测试环境先验证
2. 缓存相关参数修改后，需要等待一段时间才能看到效果
3. IO相关参数修改需要根据实际硬件配置调整
4. 编译期常量修改需要重新编译，影响较大

## 七、相关文件清单

### 定义文件
- `src/share/parameter/ob_parameter_seed.ipp` - 所有参数定义
- `src/share/parameter/ob_parameter_macro.h` - 参数定义宏
- `src/share/parameter/ob_parameter_attr.h` - 参数属性定义

### 使用文件
- `src/observer/ob_server.cpp` - 参数读取和初始化
- `src/storage/blocksstable/ob_storage_cache_suite.cpp` - 缓存实现
- `src/storage/blocksstable/ob_storage_cache_suite.h` - 缓存接口
- `src/sql/das/ob_text_retrieval_op.cpp` - 全文索引查询实现
- `src/sql/das/iter/ob_das_text_retrieval_merge_iter.h` - Token数量限制常量

### 配置文件
- `tools/deploy/obd/observer.include.yaml` - OBD部署配置示例
- `tools/deploy/single.yaml` - 单机部署配置示例


