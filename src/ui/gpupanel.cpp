// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/ui/gpupanel.cpp
#include "gpupanel.h"
#include "systeminfo.h"
#include "sizeformatter.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFormLayout>
#include <QScrollArea>
#include "style_helper.h"

GpuPanel::GpuPanel(QWidget *parent) : QWidget(parent)
{
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *content = new QWidget();
    auto *root    = new QVBoxLayout(content);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);

    auto *title = new QLabel(QStringLiteral("GPU"));
    title->setStyleSheet(StyleHelper::panelTitle());
    root->addWidget(title);

    const auto &gpus = SystemInfo::instance()->gpu()->gpus();

    QMap<QString, QLabel*> labels;

    if (gpus.isEmpty()) {
        auto *noData = new QLabel(QStringLiteral("No GPU detected via DRM/KMS."));
        noData->setStyleSheet(StyleHelper::summaryText());
        root->addWidget(noData);
    } else {
        int idx = 0;
        for (const auto &gpu : gpus) {
            auto *box  = new QGroupBox(QStringLiteral("GPU %1: %2")
                .arg(idx).arg(gpu.name.isEmpty() ? QStringLiteral("Unknown") : gpu.name));
            auto *form = new QFormLayout(box);
            form->setSpacing(8);

            auto addRow = [&](const QString &key, const QString &val) {
                if (val.isEmpty() || val == QLatin1String("0")) return;
                auto *l = new QLabel(val);
                l->setStyleSheet(StyleHelper::valueText());
                l->setTextInteractionFlags(Qt::TextSelectableByMouse);
                form->addRow(new QLabel(QStringLiteral("<b>%1</b>").arg(key)), l);
                labels[QString::number(idx) + "|" + key] = l;
            };

            addRow("Driver",      gpu.driver);
            addRow("PCI ID",      gpu.pciId);
            addRow("Render Node", gpu.renderNode);
            if (gpu.vramBytes > 0)
                addRow("VRAM",    SizeFormatter::formatBytes(gpu.vramBytes));
            if (gpu.tempCelsius >= 0)
                addRow("Temperature", QStringLiteral("%1 °C").arg(gpu.tempCelsius));

            root->addWidget(box);
            idx++;
        }
    }

    root->addStretch();
    scroll->setWidget(content);
    auto *l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(scroll);

    // Dynamic update
    connect(SystemInfo::instance(), &SystemInfo::dataRefreshed, this, [=]() mutable {
        const auto &newGpus = SystemInfo::instance()->gpu()->gpus();
        int i = 0;
        for (const auto &gpu : newGpus) {
            const QString prefix = QString::number(i) + "|";
            if (gpu.tempCelsius >= 0 && labels.contains(prefix + "Temperature")) {
                labels[prefix + "Temperature"]->setText(QStringLiteral("%1 °C").arg(gpu.tempCelsius));
            }
            if (gpu.vramBytes > 0 && labels.contains(prefix + "VRAM")) {
                labels[prefix + "VRAM"]->setText(SizeFormatter::formatBytes(gpu.vramBytes));
            }
            i++;
        }
    });
}
