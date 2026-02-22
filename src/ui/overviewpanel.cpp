// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/ui/overviewpanel.cpp
#include "overviewpanel.h"
#include "systeminfo.h"
#include "sizeformatter.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QProgressBar>
#include <QScrollArea>
#include "style_helper.h"

static QLabel* makeKey(const QString &text)
{
    auto *l = new QLabel(text + ":");
    l->setStyleSheet(QStringLiteral("font-weight:bold;font-size:12px;min-width:160px;"));
    return l;
}

static QLabel* makeVal(const QString &text)
{
    auto *l = new QLabel(text.isEmpty() ? QStringLiteral("N/A") : text);
    l->setStyleSheet(QStringLiteral("font-size:13px;font-weight:500;"));
    l->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return l;
}

OverviewPanel::OverviewPanel(QWidget *parent) : QWidget(parent)
{
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *content = new QWidget();
    auto *root    = new QVBoxLayout(content);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);

    auto *titleLabel = new QLabel(QStringLiteral("System Overview"));
    titleLabel->setStyleSheet(StyleHelper::panelTitle());
    root->addWidget(titleLabel);

    auto *si = SystemInfo::instance();
    auto *cpu = si->cpu();
    auto *mem = si->memory();
    auto *os  = si->os();
    auto *board = si->board();

    // ── Summary cards in a 2-column grid ─────────────────
    auto *grid = new QGridLayout();
    grid->setSpacing(12);

    struct Card {
        QString title;
        QString value;
    };

    const QList<Card> cards = {
        { "CPU",        cpu->modelName() },
        { "Memory",     SizeFormatter::formatBytes(mem->totalKiB() * 1024ULL) },
        { "OS",         os->distroName() },
        { "Kernel",     os->kernelVersion() },
        { "Board",      board->boardName() },
        { "Hostname",   os->hostname() },
    };

    for (int i = 0; i < cards.size(); ++i) {
        const auto &card = cards.at(i);
        auto *box = new QGroupBox();
        box->setStyleSheet(StyleHelper::card());

        auto *vl = new QVBoxLayout(box);
        auto *keyL = new QLabel(card.title);
        keyL->setStyleSheet(StyleHelper::formKey());
        auto *valL = new QLabel(card.value.isEmpty() ? QStringLiteral("N/A") : card.value);
        valL->setStyleSheet(StyleHelper::valueText());
        valL->setWordWrap(true);
        vl->addWidget(keyL);
        vl->addWidget(valL);
        grid->addWidget(box, i / 2, i % 2);
    }

    root->addLayout(grid);

    // ── Memory usage bar ──────────────────────────────────
    QLabel *usedLabel = nullptr;
    QProgressBar *bar = nullptr;

    if (mem->totalKiB() > 0) {
        auto *memBox = new QGroupBox(QStringLiteral("Memory Usage"));
        memBox->setStyleSheet(StyleHelper::card());
        auto *ml = new QVBoxLayout(memBox);

        usedLabel = new QLabel(QStringLiteral("%1 / %2  (%3%)")
            .arg(SizeFormatter::formatBytes(mem->usedKiB() * 1024ULL))
            .arg(SizeFormatter::formatBytes(mem->totalKiB() * 1024ULL))
            .arg(mem->usedPercent()));
        usedLabel->setStyleSheet(StyleHelper::valueMono());

        bar = new QProgressBar();
        bar->setRange(0, 100);
        bar->setValue(mem->usedPercent());
        bar->setFixedHeight(12);
        bar->setTextVisible(false);
        bar->setStyleSheet(StyleHelper::progressBar());

        ml->addWidget(usedLabel);
        ml->addWidget(bar);
        root->addWidget(memBox);
    }

    root->addStretch();
    scroll->setWidget(content);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(scroll);

    // Dynamic update
    connect(SystemInfo::instance(), &SystemInfo::dataRefreshed, this, [=]() mutable {
        auto *m = SystemInfo::instance()->memory();
        if (usedLabel && bar) {
            usedLabel->setText(QStringLiteral("%1 / %2  (%3%)")
                .arg(SizeFormatter::formatBytes(m->usedKiB() * 1024ULL))
                .arg(SizeFormatter::formatBytes(m->totalKiB() * 1024ULL))
                .arg(m->usedPercent()));
            bar->setValue(m->usedPercent());
        }
    });
}
