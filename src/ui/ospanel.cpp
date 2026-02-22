// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/ui/ospanel.cpp
#include "ospanel.h"
#include "systeminfo.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>

#include "style_helper.h"

OsPanel::OsPanel(QWidget *parent) : QWidget(parent)
{
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *content = new QWidget();
    auto *root    = new QVBoxLayout(content);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);

    auto *title = new QLabel(QStringLiteral("OS / Kernel"));
    title->setStyleSheet(StyleHelper::panelTitle());
    root->addWidget(title);

    auto *os = SystemInfo::instance()->os();

    QMap<QString, QLabel*> labels;

    auto buildBox = [&](const QString &boxTitle, const QList<QPair<QString,QString>> &rows) {
        auto *box  = new QGroupBox(boxTitle);
        box->setStyleSheet(StyleHelper::card());
        auto *form = new QFormLayout(box);
        form->setSpacing(8);
        for (const auto &row : rows) {
            auto *val = new QLabel(row.second.isEmpty() ? QStringLiteral("N/A") : row.second);
            
            // Format URL as a clickable link if applicable
            if (row.first == QLatin1String("Bug Report")) {
                val->setText(QStringLiteral("<a href='%1' style='font-weight:bold;'>%1</a>").arg(row.second));
                val->setOpenExternalLinks(true);
            } else if (row.first == QLatin1String("Pretty Name")) {
                const QString ansiColor = os->distroAnsiColor();
                if (!ansiColor.isEmpty()) {
                    val->setStyleSheet(StyleHelper::valueHighlight());
                } else {
                    val->setStyleSheet(StyleHelper::valueText());
                }
            } else {
                val->setStyleSheet(StyleHelper::valueText());
            }
            
            val->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
            form->addRow(new QLabel(QStringLiteral("<b>%1</b>").arg(row.first)), val);
            labels[row.first] = val;
        }
        return box;
    };

    root->addWidget(buildBox(QStringLiteral("Distribution"), {
        {"Pretty Name", os->distroPrettyName()},
        {"Name",        os->distroName()},
        {"Version",     os->distroVersion()},
        {"ID",          os->distroId()},
        {"Color",       os->distroAnsiColor()},
        {"Bug Report",  os->bugReportUrl()}
    }));

    root->addWidget(buildBox(QStringLiteral("Kernel"), {
        {"Version",      os->kernelVersion()},
        {"Architecture", os->kernelArch()},
    }));

    root->addWidget(buildBox(QStringLiteral("System"), {
        {"Hostname",    os->hostname()},
        {"Uptime",      os->uptime()},
        {"Init System", os->initSystem()},
        {"Desktop Env", os->desktopEnv()},
        {"Win Manager", os->windowManager()},
        {"Shell",       os->shell()},
    }));

    root->addStretch();
    scroll->setWidget(content);
    auto *l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(scroll);

    // Dynamic update for Uptime
    connect(SystemInfo::instance(), &SystemInfo::dataRefreshed, this, [=]() mutable {
        auto *o = SystemInfo::instance()->os();
        if (labels.contains("Uptime")) {
            labels["Uptime"]->setText(o->uptime());
        }
    });
}
