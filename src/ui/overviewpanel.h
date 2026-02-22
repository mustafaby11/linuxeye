// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/ui/overviewpanel.h
#pragma once
#include <QWidget>
class QLabel;

class OverviewPanel : public QWidget
{
    Q_OBJECT
public:
    explicit OverviewPanel(QWidget *parent = nullptr);
private:
    void populate();
    QLabel *addRow(QWidget *parent, const QString &key, const QString &value);
};
