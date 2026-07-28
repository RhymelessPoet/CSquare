#pragma once
#include "ExtensibleFlagEnum.h"
#include "Macros.h"
#include <format>
#include <source_location>
#include <string_view>

namespace CS
{

// ---------------------------------------------------------------------------
// LogLevel
// ---------------------------------------------------------------------------
struct LogLevelTag
{
    using UnderlyingType = uint8_t;
    static constexpr size_t NameLength = 16;
    static constexpr size_t MaxCount = 16;
};

using LogLevel = FlagEnum<LogLevelTag>;
using LogLevelFlags = EnumFlags<LogLevelTag>;

struct LogLevels
{
    // clang-format off
    static inline FlagEnumClass<LogLevelTag,
        "trace", "debug", "info", "performance", "warning", "error", "fatal"> Values{};
    // clang-format on

    static LogLevel Trace() { return LogLevel::Make<"trace">(); }
    static LogLevel Debug() { return LogLevel::Make<"debug">(); }
    static LogLevel Info() { return LogLevel::Make<"info">(); }
    static LogLevel Performance() { return LogLevel::Make<"performance">(); }
    static LogLevel Warning() { return LogLevel::Make<"warning">(); }
    static LogLevel Error() { return LogLevel::Make<"error">(); }
    static LogLevel Fatal() { return LogLevel::Make<"fatal">(); }
};

// ---------------------------------------------------------------------------
// LogSink
// ---------------------------------------------------------------------------
struct LogSinkTag
{
    using UnderlyingType = uint8_t;
    static constexpr size_t NameLength = 16;
    static constexpr size_t MaxCount = 16;
};

using LogSink = FlagEnum<LogSinkTag>;
using LogSinks = EnumFlags<LogSinkTag>;

struct LogSinkValues
{
    static inline FlagEnumClass<LogSinkTag, "console", "file"> Values{};

    static LogSink Console() { return LogSink::Make<"console">(); }
    static LogSink File() { return LogSink::Make<"file">(); }
    static LogSinks Both() { return LogSink::Make<"console">() | LogSink::Make<"file">(); }
};

// ---------------------------------------------------------------------------
// LogChannel
// ---------------------------------------------------------------------------
struct LogChannelTag
{
    using UnderlyingType = uint8_t;
    static constexpr size_t NameLength = 24;
    static constexpr size_t MaxCount = 64;
};

using LogChannel = FlagEnum<LogChannelTag>;
using LogChannels = EnumFlags<LogChannelTag>;

struct BuiltInChannels
{
    // clang-format off
    static inline FlagEnumClass<LogChannelTag,
        "Engine", "Graphics", "Render", "Asset", "Editor", "General"> Values{};
    // clang-format on

    static LogChannel Engine() { return LogChannel::Make<"Engine">(); }
    static LogChannel Graphics() { return LogChannel::Make<"Graphics">(); }
    static LogChannel Render() { return LogChannel::Make<"Render">(); }
    static LogChannel Asset() { return LogChannel::Make<"Asset">(); }
    static LogChannel Editor() { return LogChannel::Make<"Editor">(); }
    static LogChannel General() { return LogChannel::Make<"General">(); }
};

// ---------------------------------------------------------------------------
// Logger
// ---------------------------------------------------------------------------
class Logger
{
public:
    class SourceLocationCaptureProhibition
    {
    public:
        SourceLocationCaptureProhibition();
        ~SourceLocationCaptureProhibition();
    };
    friend class SourceLocationCaptureProhibition;
    CS_DELETE_COPY_MOVE(Logger)

    static Logger& Instance();
    static bool IsInitialized();

    static void Initialize(std::string_view logFilePath = "",
                           LogSinks sinks = LogSinkValues::Both(),
                           LogLevelFlags enabledLevels = LogLevelFlags{},
                           bool showPid = false,
                           bool showTid = false,
                           uint64_t maxFileSize = 0);

    static void Shutdown();

    void SetEnabledLevels(LogLevelFlags levels);
    void SetSinks(LogSinks sinks);

    LogLevelFlags GetEnabledLevels() const;
    LogSinks GetSinks() const;

    bool ShouldLog(LogLevel level) const;

    void Emit(LogLevel level, LogChannel channel, std::source_location loc, std::string_view message) const;
    void Emit(LogLevel level, LogChannel channel, std::string_view message) const;

private:
    Logger() = default;
    ~Logger() = default;

    struct Impl;
    Impl* m_impl{nullptr};
};

// ---------------------------------------------------------------------------
// CS::Log / CS::LogError / … – zero-macro log API
//
// source_location is captured as a trailing default argument of non-template
// inline functions.  This is the one pattern that works correctly on MSVC:
// the default argument is evaluated at the call site in the translation unit
// that calls LogError, not inside the logger implementation.
//
// Call-site syntax:
//
//   // Plain string:
//   CS::LogError(BuiltInChannels::Render(), "something went wrong");
//
//   // Formatted string (std::format is evaluated before the call, so
//   // source_location::current() still resolves to the correct line):
//   CS::LogError(BuiltInChannels::Render(), std::format("value = {}", v));
//
//   // Convenience: CS::Fmt(fmt, args...) wraps std::format
//   CS::LogError(BuiltInChannels::Render(), CS::Fmt("value = {}", v));
//
// Zero macros.  No #define CS_MODULE.  No __func__.
// ---------------------------------------------------------------------------

// Fmt – convenience wrapper around std::format for use at log call sites.
template <typename... Args>
[[nodiscard]] inline std::string Fmt(std::format_string<Args...> fmt, Args&&... args)
{
    return std::format(fmt, std::forward<Args>(args)...);
}

// clang-format off
inline void Log(LogLevel l, LogChannel ch, std::string_view msg, std::source_location loc = std::source_location::current())
{ 
    if (Logger::IsInitialized() && Logger::Instance().ShouldLog(l)) {
        Logger::Instance().Emit(l, ch, loc, msg);
        Logger::Instance().Emit(l, ch, msg);
    }
}
inline void LogTrace  (LogChannel ch, std::string_view msg, std::source_location loc = std::source_location::current()) { Log(LogLevels::Trace(),   ch, msg, loc); }
inline void LogDebug  (LogChannel ch, std::string_view msg, std::source_location loc = std::source_location::current()) { Log(LogLevels::Debug(),   ch, msg, loc); }
inline void LogInfo   (LogChannel ch, std::string_view msg, std::source_location loc = std::source_location::current()) { Log(LogLevels::Info(),    ch, msg, loc); }
inline void LogPerf   (LogChannel ch, std::string_view msg, std::source_location loc = std::source_location::current()) { Log(LogLevels::Performance(), ch, msg, loc); }
inline void LogWarning(LogChannel ch, std::string_view msg, std::source_location loc = std::source_location::current()) { Log(LogLevels::Warning(), ch, msg, loc); }
inline void LogError  (LogChannel ch, std::string_view msg, std::source_location loc = std::source_location::current()) { Log(LogLevels::Error(),   ch, msg, loc); }
inline void LogFatal  (LogChannel ch, std::string_view msg, std::source_location loc = std::source_location::current()) { Log(LogLevels::Fatal(),   ch, msg, loc); }
// clang-format on

} // namespace CS
