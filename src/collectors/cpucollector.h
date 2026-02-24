// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/collectors/cpucollector.h
#pragma once

#include "basecollector.h"
#include <QString>
#include <QStringList>

class CpuCollector : public BaseCollector
{
public:
    void collect() override;
    QString name() const override { return QStringLiteral("CPU"); }

    // Getters
    QString modelName()    const { return m_modelName; }
    QString vendor()       const { return m_vendor; }
    int     physicalCores()const { return m_physicalCores; }
    int     logicalCores() const { return m_logicalCores; }
    double  maxFreqMHz()   const { return m_maxFreqMHz; }
    double  curFreqMHz()   const { return m_curFreqMHz; }
    QString architecture() const { return m_architecture; }
    QString cacheL1d()     const { return m_cacheL1d; }
    QString cacheL1i()     const { return m_cacheL1i; }
    QString cacheL2()      const { return m_cacheL2; }
    QString cacheL3()      const { return m_cacheL3; }
    QStringList flags()    const { return m_flags; }
    QString virtualization() const { return m_virtualization; }

private:
    QString     m_modelName;
    QString     m_vendor;
    int         m_physicalCores  = 0;
    int         m_logicalCores   = 0;
    double      m_maxFreqMHz     = 0.0;
    double      m_curFreqMHz     = 0.0;
    QString     m_architecture;
    QString     m_cacheL1d;
    QString     m_cacheL1i;
    QString     m_cacheL2;
    QString     m_cacheL3;
    QStringList m_flags;
    QString     m_virtualization;

    void parseFromProcCpuinfo();
    void parseCacheInfo();
    void parseFrequency();
    void parseArchitecture();
    void detectVirtualization();
};
