// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/core/systeminfo.h
#pragma once

#include <QObject>
#include <memory>

#include "cpucollector.h"
#include "memorycollector.h"
#include "oscollector.h"
#include "storagecollector.h"
#include "networkcollector.h"
#include "gpucollector.h"
#include "boardcollector.h"
#include "sensorcollector.h"
#include "privilegedcollector.h"

/// Central data aggregator – singleton accessed by UI panels.
class SystemInfo : public QObject
{
    Q_OBJECT
public:
    static SystemInfo* instance();

    void collectAll();

    /// Trigger async privileged collection (shows pkexec auth dialog).
    void collectPrivileged();

    CpuCollector*        cpu()        const { return m_cpu.get(); }
    MemoryCollector*     memory()     const { return m_memory.get(); }
    OsCollector*         os()         const { return m_os.get(); }
    StorageCollector*    storage()    const { return m_storage.get(); }
    NetworkCollector*    network()    const { return m_network.get(); }
    GpuCollector*        gpu()        const { return m_gpu.get(); }
    BoardCollector*      board()      const { return m_board.get(); }
    SensorCollector*     sensors()    const { return m_sensors.get(); }
    PrivilegedCollector* privileged() const { return m_privileged.get(); }

signals:
    void dataRefreshed();
    void privilegedDataReady(bool success);

private:
    explicit SystemInfo(QObject *parent = nullptr);

    std::unique_ptr<CpuCollector>        m_cpu;
    std::unique_ptr<MemoryCollector>     m_memory;
    std::unique_ptr<OsCollector>         m_os;
    std::unique_ptr<StorageCollector>    m_storage;
    std::unique_ptr<NetworkCollector>    m_network;
    std::unique_ptr<GpuCollector>        m_gpu;
    std::unique_ptr<BoardCollector>      m_board;
    std::unique_ptr<SensorCollector>     m_sensors;
    std::unique_ptr<PrivilegedCollector> m_privileged;
};
