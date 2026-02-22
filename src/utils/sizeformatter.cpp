// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/utils/sizeformatter.cpp
#include "sizeformatter.h"

#include <QString>

namespace SizeFormatter {

QString formatBytes(quint64 bytes)
{
    constexpr quint64 KiB = 1024ULL;
    constexpr quint64 MiB = 1024ULL * KiB;
    constexpr quint64 GiB = 1024ULL * MiB;
    constexpr quint64 TiB = 1024ULL * GiB;

    if (bytes >= TiB)
        return QString::number(bytes / double(TiB), 'f', 2) + " TiB";
    if (bytes >= GiB)
        return QString::number(bytes / double(GiB), 'f', 1) + " GiB";
    if (bytes >= MiB)
        return QString::number(bytes / double(MiB), 'f', 1) + " MiB";
    if (bytes >= KiB)
        return QString::number(bytes / double(KiB), 'f', 1) + " KiB";
    return QString::number(bytes) + " B";
}

QString formatHz(quint64 hz)
{
    constexpr quint64 MHz = 1'000'000ULL;
    constexpr quint64 GHz = 1'000'000'000ULL;

    if (hz >= GHz)
        return QString::number(hz / double(GHz), 'f', 2) + " GHz";
    if (hz >= MHz)
        return QString::number(hz / double(MHz), 'f', 0) + " MHz";
    return QString::number(hz) + " Hz";
}

} // namespace SizeFormatter
