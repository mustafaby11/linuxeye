// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/collectors/memorycollector.h
#pragma once

#include "basecollector.h"
#include <QString>

class MemoryCollector : public BaseCollector
{
public:
    void collect() override;
    QString name() const override { return QStringLiteral("Memory"); }

    quint64 totalKiB()     const { return m_totalKiB; }
    quint64 availableKiB() const { return m_availableKiB; }
    quint64 usedKiB()      const { return m_usedKiB; }
    quint64 freeKiB()      const { return m_freeKiB; }
    quint64 buffersKiB()   const { return m_buffersKiB; }
    quint64 cachedKiB()    const { return m_cachedKiB; }
    quint64 swapTotalKiB() const { return m_swapTotalKiB; }
    quint64 swapFreeKiB()  const { return m_swapFreeKiB; }
    quint64 swapUsedKiB()  const { return m_swapTotalKiB - m_swapFreeKiB; }

    int usedPercent() const;

private:
    quint64 m_totalKiB     = 0;
    quint64 m_availableKiB = 0;
    quint64 m_usedKiB      = 0;
    quint64 m_freeKiB      = 0;
    quint64 m_buffersKiB   = 0;
    quint64 m_cachedKiB    = 0;
    quint64 m_swapTotalKiB = 0;
    quint64 m_swapFreeKiB  = 0;
};
