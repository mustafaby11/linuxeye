// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye – src/core/systeminfo.cpp
#include "systeminfo.h"

#include "cpucollector.h"
#include "memorycollector.h"
#include "oscollector.h"
#include "storagecollector.h"
#include "networkcollector.h"
#include "gpucollector.h"
#include "boardcollector.h"
#include "sensorcollector.h"
#include "privilegedcollector.h"

SystemInfo* SystemInfo::instance()
{
    static SystemInfo s_instance;
    return &s_instance;
}

SystemInfo::SystemInfo(QObject *parent)
    : QObject(parent)
    , m_cpu(std::make_unique<CpuCollector>())
    , m_memory(std::make_unique<MemoryCollector>())
    , m_os(std::make_unique<OsCollector>())
    , m_storage(std::make_unique<StorageCollector>())
    , m_network(std::make_unique<NetworkCollector>())
    , m_gpu(std::make_unique<GpuCollector>())
    , m_board(std::make_unique<BoardCollector>())
    , m_sensors(std::make_unique<SensorCollector>())
    , m_privileged(std::make_unique<PrivilegedCollector>())
{
    // Forward the async result signal
    connect(m_privileged.get(), &PrivilegedCollector::finished,
            this, &SystemInfo::privilegedDataReady);
}

void SystemInfo::collectAll()
{
    m_cpu->collect();
    m_memory->collect();
    m_os->collect();
    m_storage->collect();
    m_network->collect();
    m_gpu->collect();
    m_board->collect();
    m_sensors->collect();
    emit dataRefreshed();
}

void SystemInfo::collectPrivileged()
{
    m_privileged->start();
}
