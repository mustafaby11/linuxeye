// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/collectors/sensorcollector.h
#pragma once

#include "basecollector.h"
#include <QString>
#include <QList>

struct SensorReading {
    QString chip;       // e.g. "k10temp-pci-00c3"
    QString label;      // e.g. "Tdie"
    QString type;       // "temp", "fan", "voltage"
    double  value = 0.0;
    QString unit;       // "°C", "RPM", "V"
};

class SensorCollector : public BaseCollector
{
public:
    void collect() override;
    QString name() const override { return QStringLiteral("Sensors"); }

    const QList<SensorReading>& readings() const { return m_readings; }

private:
    QList<SensorReading> m_readings;

    void enumerateHwmon();
    void readChip(const QString &hwmonPath);
};
