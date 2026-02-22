// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/utils/sizeformatter.h
#pragma once

#include <QString>
#include <QtTypes>

namespace SizeFormatter {

/// Format bytes as human-readable string (e.g. "15.6 GiB")
QString formatBytes(quint64 bytes);

/// Format Hz as human-readable string (e.g. "3.60 GHz")
QString formatHz(quint64 hz);

} // namespace SizeFormatter
