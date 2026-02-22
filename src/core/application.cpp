// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/core/application.cpp
#include "application.h"

#include <QStyleFactory>
#include <QStyle>
#include <QIcon>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logApp, "linuxeye.app")

#include <QStyleHints>

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
{
    setApplicationName(QStringLiteral("linuxeye"));
    setApplicationVersion(QStringLiteral("0.1.0"));
    setOrganizationName(QStringLiteral("linuxeye"));

    applyStyle();
}

void Application::applyStyle()
{
    // Use system theme. If system theme is not available, fall back to Fusion.
    // Qt automatically uses the system theme on Linux (Breeze, GNOME, etc.)
    // We don't override the style - let the system decide.
    
    // Set Fusion as fallback style (used when no system theme is available)
    if (style() == nullptr || QString::compare(style()->objectName(), "fusion", Qt::CaseInsensitive) == 0) {
        setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    }
    
    // Use system icon theme with hicolor as fallback
    QIcon::setFallbackThemeName(QStringLiteral("hicolor"));
}
