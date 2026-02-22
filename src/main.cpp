// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/main.cpp
#include "application.h"
#include "mainwindow.h"
#include "systeminfo.h"

#include <QLoggingCategory>
#include <QDateTime>
#include <QEventLoop>

#include <iostream>

void coloredLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString color;
    QString level;
    switch (type) {
    case QtDebugMsg:    color = QStringLiteral("\033[36m"); level = QStringLiteral("DEBUG"); break;
    case QtInfoMsg:     color = QStringLiteral("\033[32m"); level = QStringLiteral("INFO "); break;
    case QtWarningMsg:  color = QStringLiteral("\033[33m"); level = QStringLiteral("WARN "); break;
    case QtCriticalMsg: color = QStringLiteral("\033[31m"); level = QStringLiteral("CRIT "); break;
    case QtFatalMsg:    color = QStringLiteral("\033[1;31m"); level = QStringLiteral("FATAL"); break;
    }
    
    QString category = context.category ? QString::fromLatin1(context.category) : QStringLiteral("default");
    QString timeStr = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz"));
    
    fprintf(stderr, "\033[90m%s\033[0m [%s\033[0m] [\033[1;34m%s\033[0m] %s\n", 
            qPrintable(timeStr), 
            qPrintable(color + level), 
            qPrintable(category), 
            qPrintable(msg));
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(coloredLogHandler);

    Application app(argc, argv);

    // ── 1. Privileged collection via Polkit FIRST ─────────────────────
    // QEventLoop keeps D-Bus alive so polkit-agent can show its dialog.
    // Main window is not created yet — user only sees the auth prompt.
    {
        QEventLoop authLoop;
        QObject::connect(
            SystemInfo::instance(), &SystemInfo::privilegedDataReady,
            &authLoop, &QEventLoop::quit);

        SystemInfo::instance()->collectPrivileged();
        authLoop.exec();

        bool authOk = SystemInfo::instance()->privileged()->data().isValid;

        if (!authOk) {
            // Auth failed or cancelled — exit immediately completely
            qCWarning(QLoggingCategory("linuxeye.main")) << "Authentication cancelled or failed. Exiting.";
            return 1;
        }
    }

    // ── 2. Collect unprivileged data AFTER auth ─────────────────────
    SystemInfo::instance()->collectAll();

    // ── 3. Create and show main window ──────────────────────────────
    MainWindow w;
    w.show();

    return app.exec();
}

