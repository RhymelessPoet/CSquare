#pragma once
#include "base/Logger.h"
#include <string>

namespace CSEditor
{

// Reads logger settings from a JSON config file and calls CS::Logger::Initialize.
//
// Expected JSON layout:
// {
//   "logger": {
//     "file":        "CSEditor.log",             // log file path; empty string = no file output
//     "sinks":       ["console","file"],          // array of sink names
//     "levels":      ["debug","info","warning",
//                    "error","fatal"],             // array of enabled level names
//     "showPid":     false,                       // include process ID in log lines
//     "showTid":     false,                       // include thread  ID in log lines
//     "maxFileSize": 0                            // max log file size in bytes; 0 = unlimited
//   }
// }
//
// Backward-compat: if "levels" is absent but "minLevel" is present, it is
// expanded to the equivalent set of levels (everything >= that level).
// If the config file is absent or malformed, the method falls back to safe defaults
// (console-only, info|warning|error|fatal) and still calls Logger::Initialize so
// the application can proceed without crashing.
struct LoggerConfig
{
    std::string logFile{"CSEditor.log"};
    CS::LogSinks sinks{CS::LogSinkValues::Both()};
    CS::LogLevelFlags enabledLevels{CS::LogLevels::Info() | CS::LogLevels::Warning() | CS::LogLevels::Error() |
                                    CS::LogLevels::Fatal()};
    bool showPid{false};
    bool showTid{false};
    uint64_t maxFileSize{0}; // 0 = unlimited

    // Load from a JSON file next to the executable.
    static LoggerConfig Load(const std::string& configPath);

    void Apply() const { CS::Logger::Initialize(logFile, sinks, enabledLevels, showPid, showTid, maxFileSize); }
};

} // namespace CSEditor
