#include "ComputeTestController.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QSaveFile>

ComputeTestController::ComputeTestController(QObject* parent) : QObject(parent)
{
    loadSettings();
}

QString ComputeTestController::settingsPath() const
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("compute_test.json"));
}

QString ComputeTestController::caseName() const
{
    return m_currentCase == 0 ? QStringLiteral("edge_preserving_filter") : QStringLiteral("laplacian_smoothing");
}

void ComputeTestController::loadSettings()
{
    QFile file(settingsPath());
    if (file.open(QIODevice::ReadOnly)) {
        m_currentCase = qBound(0, QJsonDocument::fromJson(file.readAll()).object().value("currentCase").toInt(), 1);
    }
}

void ComputeTestController::saveSettings() const
{
    QSaveFile file(settingsPath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(QJsonObject{{"currentCase", m_currentCase}}).toJson(QJsonDocument::Indented));
        file.commit();
    }
}

void ComputeTestController::setCurrentCase(int value)
{
    value = qBound(0, value, 1);
    if (m_currentCase == value) {
        return;
    }
    m_currentCase = value;
    saveSettings();
    emit currentCaseChanged();
    requestRun();
}

void ComputeTestController::requestRun()
{
    {
        QMutexLocker lock(&m_mutex);
        m_runPending = true;
    }
    if (!m_busy) {
        m_busy = true;
        emit busyChanged();
    }
    m_statusText = QStringLiteral("Running %1…").arg(caseName());
    emit statusChanged();
    emit runRequested();
}

bool ComputeTestController::takeRunRequest(int& caseIndex)
{
    QMutexLocker lock(&m_mutex);
    // if (!m_runPending)
    //     return false;
    m_runPending = false;
    caseIndex = m_currentCase;
    return true;
}

void ComputeTestController::completeRun(QImage image, double cpuMs, double gpuMs, QString error)
{
    QMetaObject::invokeMethod(
        this,
        [this, image = std::move(image), cpuMs, gpuMs, error = std::move(error)]() mutable {
            m_result = std::move(image);
            m_busy = false;

            if (error.isEmpty()) {
                m_statusText =
                    QStringLiteral("Complete — CPU: %1 ms, GPU: %2 ms").arg(cpuMs, 0, 'f', 2).arg(gpuMs, 0, 'f', 2);
            } else {
                m_statusText = QStringLiteral("Error — %1").arg(error);
            }
            // qInfo().noquote() << m_statusText << "result:" << m_result.size();
            emit busyChanged();
            emit resultChanged();
            emit statusChanged();
        },
        Qt::QueuedConnection);
}

QString ComputeTestController::saveResult()
{
    if (m_result.isNull())
        return QString();
    QDir dir(QCoreApplication::applicationDirPath());
    dir.mkpath(QStringLiteral("results"));
    const QString name =
        QStringLiteral("%1_%2.png").arg(caseName(), QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz"));
    const QString path = dir.filePath(QStringLiteral("results/") + name);
    if (!m_result.save(path, "PNG"))
        return QString();
    m_statusText = QStringLiteral("Saved — %1").arg(QDir::toNativeSeparators(path));
    emit statusChanged();
    return path;
}
