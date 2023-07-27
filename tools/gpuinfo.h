#ifndef NVML_INIT_FLAG_NO_GPUS
#include "nvml.h"
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

struct MemoryInfo
{
    float total_memory;
    float used_memory;
};

struct UtilizationInfo
{
    uint gpu_utilization;
    uint memory_utilization;
};

struct ProcessInfo
{
    int gpu_id;
    QHash<int, float> pid_memory;
};

struct GPUInfo
{
    QString gpu_name;
    MemoryInfo memory_info;
    UtilizationInfo utilization_info;
};

typedef QList<GPUInfo> GPUInfos;

class NvidiaGPU
{
private:
    uint gpu_device_count;
    nvmlReturn_t result;
    nvmlDevice_t device;

public:
    NvidiaGPU();
    ~NvidiaGPU();
    MemoryInfo getMemory(int gpuid);
    UtilizationInfo getUtilization(int gpuid);
    ProcessInfo getProcess(int gpuid);
    GPUInfos getAllGPUsInfo();
}
