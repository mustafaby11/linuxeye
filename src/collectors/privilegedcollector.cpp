// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/collectors/privilegedcollector.cpp
#include "privilegedcollector.h"

#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logPriv, "linuxeye.privileged")

PrivilegedCollector::PrivilegedCollector(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
{
    connect(m_process,
            qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, &PrivilegedCollector::onProcessFinished);
}

void PrivilegedCollector::start()
{
    if (m_process->state() != QProcess::NotRunning) {
        qCWarning(logPriv) << "Privileged collection already running";
        return;
    }

    m_data = {};

    const QString helper = helperPath();
    if (helper.isEmpty()) {
        qCWarning(logPriv) << "linuxeye-helper not found";
        emit finished(false);
        return;
    }

    qCInfo(logPriv) << "Launching:" << "pkexec" << helper;
    m_process->start(QStringLiteral("pkexec"), {helper});
}

bool PrivilegedCollector::startSync()
{
    m_data = {};

    const QString helper = helperPath();
    if (helper.isEmpty()) {
        qCWarning(logPriv) << "linuxeye-helper not found";
        return false;
    }

    QProcess proc;
    proc.start(QStringLiteral("pkexec"), {helper});
    proc.waitForFinished(300000);   // blocks up to 5 minutes waiting for auth

    if (proc.state() == QProcess::Running) {
        qCWarning(logPriv) << "pkexec timed out";
        proc.kill();
        return false;
    }

    if (proc.exitCode() != 0) {
        qCWarning(logPriv) << "pkexec exited with code" << proc.exitCode() << "(user cancelled?)";
        return false;
    }

    const QByteArray output = proc.readAllStandardOutput();
    const QJsonDocument doc = QJsonDocument::fromJson(output);
    if (doc.isNull() || !doc.isObject()) {
        qCWarning(logPriv) << "Failed to parse helper output";
        return false;
    }

    const QJsonObject obj = doc.object();
    m_data.boardSerial   = obj.value(QLatin1String("board_serial")).toString();
    m_data.chassisSerial = obj.value(QLatin1String("chassis_serial")).toString();
    m_data.productSerial = obj.value(QLatin1String("product_serial")).toString();
    m_data.productUuid   = obj.value(QLatin1String("product_uuid")).toString();
    m_data.isValid       = true;

    qCInfo(logPriv) << "Privileged data (sync) collected successfully";
    emit finished(true);
    return true;
}

void PrivilegedCollector::onProcessFinished(int exitCode, int /*exitStatus*/)
{
    if (exitCode != 0) {
        qCWarning(logPriv) << "pkexec exited with code" << exitCode
                           << "(user may have cancelled auth)";
        emit finished(false);
        return;
    }

    const QByteArray output = m_process->readAllStandardOutput();
    const QJsonDocument doc = QJsonDocument::fromJson(output);
    if (doc.isNull() || !doc.isObject()) {
        qCWarning(logPriv) << "Failed to parse helper output:" << output;
        emit finished(false);
        return;
    }

    const QJsonObject obj = doc.object();
    m_data.boardSerial   = obj.value(QLatin1String("board_serial")).toString();
    m_data.chassisSerial = obj.value(QLatin1String("chassis_serial")).toString();
    m_data.productSerial = obj.value(QLatin1String("product_serial")).toString();
    m_data.productUuid   = obj.value(QLatin1String("product_uuid")).toString();
    m_data.isValid       = true;

    qCInfo(logPriv) << "Privileged data collected successfully";
    emit finished(true);
}

QString PrivilegedCollector::helperPath()
{
    const QString appDir = QCoreApplication::applicationDirPath();

    // Search order:
    // 1. Same directory as the linuxeye binary (installed layout)
    // 2. helper/ subdirectory (build-tree layout: build/src/ + build/src/helper/)
    // 3. Standard install locations
    const QStringList candidates = {
        appDir + QStringLiteral("/linuxeye-helper"),
        appDir + QStringLiteral("/helper/linuxeye-helper"),
        QStringLiteral("/usr/local/bin/linuxeye-helper"),
        QStringLiteral("/usr/bin/linuxeye-helper"),
    };

    for (const QString &p : candidates) {
        if (QFileInfo::exists(p))
            return p;
    }

    return {};
}

#include "moc_privilegedcollector.cpp"
