// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/ui/cpupanel.h
#pragma once
#include <QWidget>
class CpuPanel : public QWidget
{
    Q_OBJECT
public:
    explicit CpuPanel(QWidget *parent = nullptr);
};
