// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/ui/memorypanel.cpp
#include "memorypanel.h"
#include "systeminfo.h"
#include "sizeformatter.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QGroupBox>
#include <QProgressBar>
#include "style_helper.h"
#include <QScrollArea>

MemoryPanel::MemoryPanel(QWidget *parent) : QWidget(parent)
{
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *content = new QWidget();
    auto *root    = new QVBoxLayout(content);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);

    auto *title = new QLabel(QStringLiteral("Memory"));
    title->setStyleSheet(StyleHelper::panelTitle());
    root->addWidget(title);

    auto *mem = SystemInfo::instance()->memory();

    // RAM Usage bar
    auto *usageBox = new QGroupBox(QStringLiteral("RAM Usage"));
    auto *ul = new QVBoxLayout(usageBox);

    auto *usageLabel = new QLabel(QStringLiteral("%1 used / %2 total  (%3%)")
        .arg(SizeFormatter::formatBytes(mem->usedKiB() * 1024ULL))
        .arg(SizeFormatter::formatBytes(mem->totalKiB() * 1024ULL))
        .arg(mem->usedPercent()));
    usageLabel->setStyleSheet(QStringLiteral("font-size:14px;font-weight:500;"));

    auto *bar = new QProgressBar();
    bar->setRange(0, 100);
    bar->setValue(mem->usedPercent());
    bar->setFixedHeight(16);
    bar->setTextVisible(false);

    ul->addWidget(usageLabel);
    ul->addWidget(bar);
    root->addWidget(usageBox);

    // Details
    auto *detBox  = new QGroupBox(QStringLiteral("Details"));
    auto *form    = new QFormLayout(detBox);
    form->setSpacing(8);

    QMap<QString, QLabel*> labels;

    auto addRow = [&](const QString &key, const QString &val) {
        auto *l = new QLabel(val);
        l->setStyleSheet(StyleHelper::valueMono());
        form->addRow(new QLabel(QStringLiteral("<b>%1</b>").arg(key)), l);
        labels[key] = l;
    };

    addRow("Total",      SizeFormatter::formatBytes(mem->totalKiB() * 1024ULL));
    addRow("Used",       SizeFormatter::formatBytes(mem->usedKiB() * 1024ULL));
    addRow("Free",       SizeFormatter::formatBytes(mem->freeKiB() * 1024ULL));
    addRow("Available",  SizeFormatter::formatBytes(mem->availableKiB() * 1024ULL));
    addRow("Buffers",    SizeFormatter::formatBytes(mem->buffersKiB() * 1024ULL));
    addRow("Cached",     SizeFormatter::formatBytes(mem->cachedKiB() * 1024ULL));
    root->addWidget(detBox);

    // Swap
    auto *swapBox  = new QGroupBox(QStringLiteral("Swap"));
    auto *swapForm = new QFormLayout(swapBox);
    swapForm->setSpacing(8);

    auto addSwap = [&](const QString &key, const QString &val) {
        auto *l = new QLabel(val);
        l->setStyleSheet(StyleHelper::valueMono());
        swapForm->addRow(new QLabel(QStringLiteral("<b>%1</b>").arg(key)), l);
        labels[key] = l;
    };

    addSwap("Swap Total", SizeFormatter::formatBytes(mem->swapTotalKiB() * 1024ULL));
    addSwap("Swap Used",  SizeFormatter::formatBytes(mem->swapUsedKiB() * 1024ULL));
    addSwap("Swap Free",  SizeFormatter::formatBytes(mem->swapFreeKiB() * 1024ULL));
    root->addWidget(swapBox);

    root->addStretch();
    scroll->setWidget(content);
    auto *l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(scroll);

    // Dynamic update
    connect(SystemInfo::instance(), &SystemInfo::dataRefreshed, this, [=]() mutable {
        auto *m = SystemInfo::instance()->memory();
        usageLabel->setText(QStringLiteral("%1 used / %2 total  (%3%)")
            .arg(SizeFormatter::formatBytes(m->usedKiB() * 1024ULL))
            .arg(SizeFormatter::formatBytes(m->totalKiB() * 1024ULL))
            .arg(m->usedPercent()));
        bar->setValue(m->usedPercent());

        labels["Total"]->setText(SizeFormatter::formatBytes(m->totalKiB() * 1024ULL));
        labels["Used"]->setText(SizeFormatter::formatBytes(m->usedKiB() * 1024ULL));
        labels["Free"]->setText(SizeFormatter::formatBytes(m->freeKiB() * 1024ULL));
        labels["Available"]->setText(SizeFormatter::formatBytes(m->availableKiB() * 1024ULL));
        labels["Buffers"]->setText(SizeFormatter::formatBytes(m->buffersKiB() * 1024ULL));
        labels["Cached"]->setText(SizeFormatter::formatBytes(m->cachedKiB() * 1024ULL));

        labels["Swap Total"]->setText(SizeFormatter::formatBytes(m->swapTotalKiB() * 1024ULL));
        labels["Swap Used"]->setText(SizeFormatter::formatBytes(m->swapUsedKiB() * 1024ULL));
        labels["Swap Free"]->setText(SizeFormatter::formatBytes(m->swapFreeKiB() * 1024ULL));
    });
}
