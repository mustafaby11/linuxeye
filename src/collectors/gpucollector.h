// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/collectors/gpucollector.h
#pragma once

#include "basecollector.h"
#include <QString>
#include <QList>

struct GpuInfo {
    QString name;
    QString driver;
    quint64 vramBytes  = 0;
    int     tempCelsius = -1;
    QString pciId;
    QString renderNode; // e.g. /dev/dri/renderD128
};

class GpuCollector : public BaseCollector
{
public:
    void collect() override;
    QString name() const override { return QStringLiteral("GPU"); }

    const QList<GpuInfo>& gpus() const { return m_gpus; }

private:
    QList<GpuInfo> m_gpus;

    void enumerateDrmDevices();
};
