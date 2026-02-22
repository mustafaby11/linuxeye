// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/collectors/networkcollector.h
#pragma once

#include "basecollector.h"
#include <QString>
#include <QList>

struct NetworkInterface {
    QString name;
    QString ipv4;
    QString ipv6;
    QString macAddress;
    quint64 rxBytes = 0;
    quint64 txBytes = 0;
    bool    isUp    = false;
    QString driver;
    QString hwName;
    quint64 speedMbps = 0;
};

class NetworkCollector : public BaseCollector
{
public:
    void collect() override;
    QString name() const override { return QStringLiteral("Network"); }

    const QList<NetworkInterface>& interfaces() const { return m_interfaces; }

private:
    QList<NetworkInterface> m_interfaces;

    void parseNetDev();
    void parseIfaceDetails(NetworkInterface &iface);
};
