# Logger 日志系统设计与使用

## 目录

- [1. 概述](#1-概述)
- [2. 架构设计](#2-架构设计)
- [3. 类型系统](#3-类型系统)
  - [3.1 LogLevel — 日志级别](#31-loglevel--日志级别)
  - [3.2 LogSink — 输出目标](#32-logsink--输出目标)
  - [3.3 LogChannel — 模块频道](#33-logchannel--模块频道)
- [4. Logger — 单例核心](#4-logger--单例核心)
  - [4.1 初始化与关闭](#41-初始化与关闭)
  - [4.2 运行时配置](#42-运行时配置)
- [5. 调用 API](#5-调用-api)
  - [5.1 CS::Fmt — 格式化辅助](#51-csfmt--格式化辅助)
  - [5.2 CS::LogXxx — 各级别函数](#52-cslogxxx--各级别函数)
  - [5.3 CS::Log — 通用入口](#53-cslog--通用入口)
- [6. 输出格式](#6-输出格式)
- [7. JSON 配置（CSEditor）](#7-json-配置cseditor)
- [8. 自定义频道](#8-自定义频道)
- [9. 实现细节](#9-实现细节)
  - [9.1 source_location 捕获机制](#91-source_location-捕获机制)
  - [9.2 函数名提取](#92-函数名提取)
  - [9.3 线程安全](#93-线程安全)

---

## 1. 概述

| 属性 | 值 |
|------|----|
| 头文件 | `source/engine/base/Logger.h` |
| 实现 | `source/engine/base/Logger.cpp` |
| 命名空间 | `CS` |
| 依赖 | C++20 标准库、`ExtensibleFlagEnum.h` |
| 宏 | **零宏** — 无 `#define CS_MODULE`，无 `__func__` |

日志系统的设计目标：

- **零宏调用** — 利用 `std::source_location` 的默认参数机制自动捕获调用位置，无需任何预处理器宏
- **可扩展类型** — 级别、Sink、频道均基于 `ExtensibleFlagEnum`，可在任意翻译单元中注册新值
- **格式化安全** — 通过 `CS::Fmt()` 包装 `std::format`，编译期检查格式字符串
- **运行时可配置** — 级别过滤、输出目标、pid/tid 显示均可在运行时或通过 JSON 配置文件修改

---

## 2. 架构设计

```
┌─────────────────────────────────────────────────────┐
│  调用方                                              │
│  CS::LogError(ch, CS::Fmt("binding {} OOB", idx))   │
└───────────────────┬─────────────────────────────────┘
                    │  std::source_location（自动）
                    ▼
┌─────────────────────────────────────────────────────┐
│  CS::LogError / LogInfo / Log（inline，Logger.h）    │
│  • 检查 Logger::IsInitialized()                      │
│  • 调用 Logger::Instance().Emit(level, ch, loc, msg) │
└───────────────────┬─────────────────────────────────┘
                    ▼
┌─────────────────────────────────────────────────────┐
│  Logger::Emit（Logger.cpp）                          │
│  • ShouldLog 级别过滤                                │
│  • 提取文件名 stem、简短函数名                        │
│  • 组装 [time][level][channel] File::Func(L): msg    │
│  • 分发到 Console Sink / File Sink                   │
└─────────────────────────────────────────────────────┘
```

---

## 3. 类型系统

所有类型均定义在 `Logger.h`，基于 `ExtensibleFlagEnum` 实现。

### 3.1 LogLevel — 日志级别

```cpp
// source/engine/base/Logger.h
using LogLevel = FlagEnum<LogLevelTag>;

struct LogLevels {
    static LogLevel Trace();
    static LogLevel Debug();
    static LogLevel Info();
    static LogLevel Warning();
    static LogLevel Error();
    static LogLevel Fatal();
};
```

级别按顺序递增，`minLevel` 过滤时只输出 `>= minLevel` 的条目。

| 值 | 用途 |
|----|------|
| `Trace` | 最细粒度的跟踪信息，仅在深度调试时启用 |
| `Debug` | 开发阶段诊断信息 |
| `Info` | 正常流程关键节点（默认最低级别） |
| `Warning` | 潜在问题但程序可继续运行 |
| `Error` | 操作失败，功能受损 |
| `Fatal` | 严重错误，程序可能无法继续 |

### 3.2 LogSink — 输出目标

```cpp
using LogSink  = FlagEnum<LogSinkTag>;
using LogSinks = EnumFlags<LogSinkTag>;   // 可组合标志

struct LogSinkValues {
    static LogSink  Console();            // 标准输出（含 ANSI 着色）
    static LogSink  File();               // 文件追加写入
    static LogSinks Both();               // Console | File
};
```

`LogSinks` 是可组合的标志集合，可按位或运算：

```cpp
CS::Logger::Initialize("app.log", CS::LogSinkValues::Both());
CS::Logger::Initialize("",        CS::LogSinkValues::Console());
CS::Logger::Initialize("app.log", CS::LogSinkValues::File());
```

### 3.3 LogChannel — 模块频道

```cpp
using LogChannel  = FlagEnum<LogChannelTag>;
using LogChannels = EnumFlags<LogChannelTag>;

struct BuiltInChannels {
    static LogChannel Engine();
    static LogChannel Render();
    static LogChannel Asset();
    static LogChannel Editor();
    static LogChannel General();
};
```

频道是纯标注信息，不参与过滤。每条日志必须指定一个频道，便于在日志文件中快速定位来源模块。

---

## 4. Logger — 单例核心

### 4.1 初始化与关闭

```cpp
// 签名（Logger.h）
static void Initialize(
    std::string_view logFilePath = "",
    LogSinks         sinks       = LogSinkValues::Both(),
    LogLevel         minLevel    = LogLevels::Info(),
    bool             showPid     = false,
    bool             showTid     = false
);

static void Shutdown();
```

`Initialize` **必须在任何日志调用前执行**。未初始化时，`LogXxx` 函数会检查 `IsInitialized()` 并静默跳过，不会崩溃。

`Shutdown` 刷新并关闭文件流，重置初始化状态。

**CSEditor 启动流程（`main.cpp`）：**

```cpp
#include "LoggerConfig.h"

int main(int argc, char* argv[])
{
    CSEditor::LoggerConfig::Load("editor_config.json").Apply();
    // ... 引擎初始化 ...
    CS::Logger::Shutdown();
    return 0;
}
```

### 4.2 运行时配置

初始化后可动态调整：

```cpp
CS::Logger::Instance().SetMinLevel(CS::LogLevels::Warning());
CS::Logger::Instance().SetSinks(CS::LogSinkValues::Console());
```

---

## 5. 调用 API

### 5.1 CS::Fmt — 格式化辅助

```cpp
template <typename... Args>
[[nodiscard]] std::string Fmt(std::format_string<Args...> fmt, Args&&... args);
```

`CS::Fmt` 是对 `std::format` 的薄封装，格式字符串在**编译期**验证。在需要格式化参数时使用：

```cpp
CS::LogError(CS::BuiltInChannels::Asset(),
             CS::Fmt("file not found: {}", path.string()));
```

无格式参数时，直接传递字符串字面量即可（避免不必要的 `Fmt` 调用）：

```cpp
CS::LogError(CS::BuiltInChannels::Render(), "pipeline compile failed");
```

### 5.2 CS::LogXxx — 各级别函数

```cpp
void LogTrace  (LogChannel ch, std::string_view msg,
                std::source_location loc = std::source_location::current());
void LogDebug  (LogChannel ch, std::string_view msg,
                std::source_location loc = std::source_location::current());
void LogInfo   (LogChannel ch, std::string_view msg,
                std::source_location loc = std::source_location::current());
void LogWarning(LogChannel ch, std::string_view msg,
                std::source_location loc = std::source_location::current());
void LogError  (LogChannel ch, std::string_view msg,
                std::source_location loc = std::source_location::current());
void LogFatal  (LogChannel ch, std::string_view msg,
                std::source_location loc = std::source_location::current());
```

`std::source_location` 的默认参数在**调用处**求值，自动记录文件名、函数名、行号，无需任何宏。

**常用写法汇总：**

```cpp
// 纯字符串
CS::LogInfo(CS::BuiltInChannels::Engine(), "engine started");

// 带格式参数（必须用 CS::Fmt 包裹）
CS::LogWarning(CS::BuiltInChannels::Render(),
               CS::Fmt("material '{}' missing slot {}", name, slot));

// std::format 等价写法（同样合法）
CS::LogDebug(CS::BuiltInChannels::Asset(),
             std::format("loaded {} vertices", mesh.vertexCount));
```

### 5.3 CS::Log — 通用入口

当需要动态决定级别时可使用：

```cpp
void Log(LogLevel level, LogChannel ch, std::string_view msg,
         std::source_location loc = std::source_location::current());
```

```cpp
auto level = isCritical ? CS::LogLevels::Error() : CS::LogLevels::Warning();
CS::Log(level, CS::BuiltInChannels::General(), CS::Fmt("result: {}", code));
```

---

## 6. 输出格式

### 默认格式（showPid = false, showTid = false）

```
[time              ] [level  ] [channel] FileStem::FuncName(line): message
```

示例：

```
[2026-05-07 01:23:45.678] [error  ] [Render] MaterialInstance::Apply(131): Uniform 'glossiness' not found during Apply
[2026-05-07 01:23:45.679] [info   ] [Asset ] AssetLoader::Load(22): loaded room.gltf
[2026-05-07 01:23:45.680] [warning] [Render] Material::Connect(79): Duplicate Connect from slot
```

### 启用 pid/tid 后

```
[time              ] [level  ] [channel] [pid:12345] [tid:a1b2c3d4e5f60708] FileStem::FuncName(line): message
```

### 字段说明

| 字段 | 内容 | 说明 |
|------|------|------|
| `time` | `YYYY-MM-DD HH:MM:SS.mmm` | 本地时间，毫秒精度 |
| `level` | `trace` / `debug` / `info` / `warning` / `error` / `fatal` | 日志级别 |
| `channel` | `Engine` / `Render` / `Asset` / `Editor` / … | 来源模块频道 |
| `pid` | 十进制进程 ID | `showPid = true` 时出现 |
| `tid` | 十六进制线程 ID | `showTid = true` 时出现 |
| `FileStem` | 源文件名（无路径、无扩展名） | 自动从 `source_location` 提取 |
| `FuncName` | 简短函数名（无命名空间、无参数） | 自动提取，跨编译器 |
| `line` | 行号 | 自动从 `source_location` 提取 |
| `message` | 日志内容 | 调用方提供 |

### 控制台着色（ANSI）

| 级别 | 颜色 |
|------|------|
| `trace` | 深灰 |
| `debug` | 青色 |
| `info` | 绿色 |
| `warning` | 黄色 |
| `error` | 红色（输出到 stderr） |
| `fatal` | 品红（输出到 stderr） |

---

## 7. JSON 配置（CSEditor）

CSEditor 通过 `source/editor/editor_config.json` 在启动时配置日志系统，无需重新编译。

### 文件位置

运行时工作目录下的 `editor_config.json`（CMake 在构建时通过 post-build 命令复制到可执行文件目录）。

### 完整配置项

```json
{
    "logger": {
        "file":    "CSEditor.log",
        "sinks":   ["console", "file"],
        "minLevel": "debug",
        "showPid": false,
        "showTid": false
    }
}
```

| 键 | 类型 | 默认值 | 说明 |
|----|------|--------|------|
| `file` | string | `"CSEditor.log"` | 日志文件路径；空字符串表示不写文件 |
| `sinks` | string[] | `["console","file"]` | 输出目标，可选值：`"console"`、`"file"` |
| `minLevel` | string | `"info"` | 最低输出级别：`trace`/`debug`/`info`/`warning`/`error`/`fatal` |
| `showPid` | bool | `false` | 是否在每行输出进程 ID |
| `showTid` | bool | `false` | 是否在每行输出线程 ID |

文件缺失或 JSON 格式错误时，`LoggerConfig::Load` 静默回退到安全默认值（Console-only，`info` 级别），程序不会崩溃。

### 加载流程

```
main.cpp
  └─ CSEditor::LoggerConfig::Load("editor_config.json")
       ├─ QFile 读取 JSON
       ├─ 解析 logger 节点
       └─ .Apply()
            └─ CS::Logger::Initialize(file, sinks, minLevel, showPid, showTid)
```

相关源文件：
- `source/editor/LoggerConfig.h`
- `source/editor/LoggerConfig.cpp`

---

## 8. 自定义频道

`LogChannel` 基于 `ExtensibleFlagEnum`，支持在任意翻译单元中注册新频道：

```cpp
// MyModule.cpp
#include "base/Logger.h"

// 在本翻译单元注册自定义频道
static inline CS::FlagEnumClass<CS::LogChannelTag, "Physics"> PhysicsChannels{};

static CS::LogChannel Channel()
{
    return CS::LogChannel::Make<"Physics">();
}

void PhysicsWorld::Step()
{
    CS::LogDebug(Channel(), CS::Fmt("step dt={:.4f}", dt));
}
```

注意：`FlagEnumClass` 是静态存储期对象，注册在程序启动时（静态初始化阶段）自动完成。同名频道只需注册一次；多次注册同一名称是安全的（幂等）。

---

## 9. 实现细节

### 9.1 source_location 捕获机制

MSVC 不支持在自由函数模板的参数包后面放置带默认值的 `std::source_location` 参数（无法完成模板推导）。本系统采用如下方案绕过该限制：

- `CS::LogError` 等函数是**非模板 inline 函数**，接受 `std::string_view msg`
- `source_location` 作为这些非模板函数的**最后一个带默认值参数**，MSVC 能正确在调用处求值
- 带格式参数的日志调用先通过 `CS::Fmt(...)` 生成 `std::string`，再传入 `LogError`；`Fmt` 内部使用 `std::format_string<Args...>` 保证编译期格式检查

```cpp
// Logger.h — 关键模式
inline void LogError(LogChannel ch, std::string_view msg,
                     std::source_location loc = std::source_location::current())
{
    Log(LogLevels::Error(), ch, msg, loc);
}
```

`loc` 的默认参数在**每个调用处**独立求值，因此 `LogError(ch, msg)` 中的 `loc` 始终指向源码中写下该调用的那一行。

### 9.2 函数名提取

`std::source_location::function_name()` 在 MSVC 返回完整修饰签名，例如：

```
void __cdecl CS::MaterialInstance::Apply(class CS::MaterialCompiler &)
```

`Logger::Emit` 中通过以下逻辑提取简短名称：

1. 找到第一个 `(` 的位置
2. 从该位置向前扫描，提取由字母、数字、下划线组成的标识符
3. 结果：`Apply`

此方法在 GCC/Clang 上同样有效（它们的 `function_name()` 通常已经是简短形式）。

### 9.3 线程安全

`Logger::Emit` 在格式化完成后才加锁（`std::lock_guard lock(m_impl->mutex)`），锁的粒度仅覆盖实际 I/O 写入，格式化操作在锁外进行，减少竞争：

```cpp
// 格式化（无锁）
std::string line = std::format(...);

// I/O（持锁）
std::lock_guard lock(m_impl->mutex);
std::fprintf(out, ...);
m_impl->fileStream << line;
```

`Initialize` 和 `Shutdown` 同样持锁，保证多线程环境下的配置一致性。
