// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/collectors/storagecollector.h
#pragma once

#include "basecollector.h"
#include <QString>
#include <QList>

struct PartitionInfo {
    QString name;       // e.g. "sda1"
    quint64 sizeBytes = 0;
    QString mountPoint; // e.g. "/", or empty
    QString filesystem; // e.g. "ext4", "btrfs", "vfat"
    quint64 fsTotalBytes = 0;
    quint64 fsUsedBytes = 0;
};

struct DiskInfo {
    QString name;       // e.g. "sda"
    QString model;
    quint64 sizeBytes = 0;
    QString type;       // "HDD", "SSD", "NVMe"
    QList<PartitionInfo> partitions;
};

class StorageCollector : public BaseCollector
{
public:
    void collect() override;
    QString name() const override { return QStringLiteral("Storage"); }

    const QList<DiskInfo>& disks() const { return m_disks; }

private:
    QList<DiskInfo> m_disks;

    void enumerateBlockDevices();
    QString detectDiskType(const QString &devName);
};
