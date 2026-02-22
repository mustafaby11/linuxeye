// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/collectors/memorycollector.cpp
#include "memorycollector.h"
#include "fileutils.h"

#include <QRegularExpression>

void MemoryCollector::collect()
{
    const QStringList lines = FileUtils::readLines(QStringLiteral("/proc/meminfo"));
    static const QRegularExpression re(QStringLiteral(R"(^(\S+):\s+(\d+))"));

    for (const QString &line : lines) {
        const auto m = re.match(line);
        if (!m.hasMatch()) continue;
        const QString key   = m.captured(1);
        const quint64 value = m.captured(2).toULongLong();

        if      (key == QLatin1String("MemTotal"))     m_totalKiB     = value;
        else if (key == QLatin1String("MemFree"))      m_freeKiB      = value;
        else if (key == QLatin1String("MemAvailable")) m_availableKiB = value;
        else if (key == QLatin1String("Buffers"))      m_buffersKiB   = value;
        else if (key == QLatin1String("Cached"))       m_cachedKiB    = value;
        else if (key == QLatin1String("SwapTotal"))    m_swapTotalKiB = value;
        else if (key == QLatin1String("SwapFree"))     m_swapFreeKiB  = value;
    }

    m_usedKiB = m_totalKiB - m_availableKiB;
}

int MemoryCollector::usedPercent() const
{
    if (m_totalKiB == 0) return 0;
    return static_cast<int>(m_usedKiB * 100ULL / m_totalKiB);
}
