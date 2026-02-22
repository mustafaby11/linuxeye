// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/ui/boardpanel.cpp
#include "boardpanel.h"
#include "systeminfo.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFormLayout>
#include <QScrollArea>

#include "style_helper.h"
BoardPanel::BoardPanel(QWidget *parent) : QWidget(parent)
{
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *content = new QWidget();
    m_root = new QVBoxLayout(content);
    m_root->setContentsMargins(24, 24, 24, 24);
    m_root->setSpacing(16);

    auto *title = new QLabel(QStringLiteral("Motherboard"));
    title->setStyleSheet(StyleHelper::panelTitle());
    m_root->addWidget(title);

    populateStatic();

    m_root->addStretch();
    scroll->setWidget(content);
    auto *l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(scroll);

    // Listen for privileged data
    connect(SystemInfo::instance(), &SystemInfo::privilegedDataReady,
            this, &BoardPanel::onPrivilegedData);
}

void BoardPanel::populateStatic()
{
    auto *board = SystemInfo::instance()->board();

    auto buildBox = [&](const QString &boxTitle, const QList<QPair<QString,QString>> &rows) {
        auto *box  = new QGroupBox(boxTitle);
        box->setStyleSheet(StyleHelper::card());
        auto *form = new QFormLayout(box);
        form->setSpacing(8);
        for (const auto &row : rows) {
            if (row.second.isEmpty()) continue;
            auto *l = new QLabel(row.second);
            l->setStyleSheet(StyleHelper::valueText());
            l->setTextInteractionFlags(Qt::TextSelectableByMouse);
            
            auto *keyLabel = new QLabel(row.first + QLatin1String(":"));
            keyLabel->setStyleSheet(StyleHelper::formKey());
            form->addRow(keyLabel, l);
        }
        if (form->rowCount() == 0) { delete box; return static_cast<QGroupBox*>(nullptr); }
        return box;
    };

    if (auto *b = buildBox(QStringLiteral("Motherboard"), {
        {"Board Vendor",  board->boardVendor()},
        {"Board Name",    board->boardName()},
        {"Board Version", board->boardVersion()},
    })) m_root->addWidget(b);

    if (auto *b = buildBox(QStringLiteral("System"), {
        {"Vendor",       board->sysVendor()},
        {"Product Name", board->productName()},
        {"Chassis Type", board->chassisType()},
    })) m_root->addWidget(b);

    if (auto *b = buildBox(QStringLiteral("BIOS / UEFI"), {
        {"BIOS Vendor",  board->biosVendor()},
        {"BIOS Version", board->biosVersion()},
        {"BIOS Date",    board->biosDate()},
    })) m_root->addWidget(b);
}

void BoardPanel::onPrivilegedData(bool success)
{
    if (!success) return;

    const auto &pd = SystemInfo::instance()->privileged()->data();

    // Remove the old serial box if already present
    if (m_serialBox) {
        m_root->removeWidget(m_serialBox);
        delete m_serialBox;
        m_serialBox = nullptr;
    }

    m_serialBox = new QGroupBox(QStringLiteral("Serial Numbers (privileged)"));
    m_serialBox->setStyleSheet(StyleHelper::card());
    auto *form  = new QFormLayout(m_serialBox);
    form->setSpacing(8);

    auto addRow = [&](const QString &key, const QString &val) {
        if (val.isEmpty() || val.startsWith(QLatin1String("To Be"))) return;
        auto *l = new QLabel(val);
        l->setStyleSheet(StyleHelper::valueMono());
        l->setTextInteractionFlags(Qt::TextSelectableByMouse);
        
        auto *keyLabel = new QLabel(key + QLatin1String(":"));
        keyLabel->setStyleSheet(StyleHelper::formKey());
        form->addRow(keyLabel, l);
    };

    addRow("Board Serial",   pd.boardSerial);
    addRow("Chassis Serial", pd.chassisSerial);
    addRow("Product Serial", pd.productSerial);
    addRow("Product UUID",   pd.productUuid);

    if (form->rowCount() == 0) {
        delete m_serialBox;
        m_serialBox = nullptr;
        return;
    }

    // Insert before the stretch (last item)
    m_root->insertWidget(m_root->count() - 1, m_serialBox);
}
