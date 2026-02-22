// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/ui/gpupanel.h
#pragma once
#include <QWidget>
class GpuPanel : public QWidget
{
    Q_OBJECT
public:
    explicit GpuPanel(QWidget *parent = nullptr);
};
