// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/utils/fileutils.cpp
#include "fileutils.h"

#include <QFile>
#include <QTextStream>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logFileUtils, "linuxeye.utils.file")

namespace FileUtils {

QString readFile(const QString &path)
{
    QFile f(path);
    if (!f.exists()) {
        return {}; // Silently return empty if not found; typical for sysfs discovery
    }
    
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Only warn if the file is expected to be readable but fails (e.g., EACCES)
        qCDebug(logFileUtils) << "Cannot open existing file:" << path;
        return {};
    }
    return QTextStream(&f).readAll();
}

QStringList readLines(const QString &path)
{
    const QString content = readFile(path);
    if (content.isEmpty())
        return {};
    return content.split('\n', Qt::SkipEmptyParts);
}

QString readSysfs(const QString &path)
{
    return readFile(path).trimmed();
}

} // namespace FileUtils
