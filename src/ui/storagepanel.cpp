// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/ui/storagepanel.cpp
#include "storagepanel.h"
#include "systeminfo.h"
#include "sizeformatter.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFormLayout>
#include <QScrollArea>
#include <QTreeWidget>
#include <QHeaderView>
#include <QProgressBar>
#include "style_helper.h"

StoragePanel::StoragePanel(QWidget *parent) : QWidget(parent)
{
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *content = new QWidget();
    auto *root    = new QVBoxLayout(content);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);

    auto *title = new QLabel(QStringLiteral("Storage Overview"));
    title->setStyleSheet(StyleHelper::panelTitle());
    root->addWidget(title);

    const auto &disks = SystemInfo::instance()->storage()->disks();

    if (disks.isEmpty()) {
        auto *noData = new QLabel(QStringLiteral("No storage devices found."));
        noData->setStyleSheet(StyleHelper::summaryText());
        root->addWidget(noData);
    } else {
        quint64 totalCapacity = 0;
        for (const auto &d : disks) totalCapacity += d.sizeBytes;

        // ── Top Level: Overview ───────────────────────
        auto *summary = new QLabel(QStringLiteral("<b>Total Devices:</b> %1 &nbsp;&nbsp;&bull;&nbsp;&nbsp; <b>Total Capacity:</b> %2")
            .arg(disks.size()).arg(SizeFormatter::formatBytes(totalCapacity)));
        summary->setStyleSheet(StyleHelper::summaryText());
        summary->setTextFormat(Qt::RichText);
        root->addWidget(summary);

        // ── Mid Level: Disks ──────────────────────────
        for (const auto &disk : disks) {
            auto *box  = new QGroupBox(QStringLiteral("%1  (%2)")
                .arg(disk.model.isEmpty() ? disk.name : disk.model, disk.type));
            box->setStyleSheet(StyleHelper::card());
            
            auto *vl = new QVBoxLayout(box);
            vl->setSpacing(12);

            auto *diskInfo = new QLabel(QStringLiteral("<b>Device:</b> /dev/%1 &nbsp;&nbsp;|&nbsp;&nbsp; <b>Capacity:</b> %2")
                .arg(disk.name).arg(SizeFormatter::formatBytes(disk.sizeBytes)));
            diskInfo->setStyleSheet(StyleHelper::summaryText());
            vl->addWidget(diskInfo);

            // ── Bottom Level: Partitions & Mounts ─────
            if (!disk.partitions.isEmpty()) {
                auto *tree = new QTreeWidget();
                tree->setHeaderLabels({"Partition", "Size", "Filesystem", "Mount Point", "Usage"});
                tree->setRootIsDecorated(false);
                tree->setAlternatingRowColors(true);
                tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
                tree->header()->setSectionResizeMode(4, QHeaderView::Stretch);
                tree->setSelectionMode(QAbstractItemView::NoSelection);
                tree->setFocusPolicy(Qt::NoFocus);
                tree->setStyleSheet(StyleHelper::tree());

                for (const auto &p : disk.partitions) {
                    auto *item = new QTreeWidgetItem(tree);
                    item->setText(0, p.name);
                    item->setText(1, SizeFormatter::formatBytes(p.sizeBytes));
                    item->setText(2, p.filesystem.isEmpty() ? QStringLiteral("-") : p.filesystem);
                    item->setText(3, p.mountPoint.isEmpty() ? QStringLiteral("Unmounted") : p.mountPoint);

                    if (!p.mountPoint.isEmpty() && p.fsTotalBytes > 0) {
                        auto *bar = new QProgressBar();
                        bar->setFixedHeight(16);
                        bar->setRange(0, 100);
                        int pct = (p.fsUsedBytes * 100) / p.fsTotalBytes;
                        bar->setValue(pct);
                        bar->setStyleSheet(StyleHelper::progressBar());

                        bar->setFormat(QStringLiteral("%1 / %2 (%3%)")
                            .arg(SizeFormatter::formatBytes(p.fsUsedBytes))
                            .arg(SizeFormatter::formatBytes(p.fsTotalBytes))
                            .arg(pct));
                        
                        tree->setItemWidget(item, 4, bar);
                    } else {
                        item->setText(4, QStringLiteral("-"));
                        item->setTextAlignment(4, Qt::AlignCenter);
                    }
                }
                vl->addWidget(tree);
            } else {
                auto *noPart = new QLabel(QStringLiteral("No partitions mapped directly."));
                vl->addWidget(noPart);
            }
            root->addWidget(box);
        }
    }

    root->addStretch();
    scroll->setWidget(content);
    auto *l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(scroll);
}
