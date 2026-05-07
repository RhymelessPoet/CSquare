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

    // ---- minLevel ----
    if (loggerObj.contains("minLevel")) {
        auto levelStr = loggerObj.value("minLevel").toString().toLower().toStdString();
        if (levelStr == "trace")
            cfg.minLevel = CS::LogLevels::Trace();
        else if (levelStr == "debug")
            cfg.minLevel = CS::LogLevels::Debug();
        else if (levelStr == "info")
            cfg.minLevel = CS::LogLevels::Info();
        else if (levelStr == "warning")
            cfg.minLevel = CS::LogLevels::Warning();
        else if (levelStr == "error")
            cfg.minLevel = CS::LogLevels::Error();
        else if (levelStr == "fatal")
            cfg.minLevel = CS::LogLevels::Fatal();
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
