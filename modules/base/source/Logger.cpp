#include "Logger.h"
#include <chrono>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <source_location>
#include <thread>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <process.h>
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

namespace CS
{

// ---------------------------------------------------------------------------
// Impl
// ---------------------------------------------------------------------------
struct Logger::Impl
{
    mutable std::mutex mutex;
    LogSinks sinks{LogSinkValues::Both()};
    LogLevelFlags enabledLevels{LogLevels::Info() | LogLevels::Warning() | LogLevels::Error() | LogLevels::Fatal()};
    bool showPid{false};
    bool showTid{false};
    uint64_t maxFileSize{0}; // 0 = unlimited
    std::ofstream fileStream;
    std::string filePath; // stored for rotation
    bool initialized{false};

    std::atomic<bool> prohibitSourceLocationCapture{false};

    void RotateIfNeeded()
    {
        if (maxFileSize == 0 || !fileStream.is_open())
            return;

        auto pos = fileStream.tellp();
        if (pos < 0 || static_cast<uint64_t>(pos) <= maxFileSize)
            return;

        // Exceeded maxFileSize: keep the latter half (≈ maxFileSize/2 bytes)
        fileStream.close();

        // Read entire file
        std::ifstream in(filePath, std::ios::binary | std::ios::ate);
        if (!in.is_open()) {
            fileStream.open(filePath, std::ios::out | std::ios::app);
            return;
        }

        auto totalSize = static_cast<size_t>(in.tellg());
        size_t keepSize = static_cast<size_t>(maxFileSize / 2);
        if (keepSize >= totalSize) {
            in.close();
            fileStream.open(filePath, std::ios::out | std::ios::app);
            return;
        }

        // Seek to start of kept region, then forward to next line start
        size_t seekPos = totalSize - keepSize;
        in.seekg(static_cast<std::streamoff>(seekPos));
        if (seekPos > 0) {
            std::string discard;
            std::getline(in, discard); // skip partial line
        }

        // Read the kept portion
        std::string kept(std::istreambuf_iterator<char>(in), {});
        in.close();

        // Write back in truncate mode
        fileStream.open(filePath, std::ios::out | std::ios::trunc);
        if (fileStream.is_open() && !kept.empty()) {
            fileStream << kept;
            if (kept.back() != '\n')
                fileStream << '\n';
            fileStream.flush();
        }

        // Reopen in append mode for subsequent writes
        fileStream.close();
        fileStream.open(filePath, std::ios::out | std::ios::app);
    }

    static int ProcessID()
    {
#if defined(_WIN32)
        return static_cast<int>(::GetCurrentProcessId());
#else
        return static_cast<int>(::getpid());
#endif
    }

    static uint64_t ThreadID() { return std::hash<std::thread::id>{}(std::this_thread::get_id()); }

    static std::string Timestamp()
    {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto time_t_now = system_clock::to_time_t(now);
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

        std::tm tm{};
#if defined(_WIN32)
        ::localtime_s(&tm, &time_t_now);
#else
        ::localtime_r(&time_t_now, &tm);
#endif
        return std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d}", tm.tm_year + 1900, tm.tm_mon + 1,
                           tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, static_cast<int>(ms.count()));
    }

    static std::string_view LevelColor(std::string_view levelName)
    {
        if (levelName == "trace")
            return "\033[90m";
        if (levelName == "debug")
            return "\033[36m";
        if (levelName == "info")
            return "\033[32m";
        if (levelName == "performance")
            return "\033[95m";
        if (levelName == "warning")
            return "\033[33m";
        if (levelName == "error")
            return "\033[31m";
        if (levelName == "fatal")
            return "\033[35m";
        return "";
    }

    static constexpr std::string_view kColorReset = "\033[0m";
};

// ---------------------------------------------------------------------------
// Statics
// ---------------------------------------------------------------------------
static bool s_initialized{false};

Logger::SourceLocationCaptureProhibition::SourceLocationCaptureProhibition()
{
    Logger::Instance().m_impl->prohibitSourceLocationCapture = true;
}

Logger::SourceLocationCaptureProhibition::~SourceLocationCaptureProhibition()
{
    Logger::Instance().m_impl->prohibitSourceLocationCapture = false;
}

// ---------------------------------------------------------------------------
// Logger public API
// ---------------------------------------------------------------------------

Logger& Logger::Instance()
{
    static Logger inst;
    return inst;
}

bool Logger::IsInitialized()
{
    return s_initialized;
}

void Logger::Initialize(std::string_view logFilePath,
                        LogSinks sinks,
                        LogLevelFlags enabledLevels,
                        bool showPid,
                        bool showTid,
                        uint64_t maxFileSize)
{
    auto& inst = Instance();
    if (inst.m_impl == nullptr)
        inst.m_impl = new Impl();

    std::lock_guard lock(inst.m_impl->mutex);
    inst.m_impl->sinks = sinks;
    if (enabledLevels.Any())
        inst.m_impl->enabledLevels = enabledLevels;
    inst.m_impl->showPid = showPid;
    inst.m_impl->showTid = showTid;
    inst.m_impl->maxFileSize = maxFileSize;

    if (!logFilePath.empty() && sinks.Test(LogSinkValues::File())) {
        inst.m_impl->filePath = logFilePath;
        inst.m_impl->fileStream.open(std::string(logFilePath), std::ios::out | std::ios::app);
    }
    inst.m_impl->initialized = true;
    s_initialized = true;
}

void Logger::Shutdown()
{
    auto& inst = Instance();
    if (inst.m_impl == nullptr)
        return;

    // Transfer ownership and null out the instance pointer inside the
    // critical section, then release the lock before deleting the Impl.
    // The lock_guard must not outlive the mutex it guards — deleting
    // inst.m_impl while the lock is still held would make ~lock_guard
    // unlock a destroyed mutex (use-after-free).
    Impl* victim = nullptr;
    {
        std::lock_guard lock(inst.m_impl->mutex);
        if (inst.m_impl->fileStream.is_open()) {
            inst.m_impl->fileStream.flush();
            inst.m_impl->fileStream.close();
        }
        inst.m_impl->initialized = false;
        s_initialized = false;
        victim = inst.m_impl;
        inst.m_impl = nullptr;
    }
    delete victim;
}

void Logger::SetEnabledLevels(LogLevelFlags levels)
{
    if (m_impl == nullptr)
        return;
    std::lock_guard lock(m_impl->mutex);
    m_impl->enabledLevels = levels;
}

void Logger::SetSinks(LogSinks sinks)
{
    if (m_impl == nullptr)
        return;
    std::lock_guard lock(m_impl->mutex);
    m_impl->sinks = sinks;
}

LogLevelFlags Logger::GetEnabledLevels() const
{
    if (m_impl == nullptr)
        return LogLevelFlags{};
    return m_impl->enabledLevels;
}

LogSinks Logger::GetSinks() const
{
    if (m_impl == nullptr)
        return LogSinkValues::Both();
    return m_impl->sinks;
}

bool Logger::ShouldLog(LogLevel level) const
{
    if (m_impl == nullptr || !m_impl->initialized)
        return false;
    return m_impl->enabledLevels.Test(level);
}

void Logger::Emit(LogLevel level, LogChannel channel, std::source_location loc, std::string_view message) const
{
    if (m_impl == nullptr || !m_impl->initialized || m_impl->prohibitSourceLocationCapture) {
        return;
    }

    std::string_view levelName = level;
    std::string_view channelName = channel;

    // ---- file stem ----
    std::string_view filePath = loc.file_name();
    auto lastSep = filePath.find_last_of("/\\");
    std::string_view fileBase = (lastSep != std::string_view::npos) ? filePath.substr(lastSep + 1) : filePath;
    auto dotPos = fileBase.rfind('.');
    std::string_view fileStem = (dotPos != std::string_view::npos) ? fileBase.substr(0, dotPos) : fileBase;

    // ---- simple function name ----
    // loc.function_name() on MSVC returns the full decorated signature, e.g.
    //   "void __cdecl CS::MaterialInstance::Apply(class CS::MaterialCompiler &)"
    // Extract just the last unqualified name before the first '('.

    std::string_view funcFull = loc.function_name();
    std::string_view funcName = funcFull;
    {
        auto parenPos = funcFull.find('(');
        if (parenPos != std::string_view::npos) {
            auto nameEnd = parenPos;
            auto nameStart = nameEnd;
            while (nameStart > 0 && (std::isalnum(static_cast<unsigned char>(funcFull[nameStart - 1])) ||
                                     funcFull[nameStart - 1] == '_'))
                --nameStart;
            if (nameStart < nameEnd)
                funcName = funcFull.substr(nameStart, nameEnd - nameStart);
        }
    }

    auto timestamp = Impl::Timestamp();

    // ---- assemble log line ----
    // Base format: [time] [level  ] [channel] FileStem::FuncName(line): message
    // pid/tid are appended only when enabled.
    std::string line;
    if (m_impl->showPid && m_impl->showTid) {
        auto pid = Impl::ProcessID();
        auto tid = Impl::ThreadID();
        line = std::format("[{}] [{}] [{}] [pid:{}] [tid:{:x}] {}::{}({}): {}", timestamp, levelName, channelName, pid,
                           tid, fileStem, funcName, loc.line(), message);
    } else if (m_impl->showPid) {
        line = std::format("[{}] [{}] [{}] [pid:{}] {}::{}({}): {}", timestamp, levelName, channelName,
                           Impl::ProcessID(), fileStem, funcName, loc.line(), message);
    } else if (m_impl->showTid) {
        line = std::format("[{}] [{}] [{}] [tid:{:x}] {}::{}({}): {}", timestamp, levelName, channelName,
                           Impl::ThreadID(), fileStem, funcName, loc.line(), message);
    } else {
        line = std::format("[{}] [{}] [{}] {}::{}({}): {}", timestamp, levelName, channelName, fileStem, funcName,
                           loc.line(), message);
    }

    std::lock_guard lock(m_impl->mutex);

    if (m_impl->sinks.Test(LogSinkValues::Console())) {
        auto color = Impl::LevelColor(levelName);
        auto* out = (levelName == "error" || levelName == "fatal") ? stderr : stdout;
        std::fprintf(out, "%s%s%s\n", color.data(), line.c_str(), Impl::kColorReset.data());
        std::fflush(out);
    }

    if (m_impl->sinks.Test(LogSinkValues::File()) && m_impl->fileStream.is_open()) {
        m_impl->fileStream << line << '\n';
        m_impl->RotateIfNeeded();
    }
}

void Logger::Emit(LogLevel level, LogChannel channel, std::string_view message) const
{
    if (m_impl == nullptr || !m_impl->initialized || !m_impl->prohibitSourceLocationCapture) {
        return;
    }

    std::string_view levelName = level;
    std::string_view channelName = channel;

    auto timestamp = Impl::Timestamp();

    // ---- assemble log line ----
    // Base format: [time] [level  ] [channel]: message
    // pid/tid are appended only when enabled.
    std::string line;
    if (m_impl->showPid && m_impl->showTid) {
        auto pid = Impl::ProcessID();
        auto tid = Impl::ThreadID();
        line =
            std::format("[{}] [{}] [{}] [pid:{}] [tid:{:x}]: {}", timestamp, levelName, channelName, pid, tid, message);
    } else if (m_impl->showPid) {
        line =
            std::format("[{}] [{}] [{}] [pid:{}]: {}", timestamp, levelName, channelName, Impl::ProcessID(), message);
    } else if (m_impl->showTid) {
        line =
            std::format("[{}] [{}] [{}] [tid:{:x}]: {}", timestamp, levelName, channelName, Impl::ThreadID(), message);
    } else {
        line = std::format("[{}] [{}] [{}]: {}", timestamp, levelName, channelName, message);
    }

    std::lock_guard lock(m_impl->mutex);

    if (m_impl->sinks.Test(LogSinkValues::Console())) {
        auto color = Impl::LevelColor(levelName);
        auto* out = (levelName == "error" || levelName == "fatal") ? stderr : stdout;
        std::fprintf(out, "%s%s%s\n", color.data(), line.c_str(), Impl::kColorReset.data());
        std::fflush(out);
    }

    if (m_impl->sinks.Test(LogSinkValues::File()) && m_impl->fileStream.is_open()) {
        m_impl->fileStream << line << '\n';
        m_impl->RotateIfNeeded();
    }
}

} // namespace CS
