// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/collectors/boardcollector.cpp
#include "boardcollector.h"
#include "fileutils.h"

static const QString kDmiBase = QStringLiteral("/sys/class/dmi/id/");

void BoardCollector::collect()
{
    readDmiAttribute(QStringLiteral("board_vendor"),  m_boardVendor);
    readDmiAttribute(QStringLiteral("board_name"),    m_boardName);
    readDmiAttribute(QStringLiteral("board_version"), m_boardVersion);
    readDmiAttribute(QStringLiteral("bios_vendor"),   m_biosVendor);
    readDmiAttribute(QStringLiteral("bios_version"),  m_biosVersion);
    readDmiAttribute(QStringLiteral("bios_date"),     m_biosDate);
    readDmiAttribute(QStringLiteral("bios_release"),  m_biosReleaseDate);
    readDmiAttribute(QStringLiteral("chassis_type"),  m_chassisType);
    readDmiAttribute(QStringLiteral("sys_vendor"),    m_sysVendor);
    readDmiAttribute(QStringLiteral("product_name"),  m_productName);
}

void BoardCollector::readDmiAttribute(const QString &attr, QString &out)
{
    out = FileUtils::readSysfs(kDmiBase + attr);
}
