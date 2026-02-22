// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/ui/sensorpanel.cpp
#include "sensorpanel.h"
#include "systeminfo.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>
#include <QTreeWidget>
#include <QHeaderView>
#include <QColor>
#include <QPalette>
#include "style_helper.h"

SensorPanel::SensorPanel(QWidget *parent) : QWidget(parent)
{
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *content = new QWidget();
    auto *root    = new QVBoxLayout(content);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);

    auto *title = new QLabel(QStringLiteral("Sensors"));
    title->setStyleSheet(StyleHelper::panelTitle());
    root->addWidget(title);

    const auto &readings = SystemInfo::instance()->sensors()->readings();

    QMap<QString, QTreeWidgetItem*> items;

    if (readings.isEmpty()) {
        auto *noData = new QLabel(
            QStringLiteral("No sensor data available.\nMake sure hwmon drivers are loaded."));
        noData->setStyleSheet(StyleHelper::summaryText());
        root->addWidget(noData);
    } else {
        // Group by chip
        QMap<QString, QList<const SensorReading*>> byChip;
        for (const auto &r : readings)
            byChip[r.chip].append(&r);

        for (auto it = byChip.cbegin(); it != byChip.cend(); ++it) {
            auto *box  = new QGroupBox(it.key());
            auto *vl   = new QVBoxLayout(box);

            auto *tree = new QTreeWidget();
            tree->setHeaderLabels({QStringLiteral("Label"),
                                   QStringLiteral("Type"),
                                   QStringLiteral("Value")});
            tree->setRootIsDecorated(false);
            tree->setAlternatingRowColors(true);
            tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
            tree->setSelectionMode(QAbstractItemView::NoSelection);
            tree->setFocusPolicy(Qt::NoFocus);

            for (const auto *r : it.value()) {
                auto *item = new QTreeWidgetItem(tree);
                item->setText(0, r->label);
                item->setText(1, r->type);
                const QString valStr =
                    QStringLiteral("%1 %2").arg(r->value, 0, 'f', 1).arg(r->unit);
                item->setText(2, valStr);

                // Color-code temperatures using system palette
                if (r->type == QLatin1String("temp")) {
                    QPalette palette;
                    const QString col = r->value >= 80 ? palette.color(QPalette::Text).name()
                                      : r->value >= 60 ? palette.color(QPalette::Highlight).name()
                                      : palette.color(QPalette::Text).name();
                    item->setForeground(2, QColor(col));
                }
                
                items[r->chip + "|" + r->label] = item;
            }

            vl->addWidget(tree);
            root->addWidget(box);
        }
    }

    root->addStretch();
    scroll->setWidget(content);
    auto *l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(scroll);

    // Dynamic update
    connect(SystemInfo::instance(), &SystemInfo::dataRefreshed, this, [=]() mutable {
        const auto &newReadings = SystemInfo::instance()->sensors()->readings();
        for (const auto &r : newReadings) {
            const QString key = r.chip + "|" + r.label;
            if (items.contains(key)) {
                auto *item = items[key];
                const QString valStr =
                    QStringLiteral("%1 %2").arg(r.value, 0, 'f', 1).arg(r.unit);
                item->setText(2, valStr);

                if (r.type == QLatin1String("temp")) {
                    QPalette palette;
                    const QString col = r.value >= 80 ? palette.color(QPalette::Text).name()
                                      : r.value >= 60 ? palette.color(QPalette::Highlight).name()
                                      : palette.color(QPalette::Text).name();
                    item->setForeground(2, QColor(col));
                }
            }
        }
    });
}
