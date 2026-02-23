// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/ui/mainwindow.cpp
#include "mainwindow.h"
#include "sidebar.h"
#include "overviewpanel.h"
#include "cpupanel.h"
#include "memorypanel.h"
#include "ospanel.h"
#include "storagepanel.h"
#include "networkpanel.h"
#include "gpupanel.h"
#include "boardpanel.h"
#include "sensorpanel.h"
#include "systeminfo.h"
#include "../core/application.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QStatusBar>
#include <QDateTime>
#include <QIcon>
#include <QTimer>
#include <QDesktopServices>
#include <QUrl>
#include <QSvgRenderer>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("linuxeye – System Analyzer"));
    resize(1100, 720);
    setMinimumSize(820, 560);

    // Data is already collected in main.cpp (before the window was created).
    // setupUi() creates panels that read the already-populated data.
    setupUi();

    // Show privilege status based on what startSync() did in main.cpp
    const bool privileged = SystemInfo::instance()->privileged()->data().isValid;
    const QString ts = QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss"));
    statusBar()->showMessage(privileged
        ? QStringLiteral("Ready — privileged data loaded at %1").arg(ts)
        : QStringLiteral("Ready — unprivileged mode (serial numbers unavailable)"));

    // Update the icon to reflect auth outcome
    onPrivilegedDataReady(privileged);

    // Setup auto-refresh timer (every 1 second for live feeling)
    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, []() {
        SystemInfo::instance()->collectAll();
    });
    timer->start(1000);
}

void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ── Top header bar ────────────────────────────────
    auto *headerBar = new QWidget();
    headerBar->setObjectName(QStringLiteral("headerBar"));
    headerBar->setFixedHeight(52);

    auto *headerLayout = new QHBoxLayout(headerBar);
    headerLayout->setContentsMargins(16, 0, 16, 0);

    auto *logoLabel = new QLabel(
        QStringLiteral("<b>linux</b><span style='color:palette(highlight);font-weight:bold;'>eye</span>"));
    logoLabel->setTextFormat(Qt::RichText);
    logoLabel->setStyleSheet(QStringLiteral("font-size:18px;padding-left:4px;"));

    auto *versionLabel = new QLabel(QStringLiteral("v0.1.0"));
    versionLabel->setStyleSheet(QStringLiteral("font-size:11px;"));

    // Theme is now handled by the system - no toggle button needed

    m_privStatusLabel = new QLabel();
    m_privStatusLabel->setFixedSize(20, 20);

    // GitHub link button
    auto *githubBtn = new QPushButton();
    githubBtn->setToolTip(QStringLiteral("View on GitHub"));
    githubBtn->setFixedSize(28, 28);
    githubBtn->setCursor(Qt::PointingHandCursor);
    githubBtn->setFlat(true);
    githubBtn->setStyleSheet(QStringLiteral(
        "QPushButton { border: none; padding: 4px; border-radius: 4px; }"
        "QPushButton:hover { background: palette(midlight); }"));

    // Load GitHub icon from embedded resource
    {
        QSvgRenderer renderer(QStringLiteral(":/svg/github.svg"));
        QPixmap pm(64, 64);
        pm.fill(Qt::transparent);
        QPainter painter(&pm);
        renderer.render(&painter);
        githubBtn->setIcon(QIcon(pm));
        githubBtn->setIconSize(QSize(16, 16));
    }

    connect(githubBtn, &QPushButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://github.com/mustafaby11/linuxeye")));
    });

    headerLayout->addWidget(logoLabel);
    headerLayout->addSpacing(8);
    headerLayout->addWidget(versionLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(githubBtn);
    headerLayout->addSpacing(8);
    headerLayout->addWidget(m_privStatusLabel);

    rootLayout->addWidget(headerBar);

    // ── Body: sidebar + stack ─────────────────────────
    auto *bodyWidget = new QWidget();
    auto *bodyLayout = new QHBoxLayout(bodyWidget);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);

    m_sidebar = new Sidebar();
    m_stack   = new QStackedWidget();

    m_stack->addWidget(new OverviewPanel());
    m_stack->addWidget(new CpuPanel());
    m_stack->addWidget(new MemoryPanel());
    m_stack->addWidget(new OsPanel());
    m_stack->addWidget(new StoragePanel());
    m_stack->addWidget(new NetworkPanel());
    m_stack->addWidget(new GpuPanel());
    m_stack->addWidget(new BoardPanel());
    m_stack->addWidget(new SensorPanel());

    bodyLayout->addWidget(m_sidebar);
    bodyLayout->addWidget(m_stack, 1);
    rootLayout->addWidget(bodyWidget, 1);

    connect(m_sidebar, &Sidebar::pageSelected,
            m_stack,   &QStackedWidget::setCurrentIndex);
}

void MainWindow::onPrivilegedDataReady(bool success)
{
    const QIcon icon = success
        ? QIcon::fromTheme(QStringLiteral("security-high-symbolic"),
                           QIcon::fromTheme(QStringLiteral("dialog-ok")))
        : QIcon::fromTheme(QStringLiteral("security-low-symbolic"),
                           QIcon::fromTheme(QStringLiteral("dialog-warning")));

    if (!icon.isNull())
        m_privStatusLabel->setPixmap(icon.pixmap(16, 16));

    m_privStatusLabel->setToolTip(success
        ? QStringLiteral("Privileged data loaded")
        : QStringLiteral("Unprivileged mode — serial numbers unavailable"));
}
