// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/collectors/cpucollector.cpp
#include "cpucollector.h"
#include "fileutils.h"

#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <sys/utsname.h>

void CpuCollector::collect()
{
    parseFromProcCpuinfo();
    parseCacheInfo();
    parseFrequency();
    parseArchitecture();
    detectVirtualization();
}

void CpuCollector::parseFromProcCpuinfo()
{
    const QStringList lines = FileUtils::readLines(QStringLiteral("/proc/cpuinfo"));
    QSet<int> physicalIds;
    int logicalCount = 0;

    for (const QString &line : lines) {
        const int colon = line.indexOf(':');
        if (colon < 0) continue;
        const QString key   = line.left(colon).trimmed();
        const QString value = line.mid(colon + 1).trimmed();

        if (key == QLatin1String("model name") && m_modelName.isEmpty())
            m_modelName = value;
        else if (key == QLatin1String("vendor_id") && m_vendor.isEmpty())
            m_vendor = value;
        else if (key == QLatin1String("physical id"))
            physicalIds.insert(value.toInt());
        else if (key == QLatin1String("processor"))
            logicalCount = value.toInt() + 1;
        else if (key == QLatin1String("flags") && m_flags.isEmpty())
            m_flags = value.split(' ', Qt::SkipEmptyParts);
    }

    m_logicalCores  = logicalCount;
    m_physicalCores = physicalIds.isEmpty() ? logicalCount : physicalIds.size();
}

void CpuCollector::parseCacheInfo()
{
    const QString base = QStringLiteral("/sys/devices/system/cpu/cpu0/cache");
    QDir cacheDir(base);
    if (!cacheDir.exists()) return;

    const QStringList indices = cacheDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &idx : indices) {
        const QString idxPath = base + "/" + idx;
        const QString level = FileUtils::readSysfs(idxPath + "/level");
        const QString type  = FileUtils::readSysfs(idxPath + "/type");
        const QString size  = FileUtils::readSysfs(idxPath + "/size");

        if (level == "1" && type == "Data")        m_cacheL1d = size;
        else if (level == "1" && type == "Instruction") m_cacheL1i = size;
        else if (level == "2")                     m_cacheL2  = size;
        else if (level == "3")                     m_cacheL3  = size;
    }
}

void CpuCollector::parseFrequency()
{
    // Max frequency from cpufreq
    QString maxFreq = FileUtils::readSysfs(
        QStringLiteral("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq"));
    if (!maxFreq.isEmpty())
        m_maxFreqMHz = maxFreq.toDouble() / 1000.0;

    // Current scaling frequency
    QString curFreq = FileUtils::readSysfs(
        QStringLiteral("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"));
    if (!curFreq.isEmpty())
        m_curFreqMHz = curFreq.toDouble() / 1000.0;

    // Fallback: parse "cpu MHz" from /proc/cpuinfo
    if (m_curFreqMHz == 0.0) {
        const QStringList lines = FileUtils::readLines(QStringLiteral("/proc/cpuinfo"));
        for (const QString &line : lines) {
            if (line.startsWith(QLatin1String("cpu MHz"))) {
                const int colon = line.indexOf(':');
                if (colon >= 0)
                    m_curFreqMHz = line.mid(colon + 1).trimmed().toDouble();
                break;
            }
        }
    }
}

void CpuCollector::parseArchitecture()
{
    struct utsname uts;
    if (uname(&uts) == 0)
        m_architecture = QString::fromLocal8Bit(uts.machine);
}

void CpuCollector::detectVirtualization()
{
    // Detect from CPU flags: vmx = Intel VT-x, svm = AMD-V
    const bool hasVmx = m_flags.contains(QStringLiteral("vmx"));
    const bool hasSvm = m_flags.contains(QStringLiteral("svm"));

    QString tech;
    if (hasVmx)
        tech = QStringLiteral("VT-x (Intel)");
    else if (hasSvm)
        tech = QStringLiteral("AMD-V (AMD)");

    if (tech.isEmpty()) {
        m_virtualization = QStringLiteral("Not supported");
        return;
    }

    // Check if KVM is available (/dev/kvm exists)
    const bool kvmAvailable = QFile::exists(QStringLiteral("/dev/kvm"));

    // Check if KVM module is loaded
    const QString modules = FileUtils::readFile(QStringLiteral("/proc/modules"));
    const bool kvmLoaded = modules.contains(QLatin1String("kvm_intel")) ||
                           modules.contains(QLatin1String("kvm_amd"));

    if (kvmAvailable && kvmLoaded)
        m_virtualization = tech + QStringLiteral(" (Enabled, KVM active)");
    else if (kvmAvailable)
        m_virtualization = tech + QStringLiteral(" (Enabled, KVM available)");
    else
        m_virtualization = tech + QStringLiteral(" (Supported, KVM not loaded)");
}
