// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/ui/style_helper.h
#pragma once

#include <QString>
#include <QApplication>
#include <QStyle>

namespace StyleHelper {

inline QString panelTitle() {
    return QStringLiteral("font-weight: bold; font-size: 16px; margin-bottom: 6px;");
}

inline QString summaryText() {
    return QStringLiteral("font-size: 13px; margin-bottom: 4px;");
}

inline QString card() {
    // Use system palette for group box styling
    return QString();
}

inline QString formKey() {
    return QStringLiteral("font-weight: bold;");
}

inline QString valueText() {
    return QString();
}

inline QString valueHighlight() {
    return QStringLiteral("font-weight: bold;");
}

inline QString valueMono() {
    return QStringLiteral("font-family: monospace;");
}

inline QString tree() {
    return QString();
}

inline QString progressBar() {
    return QString();
}

}
