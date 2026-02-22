// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/collectors/networkcollector.cpp
#include "networkcollector.h"
#include "fileutils.h"
#include "hwutils.h"

#include <QDir>
#include <QRegularExpression>

void NetworkCollector::collect()
{
    m_interfaces.clear();
    parseNetDev();
}

void NetworkCollector::parseNetDev()
{
    const QStringList lines = FileUtils::readLines(QStringLiteral("/proc/net/dev"));
    // Skip 2 header lines
    for (int i = 2; i < lines.size(); ++i) {
        const QString &line = lines.at(i);
        const int colon = line.indexOf(':');
        if (colon < 0) continue;

        const QString ifaceName = line.left(colon).trimmed();
        if (ifaceName == QLatin1String("lo")) continue; // skip loopback

        const QStringList parts = line.mid(colon + 1).trimmed().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        // /proc/net/dev format: rx_bytes rx_pkts rx_errs ... tx_bytes ...
        NetworkInterface iface;
        iface.name = ifaceName;
        if (parts.size() >= 9) {
            iface.rxBytes = parts.at(0).toULongLong();
            iface.txBytes = parts.at(8).toULongLong();
        }

        parseIfaceDetails(iface);
        m_interfaces.append(iface);
    }
}

void NetworkCollector::parseIfaceDetails(NetworkInterface &iface)
{
    const QString basePath = QStringLiteral("/sys/class/net/") + iface.name;

    // MAC address
    iface.macAddress = FileUtils::readSysfs(basePath + "/address");

    // Up/down state
    const QString operstate = FileUtils::readSysfs(basePath + "/operstate");
    iface.isUp = (operstate == QLatin1String("up"));

    // Speed (Mbps, may fail for virtual interfaces)
    const QString speed = FileUtils::readSysfs(basePath + "/speed");
    if (!speed.isEmpty()) {
        bool ok = false;
        const int s = speed.toInt(&ok);
        if (ok && s > 0) iface.speedMbps = static_cast<quint64>(s);
    }

    // Driver via symlink uevent
    const QString driverLink = basePath + "/device/driver";
    QFileInfo fi(driverLink);
    if (fi.isSymLink())
        iface.driver = fi.symLinkTarget().section('/', -1);

    // Hardware name via lspci
    const QString deviceLink = basePath + "/device";
    QFileInfo dfi(deviceLink);
    if (dfi.isSymLink()) {
        const QString pciSlot = dfi.symLinkTarget().section('/', -1);
        iface.hwName = HwUtils::getPciDeviceName(pciSlot);
    }

    // IPv4 from /proc/net/fib_trie is complex; use ioctl instead:
    // For simplicity, read from /proc/net/if_inet6 for IPv6
    // and skip raw IPv4 (needs ioctl – will add in Phase 2)
}
