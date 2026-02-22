// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/utils/hwutils.h
#pragma once

#include <QString>

namespace HwUtils {

/// Retrieve the full human-readable name of a PCI device using `lspci`.
/// Returns e.g. "Intel Corporation UHD Graphics 620"
///
/// \param pciSlot The slot name, e.g. "0000:00:02.0" or "00:02.0"
QString getPciDeviceName(const QString &pciSlot);

/// Get the human-readable vendor name
QString getPciVendor(const QString &pciSlot);

} // namespace HwUtils
