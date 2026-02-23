// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/collectors/oscollector.h
#pragma once

#include "basecollector.h"
#include <QString>
#include <QStringList>

class OsCollector : public BaseCollector
{
public:
    void collect() override;
    QString name() const override { return QStringLiteral("OS / Kernel"); }

    QString distroName()    const { return m_distroName; }
    QString distroVersion() const { return m_distroVersion; }
    QString distroId()      const { return m_distroId; }
    QString distroPrettyName() const { return m_distroPrettyName; }
    QString distroAnsiColor()  const { return m_distroAnsiColor; }
    QString bugReportUrl()     const { return m_bugReportUrl; }
    QString distroLogoPath()   const { return m_distroLogoPath; }

    QString kernelVersion() const { return m_kernelVersion; }
    QString kernelArch()    const { return m_kernelArch; }
    QString hostname()      const { return m_hostname; }
    QString uptime()        const { return m_uptime; }
    QString initSystem()    const { return m_initSystem; }
    QString desktopEnv()    const { return m_desktopEnv; }
    QString windowManager() const { return m_windowManager; }
    QString shell()         const { return m_shell; }

private:
    QString m_distroName;
    QString m_distroVersion;
    QString m_distroId;
    QString m_distroPrettyName;
    QString m_distroAnsiColor;
    QString m_bugReportUrl;
    QString m_distroLogoPath;

    QString m_kernelVersion;
    QString m_kernelArch;
    QString m_hostname;
    QString m_uptime;
    QString m_initSystem;
    QString m_desktopEnv;
    QString m_windowManager;
    QString m_shell;

    void parseOsRelease();
    void parseKernelInfo();
    void parseUptime();
    void detectInitSystem();
    void detectWindowManager();
    void findDistroLogo();
};
