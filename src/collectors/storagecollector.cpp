// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/collectors/storagecollector.cpp
#include "storagecollector.h"
#include "fileutils.h"

#include <QDir>

#include <sys/statvfs.h>
#include <QMap>
#include <QPair>

void StorageCollector::collect()
{
    m_disks.clear();
    enumerateBlockDevices();
}

void StorageCollector::enumerateBlockDevices()
{
    // Parse /proc/mounts for mount point and filesystem info
    QMap<QString, QPair<QString, QString>> mountMap; // devName -> (mountpoint, fsType)
    const QStringList mountLines = FileUtils::readLines(QStringLiteral("/proc/mounts"));
    for (const QString &line : mountLines) {
        const QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (parts.size() >= 3 && parts[0].startsWith(QLatin1String("/dev/"))) {
            QString devName = parts[0].mid(5); // remove /dev/
            mountMap[devName] = qMakePair(parts[1], parts[2]);
        }
    }

    QDir blockDir(QStringLiteral("/sys/block"));
    const QStringList entries = blockDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // Filter out logical/virtual devices unless needed, but typical physical are not loop/ram/zram
    for (const QString &dev : entries) {
        if (dev.startsWith(QLatin1String("loop")) ||
            dev.startsWith(QLatin1String("ram"))  ||
            dev.startsWith(QLatin1String("zram")))
            continue;

        const QString devPath = QStringLiteral("/sys/block/") + dev;
        DiskInfo disk;
        disk.name = dev;

        // Model
        disk.model = FileUtils::readSysfs(devPath + "/device/model");
        if (disk.model.isEmpty())
            disk.model = FileUtils::readSysfs(devPath + "/device/name");

        // Size in 512-byte sectors
        const QString sectors = FileUtils::readSysfs(devPath + "/size");
        disk.sizeBytes = sectors.toULongLong() * 512ULL;

        // Type detection
        disk.type = detectDiskType(dev);

        // Find partitions (they are subdirectories in the block device dir that have a 'partition' file inside)
        QDir devDir(devPath);
        const QStringList subDirs = devDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &subDir : subDirs) {
            if (QFile::exists(devPath + "/" + subDir + "/partition")) {
                PartitionInfo part;
                part.name = subDir;

                const QString pSectors = FileUtils::readSysfs(devPath + "/" + subDir + "/size");
                part.sizeBytes = pSectors.toULongLong() * 512ULL;

                if (mountMap.contains(subDir)) {
                    part.mountPoint = mountMap[subDir].first;
                    part.filesystem = mountMap[subDir].second;

                    // Get used/total space via statvfs
                    struct statvfs st;
                    if (statvfs(part.mountPoint.toLocal8Bit().constData(), &st) == 0) {
                        part.fsTotalBytes = static_cast<quint64>(st.f_blocks) * st.f_frsize;
                        part.fsUsedBytes  = static_cast<quint64>(st.f_blocks - st.f_bfree) * st.f_frsize;
                    }
                }
                disk.partitions.append(part);
            }
        }

        m_disks.append(disk);
    }
}

QString StorageCollector::detectDiskType(const QString &devName)
{
    // NVMe
    if (devName.startsWith(QLatin1String("nvme")))
        return QStringLiteral("NVMe");

    // Check rotational flag
    const QString rotPath = QStringLiteral("/sys/block/") + devName + QStringLiteral("/queue/rotational");
    const QString rotVal  = FileUtils::readSysfs(rotPath);
    if (rotVal == "0")
        return QStringLiteral("SSD");
    if (rotVal == "1")
        return QStringLiteral("HDD");

    return QStringLiteral("Unknown");
}
