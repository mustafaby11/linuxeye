// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/utils/fileutils.h
#pragma once

#include <QString>
#include <QStringList>

namespace FileUtils {

/// Read the entire content of a file. Returns empty string on error.
QString readFile(const QString &path);

/// Read all lines of a file. Returns empty list on error.
QStringList readLines(const QString &path);

/// Read a single trimmed value from a sysfs file.
QString readSysfs(const QString &path);

} // namespace FileUtils
