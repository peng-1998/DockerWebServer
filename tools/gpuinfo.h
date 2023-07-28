#pragma once
#ifndef NVML_INIT_FLAG_NO_GPUS
#include <nvml.h>
#endif
#ifndef MAP_H
#include <map>
#endif
#ifndef STRING_H
#include <string>
#endif
#define MAX_PROCESS_PER_GPU 32

#include <QMap>
#include <QString>
#include <QList>
#include <QHash>
#include <QJsonObject>
#include <QJsonArray>


struct MemoryInfo
{
    float total_memory;
    float used_memory;
    QJsonObject toJson() const
    {
        return QJsonObject{
            {"total", total_memory},
            {"used", used_memory}};
    }
};

struct UtilizationInfo
{
    uint gpu_utilization;
    uint memory_utilization;
    QJsonObject toJson() const
    {
        return QJsonObject{
            {"gpu", int(gpu_utilization)},
            {"memory", int(memory_utilization)}};
    }
};

struct ProcessInfo
{
    int gpu_id;
    QHash<int, float> pid_memory;
    QJsonObject toJson() const
    {
        auto pid_memory = QJsonObject();
        for (auto pid : this->pid_memory.keys())
            pid_memory.insert(QString::number(pid), this->pid_memory[pid]);
        return QJsonObject{
            {"gpu_id", gpu_id},
            {"pid_memory", pid_memory}};
    }
};

struct GPUInfo
{
    QString gpu_name;
    MemoryInfo memory_info;
    UtilizationInfo utilization_info;
    QJsonObject toJson() const
    {
        return QJsonObject{
            {"gpu_name", gpu_name},
            {"memory_info", memory_info.toJson()},
            {"utilization_info", utilization_info.toJson()}};
    }
};

typedef QList<GPUInfo> GPUInfos;

class NvidiaGPU
{
private:
    uint gpu_device_count;
    nvmlReturn_t result;
    nvmlDevice_t device;
    bool available;

public:
    NvidiaGPU();
    ~NvidiaGPU();
    MemoryInfo getMemory(int gpuid);
    UtilizationInfo getUtilization(int gpuid);
    ProcessInfo getProcess(int gpuid);
    GPUInfos getAllGPUsInfo();
    inline bool isVaild() const { return available; }
};
