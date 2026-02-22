// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/ui/boardpanel.h
#pragma once
#include <QWidget>

class QVBoxLayout;
class QGroupBox;

class BoardPanel : public QWidget
{
    Q_OBJECT
public:
    explicit BoardPanel(QWidget *parent = nullptr);

private slots:
    void onPrivilegedData(bool success);

private:
    void populateStatic();

    QVBoxLayout *m_root      = nullptr;
    QGroupBox   *m_serialBox = nullptr;
};
