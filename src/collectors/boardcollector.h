// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/collectors/boardcollector.h
#pragma once

#include "basecollector.h"
#include <QString>

class BoardCollector : public BaseCollector
{
public:
    void collect() override;
    QString name() const override { return QStringLiteral("Motherboard"); }

    QString boardVendor()   const { return m_boardVendor; }
    QString boardName()     const { return m_boardName; }
    QString boardVersion()  const { return m_boardVersion; }
    QString biosVendor()    const { return m_biosVendor; }
    QString biosVersion()   const { return m_biosVersion; }
    QString biosDate()      const { return m_biosDate; }
    QString biosReleaseDate() const { return m_biosReleaseDate; }
    QString chassisType()   const { return m_chassisType; }
    QString sysVendor()     const { return m_sysVendor; }
    QString productName()   const { return m_productName; }

private:
    QString m_boardVendor;
    QString m_boardName;
    QString m_boardVersion;
    QString m_biosVendor;
    QString m_biosVersion;
    QString m_biosDate;
    QString m_biosReleaseDate;
    QString m_chassisType;
    QString m_sysVendor;
    QString m_productName;

    void readDmiAttribute(const QString &attr, QString &out);
};
