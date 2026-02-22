// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/ui/mainwindow.h
#pragma once

#include <QMainWindow>

class Sidebar;
class QStackedWidget;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onPrivilegedDataReady(bool success);

private:
    void setupUi();
    void setupStatusBar();
    void refresh();

    Sidebar        *m_sidebar = nullptr;
    QStackedWidget *m_stack   = nullptr;
    QLabel         *m_privStatusLabel = nullptr;
};
