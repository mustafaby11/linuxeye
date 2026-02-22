// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/utils/hwutils.cpp
#include "hwutils.h"

#include <QProcess>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logHw, "linuxeye.utils.hw")

namespace HwUtils {

struct PciDev {
    QString vendor;
    QString device;
};

static QHash<QString, PciDev> s_pciCache;
static bool s_pciLoaded = false;
static QMutex s_mutex;

static void loadPciCache()
{
    if (s_pciLoaded)
        return;

    QProcess proc;
    proc.start(QStringLiteral("lspci"), {QStringLiteral("-vmm")});
    proc.waitForFinished(2000);

    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
        qCWarning(logHw) << "Failed to run lspci";
        s_pciLoaded = true;
        return;
    }

    const QString output = QString::fromUtf8(proc.readAllStandardOutput());
    const QStringList blocks = output.split(QStringLiteral("\n\n"), Qt::SkipEmptyParts);

    for (const QString &block : blocks) {
        QString slot;
        QString vendor;
        QString device;

        const QStringList lines = block.split('\n', Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            if (line.startsWith(QLatin1String("Slot:\t"))) {
                slot = line.mid(6);
            } else if (line.startsWith(QLatin1String("Vendor:\t"))) {
                vendor = line.mid(8);
            } else if (line.startsWith(QLatin1String("Device:\t"))) {
                device = line.mid(8);
            }
        }

        if (!slot.isEmpty()) {
            // pad slot "00:02.0" to "0000:00:02.0" pattern
            if (slot.length() == 7 && slot[2] == ':') {
                s_pciCache.insert(QStringLiteral("0000:") + slot, {vendor, device});
            }
            s_pciCache.insert(slot, {vendor, device});
            
            // Allow exact match with lspci output
            // e.g., "00:02.0"
        }
    }

    s_pciLoaded = true;
}

QString getPciDeviceName(const QString &pciSlot)
{
    QMutexLocker locker(&s_mutex);
    loadPciCache();

    // The slot format from sysfs is usually like 0000:06:00.1
    // or sometimes just 06:00.1 if parsed differently.
    // Try find exactly or without 0000:.
    
    QString searchSlot = pciSlot;
    if (searchSlot.startsWith(QLatin1String("0000:"))) {
         // Some tools use 00:00.0 instead of 0000:00:00.0
         // We cached both above.
    }

    const PciDev &dev = s_pciCache.value(searchSlot);
    if (!dev.vendor.isEmpty() && !dev.device.isEmpty()) {
        return dev.vendor + QLatin1Char(' ') + dev.device;
    } else if (!dev.device.isEmpty()) {
        return dev.device;
    }
    return {};
}

QString getPciVendor(const QString &pciSlot)
{
    QMutexLocker locker(&s_mutex);
    loadPciCache();
    return s_pciCache.value(pciSlot).vendor;
}

} // namespace HwUtils
