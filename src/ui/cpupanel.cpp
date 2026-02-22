// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/ui/cpupanel.cpp
#include "cpupanel.h"
#include "systeminfo.h"
#include "sizeformatter.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>
#include <QTreeWidget>
#include "style_helper.h"

CpuPanel::CpuPanel(QWidget *parent) : QWidget(parent)
{
    auto *scroll  = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *content = new QWidget();
    auto *root    = new QVBoxLayout(content);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);

    auto *title = new QLabel(QStringLiteral("CPU"));
    title->setStyleSheet(StyleHelper::panelTitle());
    root->addWidget(title);

    auto *cpu = SystemInfo::instance()->cpu();

    // ── General Info ──────────────────────────────────────
    auto *genBox = new QGroupBox(QStringLiteral("Processor"));
    genBox->setStyleSheet(StyleHelper::card());
    auto *form   = new QFormLayout(genBox);
    form->setSpacing(8);

    QMap<QString, QLabel*> labels;

    auto addRow = [&](const QString &label, const QString &value) {
        auto *val = new QLabel(value.isEmpty() ? QStringLiteral("N/A") : value);
        val->setStyleSheet(StyleHelper::valueText());
        val->setTextInteractionFlags(Qt::TextSelectableByMouse);
        form->addRow(
            new QLabel(QStringLiteral("<b>%1</b>").arg(label)), val);
        labels[label] = val;
    };

    addRow("Model",           cpu->modelName());
    addRow("Vendor",          cpu->vendor());
    addRow("Architecture",    cpu->architecture());
    addRow("Physical Cores",  QString::number(cpu->physicalCores()));
    addRow("Logical Cores",   QString::number(cpu->logicalCores()));
    addRow("Max Frequency",
        cpu->maxFreqMHz() > 0
            ? SizeFormatter::formatHz(quint64(cpu->maxFreqMHz() * 1'000'000))
            : QStringLiteral("N/A"));
    addRow("Current Frequency",
        cpu->curFreqMHz() > 0
            ? SizeFormatter::formatHz(quint64(cpu->curFreqMHz() * 1'000'000))
            : QStringLiteral("N/A"));
    root->addWidget(genBox);

    // ── Cache ─────────────────────────────────────────────
    auto *cacheBox  = new QGroupBox(QStringLiteral("Cache"));
    auto *cacheForm = new QFormLayout(cacheBox);
    cacheForm->setSpacing(8);

    auto addCache = [&](const QString &label, const QString &value) {
        if (value.isEmpty()) return;
        auto *val = new QLabel(value);
        val->setStyleSheet(StyleHelper::valueMono());
        cacheForm->addRow(
            new QLabel(QStringLiteral("<b>%1</b>").arg(label)), val);
    };

    addCache("L1 Data",    cpu->cacheL1d());
    addCache("L1 Instr",   cpu->cacheL1i());
    addCache("L2",         cpu->cacheL2());
    addCache("L3",         cpu->cacheL3());
    if (cacheForm->rowCount() > 0)
        root->addWidget(cacheBox);

    // ── CPU Flags ─────────────────────────────────────────
    const QStringList &flags = cpu->flags();
    if (!flags.isEmpty()) {
        auto *flagBox = new QGroupBox(QStringLiteral("CPU Flags (%1)").arg(flags.size()));
        auto *fl      = new QVBoxLayout(flagBox);
        auto *flagLabel = new QLabel(flags.join(QStringLiteral("  ")));
        flagLabel->setStyleSheet(QStringLiteral(
            "font-family:monospace;font-size:11px;"));
        flagLabel->setWordWrap(true);
        flagLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        fl->addWidget(flagLabel);
        root->addWidget(flagBox);
    }

    root->addStretch();
    scroll->setWidget(content);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(scroll);

    // Dynamic update
    connect(SystemInfo::instance(), &SystemInfo::dataRefreshed, this, [=]() mutable {
        auto *c = SystemInfo::instance()->cpu();
        labels["Max Frequency"]->setText(c->maxFreqMHz() > 0
            ? SizeFormatter::formatHz(quint64(c->maxFreqMHz() * 1'000'000))
            : QStringLiteral("N/A"));
        labels["Current Frequency"]->setText(c->curFreqMHz() > 0
            ? SizeFormatter::formatHz(quint64(c->curFreqMHz() * 1'000'000))
            : QStringLiteral("N/A"));
    });
}
