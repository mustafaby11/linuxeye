// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/ui/networkpanel.cpp
#include "networkpanel.h"
#include "systeminfo.h"
#include "sizeformatter.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFormLayout>
#include <QScrollArea>
#include <QPalette>
#include "style_helper.h"

NetworkPanel::NetworkPanel(QWidget *parent) : QWidget(parent)
{
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *content = new QWidget();
    auto *root    = new QVBoxLayout(content);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);

    auto *title = new QLabel(QStringLiteral("Network"));
    title->setStyleSheet(StyleHelper::panelTitle());
    root->addWidget(title);

    const auto &ifaces = SystemInfo::instance()->network()->interfaces();

    QMap<QString, QLabel*> labels;

    if (ifaces.isEmpty()) {
        auto *noData = new QLabel(QStringLiteral("No network interfaces found."));
        noData->setStyleSheet(StyleHelper::summaryText());
        root->addWidget(noData);
    } else {
        for (const auto &iface : ifaces) {
            QPalette palette;
            const QString statusColor = iface.isUp ? palette.color(QPalette::Text).name() : palette.color(QPalette::Highlight).name();
            const QString statusText  = iface.isUp ? "Up" : "Down";
            auto *box  = new QGroupBox(iface.name);
            auto *form = new QFormLayout(box);
            form->setSpacing(8);

            auto addRow = [&](const QString &key, const QString &val, const QString &style = {}) {
                auto *l = new QLabel(val.isEmpty() ? QStringLiteral("N/A") : val);
                l->setStyleSheet(style.isEmpty()
                    ? StyleHelper::valueText()
                    : style);
                l->setTextInteractionFlags(Qt::TextSelectableByMouse);
                form->addRow(new QLabel(QStringLiteral("<b>%1</b>").arg(key)), l);
                labels[iface.name + "|" + key] = l;
            };

            addRow("Status",  statusText,
                QStringLiteral("color:%1;font-weight:bold;").arg(statusColor));
            addRow("MAC",     iface.macAddress);
            if (iface.speedMbps > 0)
                addRow("Speed",   QStringLiteral("%1 Mbps").arg(iface.speedMbps));
            addRow("RX",      SizeFormatter::formatBytes(iface.rxBytes));
            addRow("TX",      SizeFormatter::formatBytes(iface.txBytes));
            if (!iface.hwName.isEmpty())
                addRow("Hardware", iface.hwName);
            if (!iface.driver.isEmpty())
                addRow("Driver",  iface.driver);

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
        const auto &newIfaces = SystemInfo::instance()->network()->interfaces();
        for (const auto &iface : newIfaces) {
            const QString prefix = iface.name + "|";
            if (labels.contains(prefix + "RX"))
                labels[prefix + "RX"]->setText(SizeFormatter::formatBytes(iface.rxBytes));
            if (labels.contains(prefix + "TX"))
                labels[prefix + "TX"]->setText(SizeFormatter::formatBytes(iface.txBytes));
            
            if (labels.contains(prefix + "Status")) {
                QPalette palette;
                const QString statusColor = iface.isUp ? palette.color(QPalette::Text).name() : palette.color(QPalette::Highlight).name();
                const QString statusText  = iface.isUp ? "Up" : "Down";
                auto *lObj = labels[prefix + "Status"];
                lObj->setText(statusText);
                lObj->setStyleSheet(QStringLiteral("color:%1;font-weight:bold;").arg(statusColor));
            }
        }
    });
}
