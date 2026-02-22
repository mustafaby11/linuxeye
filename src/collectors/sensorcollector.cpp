// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/collectors/sensorcollector.cpp
#include "sensorcollector.h"
#include "../utils/fileutils.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>

void SensorCollector::collect()
{
    m_readings.clear();
    enumerateHwmon();
}

void SensorCollector::enumerateHwmon()
{
    QDir hwmonDir(QStringLiteral("/sys/class/hwmon"));
    if (!hwmonDir.exists())
        return;

    const auto hwmons = hwmonDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const auto &hwmon : hwmons) {
        readChip(hwmonDir.absoluteFilePath(hwmon));
    }
}

void SensorCollector::readChip(const QString &hwmonPath)
{
    QDir dir(hwmonPath);
    QString chipName = FileUtils::readSysfs(dir.absoluteFilePath(QStringLiteral("name")));
    if (chipName.isEmpty())
        chipName = dir.dirName();

    const auto files = dir.entryList(QDir::Files);
    for (const auto &file : files) {
        if (file.endsWith(QLatin1String("_input"))) {
            QString baseName = file.left(file.length() - 6);
            QString label = FileUtils::readSysfs(dir.absoluteFilePath(baseName + QLatin1String("_label")));
            if (label.isEmpty())
                label = baseName;

            QString valStr = FileUtils::readSysfs(dir.absoluteFilePath(file));
            if (valStr.isEmpty())
                continue;

            bool ok;
            double val = valStr.toDouble(&ok);
            if (!ok) continue;

            SensorReading r;
            r.chip = chipName;
            r.label = label;
            
            if (file.startsWith(QLatin1String("temp"))) {
                r.type = QStringLiteral("temp");
                r.value = val / 1000.0;
                r.unit = QStringLiteral("°C");
            } else if (file.startsWith(QLatin1String("fan"))) {
                r.type = QStringLiteral("fan");
                r.value = val;
                r.unit = QStringLiteral("RPM");
            } else if (file.startsWith(QLatin1String("in"))) {
                r.type = QStringLiteral("voltage");
                r.value = val / 1000.0;
                r.unit = QStringLiteral("V");
            } else if (file.startsWith(QLatin1String("power"))) {
                r.type = QStringLiteral("power");
                r.value = val / 1000000.0;
                r.unit = QStringLiteral("W");
            } else if (file.startsWith(QLatin1String("curr"))) {
                r.type = QStringLiteral("current");
                r.value = val / 1000.0;
                r.unit = QStringLiteral("A");
            } else {
                r.type = QStringLiteral("unknown");
                r.value = val;
            }

            m_readings.append(r);
        }
    }
}
