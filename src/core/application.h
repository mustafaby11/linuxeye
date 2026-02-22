// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/core/application.h
#pragma once

#include <QApplication>

class Application : public QApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);

signals:
    void themeChanged(bool isDark);

private:
    void applyStyle();
};
