// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/collectors/gpucollector.cpp
#include "gpucollector.h"
#include "fileutils.h"
#include "hwutils.h"

#include <QDir>

void GpuCollector::collect()
{
    m_gpus.clear();
    enumerateDrmDevices();
}

void GpuCollector::enumerateDrmDevices()
{
    QDir drmDir(QStringLiteral("/sys/class/drm"));
    if (!drmDir.exists()) return;

    const QStringList entries = drmDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &entry : entries) {
        // Only top-level cards (card0, card1, …) not connectors
        if (!entry.startsWith(QLatin1String("card")) || entry.contains('-'))
            continue;

        const QString cardPath = QStringLiteral("/sys/class/drm/") + entry;
        GpuInfo gpu;

        // Device path via symlink (e.g., /sys/class/drm/card0/device points to ../../../0000:01:00.0)
        const QString devicePath = cardPath + "/device";
        const QString symLinkTarget = QFileInfo(devicePath).symLinkTarget();
        const QString pciSlot = symLinkTarget.section('/', -1); // e.g., 0000:01:00.0

        // GPU label (vendor + device from uevent) for PCI ID and Driver
        const QString uevent = FileUtils::readSysfs(devicePath + "/uevent");
        for (const QString &line : uevent.split('\n')) {
            if (line.startsWith(QLatin1String("PCI_ID=")))
                gpu.pciId = line.mid(7); // e.g. 1002:687F
            if (line.startsWith(QLatin1String("DRIVER=")))
                gpu.driver = line.mid(7);
        }

        // Use lspci-based HwUtils for beautiful human-readable vendor/device name
        gpu.name = HwUtils::getPciDeviceName(pciSlot);

        if (gpu.name.isEmpty()) {
            // fall back to sysfs vendor/device lookup natively
            const QString vendor = FileUtils::readSysfs(devicePath + "/vendor");
            const QString device = FileUtils::readSysfs(devicePath + "/device");
            if (!vendor.isEmpty())
                gpu.name = QStringLiteral("GPU %1:%2").arg(vendor, device);
            else
                gpu.name = entry;
        }

        // VRAM size (AMD: mem_info_vram_total)
        const QString vramPath = cardPath + "/device/mem_info_vram_total";
        const QString vram = FileUtils::readSysfs(vramPath);
        if (!vram.isEmpty())
            gpu.vramBytes = vram.toULongLong();

        // Temperature (hwmon under device)
        QDir hwmonDir(devicePath + "/hwmon");
        if (hwmonDir.exists()) {
            const auto hwmons = hwmonDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            if (!hwmons.isEmpty()) {
                const QString tempStr = FileUtils::readSysfs(
                    devicePath + "/hwmon/" + hwmons.first() + "/temp1_input");
                if (!tempStr.isEmpty())
                    gpu.tempCelsius = tempStr.toInt() / 1000;
            }
        }

        gpu.renderNode = QStringLiteral("/dev/dri/renderD") + QString::number(128 + m_gpus.size());
        m_gpus.append(gpu);
    }
}
