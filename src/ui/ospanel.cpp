// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/ui/ospanel.cpp
#include "ospanel.h"
#include "systeminfo.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>
#include <QPixmap>
#include <QSvgRenderer>
#include <QPainter>
#include <QFileInfo>

#include "style_helper.h"

static QPixmap loadDistroLogo(const QString &path, int size)
{
    if (path.isEmpty()) return {};

    QPixmap pm;

    if (path.endsWith(QLatin1String(".svg"), Qt::CaseInsensitive)) {
        QSvgRenderer renderer(path);
        if (renderer.isValid()) {
            pm = QPixmap(size, size);
            pm.fill(Qt::transparent);
            QPainter painter(&pm);
            renderer.render(&painter);
        }
    } else {
        pm.load(path);
    }

    if (pm.isNull()) return {};

    return pm.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

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

    // ── Distribution section with logo inside the box ─────
    {
        const QList<QPair<QString,QString>> distroRows = {
            {"Pretty Name", os->distroPrettyName()},
            {"Name",        os->distroName()},
            {"Version",     os->distroVersion()},
            {"ID",          os->distroId()},
            {"Color",       os->distroAnsiColor()},
            {"Bug Report",  os->bugReportUrl()}
        };

        auto *distroBox = new QGroupBox(QStringLiteral("Distribution"));
        distroBox->setStyleSheet(StyleHelper::card());

        auto *hLayout = new QHBoxLayout(distroBox);
        hLayout->setSpacing(16);

        // Logo on the left (inside the box)
        const QString logoPath = os->distroLogoPath();
        if (!logoPath.isEmpty()) {
            const int logoSize = 96;
            QPixmap logoPm = loadDistroLogo(logoPath, logoSize);
            if (!logoPm.isNull()) {
                auto *logoLabel = new QLabel();
                logoLabel->setPixmap(logoPm);
                logoLabel->setFixedSize(logoSize, logoSize);
                logoLabel->setAlignment(Qt::AlignCenter);
                logoLabel->setStyleSheet(QStringLiteral("background: transparent; border: none; padding: 0;"));
                hLayout->addWidget(logoLabel, 0, Qt::AlignTop);
            }
        }

        // Form fields on the right
        auto *formWidget = new QWidget();
        auto *form = new QFormLayout(formWidget);
        form->setSpacing(8);
        form->setContentsMargins(0, 0, 0, 0);

        for (const auto &row : distroRows) {
            auto *val = new QLabel(row.second.isEmpty() ? QStringLiteral("N/A") : row.second);

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

        hLayout->addWidget(formWidget, 1);
        root->addWidget(distroBox);
    }

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
