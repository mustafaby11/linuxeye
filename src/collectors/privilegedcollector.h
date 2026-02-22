// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/collectors/privilegedcollector.h
#pragma once

#include <QObject>
#include <QString>

/// Data returned by the privileged helper (linuxeye-helper via pkexec).
struct PrivilegedData {
    QString boardSerial;
    QString chassisSerial;
    QString productSerial;
    QString productUuid;

    bool isValid = false; ///< true after successful privileged collection
};

class QProcess;

/// Runs linuxeye-helper via pkexec and parses the resulting JSON.
/// Signals are emitted asynchronously to avoid blocking the UI.
class PrivilegedCollector : public QObject
{
    Q_OBJECT
public:
    explicit PrivilegedCollector(QObject *parent = nullptr);

    /// Start an async privileged collection. Emits finished() when done.
    void start();

    /// Blocking version – runs pkexec and waits. Use before UI is shown.
    /// Returns true on success.
    bool startSync();

    const PrivilegedData &data() const { return m_data; }

signals:
    void finished(bool success);

private slots:
    void onProcessFinished(int exitCode, int exitStatus);

private:
    QProcess      *m_process = nullptr;
    PrivilegedData m_data;

    static QString helperPath();
};
