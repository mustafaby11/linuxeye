// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/ui/sidebar.h
#pragma once

#include <QListWidget>

class Sidebar : public QListWidget
{
    Q_OBJECT
public:
    explicit Sidebar(QWidget *parent = nullptr);

signals:
    void pageSelected(int index);

private:
    void setup();
};
