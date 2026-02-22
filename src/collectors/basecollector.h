// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/collectors/basecollector.h
#pragma once

#include <QString>

/// Abstract base class for all hardware/system data collectors.
class BaseCollector
{
public:
    virtual ~BaseCollector() = default;

    /// Perform a data collection pass (reads kernel interfaces).
    virtual void collect() = 0;

    /// Human-readable name of this collector (e.g. "CPU").
    virtual QString name() const = 0;
};
