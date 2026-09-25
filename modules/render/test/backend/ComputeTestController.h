#pragma once
#include <QObject>
#include <QImage>
#include <QMutex>

class ComputeTestController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int currentCase READ currentCase WRITE setCurrentCase NOTIFY currentCaseChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool hasResult READ hasResult NOTIFY resultChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
public:
    explicit ComputeTestController(QObject* parent = nullptr);

    int currentCase() const { return m_currentCase; }
    bool busy() const { return m_busy; }
    bool hasResult() const { return !m_result.isNull(); }
    QString statusText() const { return m_statusText; }

    void setCurrentCase(int value);
    bool takeRunRequest(int& caseIndex);
    void completeRun(QImage image, double cpuMs, double gpuMs, QString error = {});

    Q_INVOKABLE void requestRun();
    Q_INVOKABLE QString saveResult();

signals:
    void currentCaseChanged();
    void busyChanged();
    void resultChanged();
    void statusChanged();
    void runRequested();

private:
    QString settingsPath() const;
    QString caseName() const;
    void loadSettings();
    void saveSettings() const;

    mutable QMutex m_mutex;
    int m_currentCase{0};
    bool m_busy{false};
    bool m_runPending{false};
    QImage m_result;
    QString m_statusText{QStringLiteral("Ready — CPU: -- ms, GPU: -- ms")};
};
