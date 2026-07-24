#include "LoggerConfig.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

namespace CSEditor
{

LoggerConfig LoggerConfig::Load(const std::string& configPath)
{
    LoggerConfig cfg;

    QFile file(QString::fromStdString(configPath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // File absent – return defaults silently (logger not yet initialized)
        return cfg;
    }

    QJsonParseError parseError;
    auto doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (doc.isNull() || !doc.isObject()) {
        return cfg;
    }

    auto root = doc.object();
    auto loggerNode = root.value("logger");
    if (!loggerNode.isObject()) {
        return cfg;
    }

    auto loggerObj = loggerNode.toObject();

    // ---- log file path ----
    if (loggerObj.contains("file")) {
        cfg.logFile = loggerObj.value("file").toString().toStdString();
    }

    // ---- sinks ----
    if (loggerObj.contains("sinks") && loggerObj.value("sinks").isArray()) {
        CS::LogSinks sinks{};
        for (const auto& entry : loggerObj.value("sinks").toArray()) {
            auto name = entry.toString().toLower().toStdString();
            if (name == "console")
                sinks |= CS::LogSinkValues::Console();
            else if (name == "file")
                sinks |= CS::LogSinkValues::File();
        }
        cfg.sinks = sinks;
    }

    // ---- enabled levels (array) ----
    if (loggerObj.contains("levels") && loggerObj.value("levels").isArray()) {
        CS::LogLevelFlags levels{};
        for (const auto& entry : loggerObj.value("levels").toArray()) {
            auto name = entry.toString().toLower().toStdString();
            if (name == "trace")
                levels |= CS::LogLevels::Trace();
            else if (name == "debug")
                levels |= CS::LogLevels::Debug();
            else if (name == "info")
                levels |= CS::LogLevels::Info();
            else if (name == "performance")
                levels |= CS::LogLevels::Performance();
            else if (name == "warning")
                levels |= CS::LogLevels::Warning();
            else if (name == "error")
                levels |= CS::LogLevels::Error();
            else if (name == "fatal")
                levels |= CS::LogLevels::Fatal();
        }
        if (levels.Any())
            cfg.enabledLevels = levels;
    }
    // ---- minLevel (backward compat, only used when "levels" absent) ----
    else if (loggerObj.contains("minLevel"))
    {
        auto levelStr = loggerObj.value("minLevel").toString().toLower().toStdString();
        CS::LogLevel threshold = CS::LogLevels::Info();
        if (levelStr == "trace")
            threshold = CS::LogLevels::Trace();
        else if (levelStr == "debug")
            threshold = CS::LogLevels::Debug();
        else if (levelStr == "info")
            threshold = CS::LogLevels::Info();
        else if (levelStr == "performance")
            threshold = CS::LogLevels::Performance();
        else if (levelStr == "warning")
            threshold = CS::LogLevels::Warning();
        else if (levelStr == "error")
            threshold = CS::LogLevels::Error();
        else if (levelStr == "fatal")
            threshold = CS::LogLevels::Fatal();
        // Expand to all registered levels >= threshold
        auto t = static_cast<uint8_t>(threshold);
        CS::LogLevelFlags levels{};
        auto setIfGeq = [&](CS::LogLevel lv) {
            if (static_cast<uint8_t>(lv) >= t)
                levels.Set(lv);
        };
        setIfGeq(CS::LogLevels::Trace());
        setIfGeq(CS::LogLevels::Debug());
        setIfGeq(CS::LogLevels::Info());
        setIfGeq(CS::LogLevels::Performance());
        setIfGeq(CS::LogLevels::Warning());
        setIfGeq(CS::LogLevels::Error());
        setIfGeq(CS::LogLevels::Fatal());
        cfg.enabledLevels = levels;
    }

    // ---- showPid / showTid ----
    if (loggerObj.contains("showPid"))
        cfg.showPid = loggerObj.value("showPid").toBool(false);
    if (loggerObj.contains("showTid"))
        cfg.showTid = loggerObj.value("showTid").toBool(false);

    // ---- maxFileSize ----
    if (loggerObj.contains("maxFileSize") && !loggerObj.contains("//")) {
        auto val = loggerObj.value("maxFileSize");
        if (val.isDouble()) {
            auto size = val.toDouble();
            if (size >= 0)
                cfg.maxFileSize = static_cast<uint64_t>(size) * 1024 * 1024;
        }
    }

    return cfg;
}

} // namespace CSEditor
