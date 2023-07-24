#ifndef NVML_INIT_FLAG_NO_GPUS
#include <hwloc/nvml.h>
#endif
#ifndef MAP_H
#include <map>
#endif
#ifndef STRING_H
#include <string>
#endif
#define MAX_PROCESS_PER_GPU 32

struct MemoryInfo
{
    float total_memory;
    float used_memory;
};

struct UtilizationInfo
{
    unsigned int gpu_utilization;
    unsigned int memory_utilization;
};

struct ProcessInfo
{
    int gpu_id;
    std::map<int, float> pid_memory;
};

struct GPUInfo
{
    /* data */
    std::string gpu_name;
    MemoryInfo memory_info;
    UtilizationInfo utilization_info;
};

class NvidiaGPU
{
private:
    unsigned int gpu_device_count;
    nvmlReturn_t result;
    nvmlDevice_t device;

public:
    NvidiaGPU();
    ~NvidiaGPU();
    MemoryInfo getMemory(int gpuid);
    UtilizationInfo getUtilization(int gpuid);
    ProcessInfo getProcess(int gpuid);
    std::map getAllGPUsInfo();
    jsonfy();
}
