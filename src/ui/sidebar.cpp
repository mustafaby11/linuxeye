// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/ui/sidebar.cpp
#include "sidebar.h"

#include <QIcon>

struct NavItem {
    const char *primary;   // preferred XDG icon name
    const char *fallback;  // secondary icon name
    const char *label;
};

// Icon names tested against Breeze, Adwaita, hicolor.
// Each entry has a primary + fallback so something always shows.
static const NavItem kNavItems[] = {
    { "computer",                 "system-run",                 "Overview"    },
    { "cpu",                      "preferences-system",         "CPU"         },
    { "memory",                   "drive-harddisk",             "Memory"      },
    { "distributor-logo",         "start-here",                 "OS / Kernel" },
    { "drive-harddisk",           "drive-harddisk-usb",         "Storage"     },
    { "network-wired",            "network-wired-symbolic",     "Network"     },
    { "video-display",            "display",                    "GPU"         },
    { "computer",                 "preferences-desktop",        "Motherboard" },
    { "temperature-symbolic",     "applications-utilities",     "Sensors"     },
};

Sidebar::Sidebar(QWidget *parent)
    : QListWidget(parent)
{
    setup();
}

void Sidebar::setup()
{
    setSpacing(2);
    setIconSize(QSize(22, 22));
    setSelectionMode(QAbstractItemView::SingleSelection);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::NoFocus);

    for (const auto &nav : kNavItems) {
        // Try primary first, fall back to secondary
        QIcon icon = QIcon::fromTheme(QString::fromUtf8(nav.primary));
        if (icon.isNull())
            icon = QIcon::fromTheme(QString::fromUtf8(nav.fallback));

        auto *item = new QListWidgetItem(QString::fromUtf8(nav.label), this);
        if (!icon.isNull())
            item->setIcon(icon);
        item->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    }

    setCurrentRow(0);
    connect(this, &QListWidget::currentRowChanged, this, &Sidebar::pageSelected);
}
