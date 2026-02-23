// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/collectors/oscollector.cpp
#include "oscollector.h"
#include "fileutils.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <sys/utsname.h>
#include <unistd.h>
#include <cstdlib>

void OsCollector::collect()
{
    parseOsRelease();
    parseKernelInfo();
    parseUptime();
    detectInitSystem();

    // Desktop environment
    m_desktopEnv = QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP"));
    if (m_desktopEnv.isEmpty())
        m_desktopEnv = QString::fromLocal8Bit(qgetenv("DESKTOP_SESSION"));

    // Window Manager
    detectWindowManager();

    // Shell
    m_shell = QString::fromLocal8Bit(qgetenv("SHELL"));

    // Distro logo
    findDistroLogo();
}

void OsCollector::parseOsRelease()
{
    const QStringList lines = FileUtils::readLines(QStringLiteral("/etc/os-release"));
    for (const QString &line : lines) {
        const int eq = line.indexOf('=');
        if (eq < 0) continue;
        const QString key   = line.left(eq).trimmed();
        QString value = line.mid(eq + 1).trimmed();
        // Strip surrounding quotes
        if (value.startsWith('"') && value.endsWith('"'))
            value = value.mid(1, value.length() - 2);

        if      (key == QLatin1String("NAME"))          m_distroName    = value;
        else if (key == QLatin1String("VERSION"))       m_distroVersion = value;
        else if (key == QLatin1String("ID"))            m_distroId      = value;
        else if (key == QLatin1String("HOME_URL"))      m_homeUrl       = value;
    }
}

void OsCollector::parseKernelInfo()
{
    struct utsname uts;
    if (uname(&uts) == 0) {
        m_kernelVersion = QString::fromLocal8Bit(uts.release);
        m_kernelArch    = QString::fromLocal8Bit(uts.machine);
        m_hostname      = QString::fromLocal8Bit(uts.nodename);
    }
}

void OsCollector::parseUptime()
{
    const QString content = FileUtils::readFile(QStringLiteral("/proc/uptime"));
    if (content.isEmpty()) return;

    const double totalSeconds = content.split(' ').first().toDouble();
    const int days    = int(totalSeconds) / 86400;
    const int hours   = (int(totalSeconds) % 86400) / 3600;
    const int minutes = (int(totalSeconds) % 3600)  / 60;
    const int secs    = int(totalSeconds) % 60;

    m_uptime = QStringLiteral("%1 days, %2h %3m %4s")
        .arg(days).arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
}

void OsCollector::detectInitSystem()
{
    if (QFile::exists(QStringLiteral("/run/systemd/private")))
        m_initSystem = QStringLiteral("systemd");
    else if (QFile::exists(QStringLiteral("/sbin/openrc")))
        m_initSystem = QStringLiteral("OpenRC");
    else if (QFile::exists(QStringLiteral("/sbin/runit")))
        m_initSystem = QStringLiteral("runit");
    else
        m_initSystem = QStringLiteral("Unknown");
}

void OsCollector::detectWindowManager()
{
    auto findWm = [&]() -> QString {
        // Check specific environment variables first
        if (!qgetenv("HYPRLAND_INSTANCE_SIGNATURE").isEmpty()) return QStringLiteral("Hyprland");
        if (!qgetenv("SWAYSOCK").isEmpty()) return QStringLiteral("Sway");

        const QStringList knownWms = {
            QStringLiteral("kwin_wayland"), QStringLiteral("kwin_x11"), 
            QStringLiteral("mutter"), QStringLiteral("gnome-shell"), 
            QStringLiteral("xfwm4"), QStringLiteral("openbox"), 
            QStringLiteral("i3"), QStringLiteral("sway"), 
            QStringLiteral("awesome"), QStringLiteral("bspwm"), 
            QStringLiteral("xmonad"), QStringLiteral("hyprland"), 
            QStringLiteral("compiz"), QStringLiteral("muffin"), 
            QStringLiteral("marco"), QStringLiteral("cinnamon"), 
            QStringLiteral("enlightenment"), QStringLiteral("fluxbox"), 
            QStringLiteral("pekwm"), QStringLiteral("icewm")
        };

        QDir procDir(QStringLiteral("/proc"));
        const QStringList pids = procDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &pidStr : pids) {
            bool ok;
            const int pid = pidStr.toInt(&ok);
            if (!ok || pid <= 0) continue;

            const QString comm = FileUtils::readSysfs(QStringLiteral("/proc/%1/comm").arg(pid));
            if (knownWms.contains(comm)) {
                if (comm == QLatin1String("kwin_wayland") || comm == QLatin1String("kwin_x11"))
                    return QStringLiteral("KWin");
                if (comm == QLatin1String("gnome-shell"))
                    return QStringLiteral("Mutter");
                if (comm == QLatin1String("xfwm4"))
                    return QStringLiteral("Xfwm4");
                return comm;
            }
        }

        // Fallbacks
        if (m_desktopEnv.contains(QLatin1String("KDE"), Qt::CaseInsensitive))
            return QStringLiteral("KWin");
        if (m_desktopEnv.contains(QLatin1String("GNOME"), Qt::CaseInsensitive))
            return QStringLiteral("Mutter");
        if (m_desktopEnv.contains(QLatin1String("XFCE"), Qt::CaseInsensitive))
            return QStringLiteral("Xfwm4");
            
        return QStringLiteral("Unknown");
    };

    m_windowManager = findWm();

    // Detect session protocol (Wayland vs X11)
    QString sessionType = QString::fromLocal8Bit(qgetenv("XDG_SESSION_TYPE")).toLower();
    if (sessionType.isEmpty()) {
        if (!qgetenv("WAYLAND_DISPLAY").isEmpty())
            sessionType = QStringLiteral("wayland");
        else if (!qgetenv("DISPLAY").isEmpty())
            sessionType = QStringLiteral("x11");
    }

    if (!sessionType.isEmpty()) {
        if (sessionType == QLatin1String("wayland"))
            m_windowManager += QStringLiteral(" (Wayland)");
        else if (sessionType == QLatin1String("x11"))
            m_windowManager += QStringLiteral(" (X11)");
        else {
            // Capitalize first letter of unknown types e.g. "tty" -> "Tty"
            sessionType[0] = sessionType[0].toUpper();
            m_windowManager += QStringLiteral(" (") + sessionType + QStringLiteral(")");
        }
    }
}

void OsCollector::findDistroLogo()
{
    const QString id = m_distroId.toLower();
    if (id.isEmpty()) return;

    // Helper: check if a file exists and set the logo path
    auto tryPath = [this](const QString &path) -> bool {
        if (QFileInfo::exists(path)) {
            m_distroLogoPath = path;
            return true;
        }
        return false;
    };

    // 1) Direct match in /usr/share/icons/{id}.svg or .png
    if (tryPath(QStringLiteral("/usr/share/icons/%1.svg").arg(id))) return;
    if (tryPath(QStringLiteral("/usr/share/icons/%1.png").arg(id))) return;

    // 2) Search hicolor icon theme – prefer larger resolutions
    //    Look for icons named {id}, distributor-logo, or start-here
    const QStringList iconNames = { id, QStringLiteral("distributor-logo"), QStringLiteral("start-here") };
    const QStringList sizes = {
        QStringLiteral("scalable"),
        QStringLiteral("512x512"), QStringLiteral("256x256"),
        QStringLiteral("128x128"), QStringLiteral("96x96"),
        QStringLiteral("64x64"),   QStringLiteral("48x48")
    };
    const QString hicolorBase = QStringLiteral("/usr/share/icons/hicolor");

    for (const QString &iconName : iconNames) {
        for (const QString &sz : sizes) {
            const QString dir = hicolorBase + '/' + sz + QStringLiteral("/apps");
            if (!QDir(dir).exists()) continue;

            // Check SVG first, then PNG
            if (tryPath(dir + '/' + iconName + QStringLiteral(".svg"))) return;
            if (tryPath(dir + '/' + iconName + QStringLiteral(".png"))) return;
        }
    }

    // 3) Search /usr/share/pixmaps for {id}*
    {
        QDir pixmaps(QStringLiteral("/usr/share/pixmaps"));
        if (pixmaps.exists()) {
            const QStringList filters = {
                id + QStringLiteral("*.svg"),
                id + QStringLiteral("*.png"),
                id + QStringLiteral("-logo*.svg"),
                id + QStringLiteral("-logo*.png"),
            };
            const QStringList entries = pixmaps.entryList(filters, QDir::Files, QDir::Name);
            if (!entries.isEmpty()) {
                m_distroLogoPath = pixmaps.absoluteFilePath(entries.first());
                return;
            }
        }
    }

    // 4) Fallback for Arch-based distros (CachyOS, EndeavourOS, etc.)
    const QStringList archBased = {
        QStringLiteral("cachyos"), QStringLiteral("endeavouros"),
        QStringLiteral("manjaro"), QStringLiteral("garuda"),
        QStringLiteral("arcolinux"), QStringLiteral("artix")
    };
    if (archBased.contains(id)) {
        if (tryPath(QStringLiteral("/usr/share/pixmaps/archlinux-logo.svg"))) return;
        if (tryPath(QStringLiteral("/usr/share/pixmaps/archlinux-logo.png"))) return;
    }
}
