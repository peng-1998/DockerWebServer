#include "gpuinfo.h"

NvidiaGPU::NvidiaGPU()
{
    result = nvmlInit();
    result = nvmlDeviceGetCount(&gpu_device_count);
    // if (result != NVML_SUCCESS) // 难以被catch
    //     throw "Failed to initialize NVML" + std::string(nvmlErrorString(result));
    available = result == NVML_SUCCESS;
}

NvidiaGPU::~NvidiaGPU()
{
    result = nvmlShutdown();
    // if (result != NVML_SUCCESS) // 无法被catch
    //     throw "Failed to shutdown NVML." + std::string(nvmlErrorString(result));
}

MemoryInfo NvidiaGPU::getMemory(int gpuid)
{
    nvmlMemory_t memory;
    result = nvmlDeviceGetHandleByIndex(gpuid, &device);
    result = nvmlDeviceGetMemoryInfo(device, &memory);
    if (result != NVML_SUCCESS)
        throw "Failed to get handle for device." + std::string(nvmlErrorString(result));
    return { (float)memory.total / 1024 / 1024, (float)memory.used / 1024 / 1024 };
}

UtilizationInfo NvidiaGPU::getUtilization(int gpuid)
{
    nvmlUtilization_t utilization;
    result = nvmlDeviceGetHandleByIndex(gpuid, &device);
    result = nvmlDeviceGetUtilizationRates(device, &utilization);
    if (result != NVML_SUCCESS)
        throw "Failed to get handle for device." + std::string(nvmlErrorString(result));
    return {utilization.gpu, utilization.memory};
}

ProcessInfo NvidiaGPU::getProcess(int gpuid)
{
    nvmlProcessInfo_t process[MAX_PROCESS_PER_GPU];
    unsigned int infoCount = 0;
    result = nvmlDeviceGetHandleByIndex(gpuid, &device);
    result = nvmlDeviceGetComputeRunningProcesses_v3(device, &infoCount, process);
    if (result != NVML_SUCCESS)
        throw "Failed to get handle for device." + std::string(nvmlErrorString(result));
    auto process_info = ProcessInfo();
    process_info.gpu_id = gpuid;
    for (int i = 0; i < infoCount; ++i)
        process_info.pid_memory.insert(process[i].pid, process[i].usedGpuMemory / 1024 / 1024);
    return process_info;
}

GPUInfos NvidiaGPU::getAllGPUsInfo()
{
    GPUInfos all_gpus_info;
    char name[NVML_DEVICE_NAME_BUFFER_SIZE];
    nvmlMemory_t memory;
    nvmlUtilization_t utilization;
    try
    {
        for (int i = 0; i < gpu_device_count; i++)
        {
            result = nvmlDeviceGetHandleByIndex(i, &device);
            result = nvmlDeviceGetName(device, name, NVML_DEVICE_NAME_BUFFER_SIZE);
            result = nvmlDeviceGetMemoryInfo(device, &memory);
            result = nvmlDeviceGetUtilizationRates(device, &utilization);
            all_gpus_info << GPUInfo{QString(name), {(float)memory.total / 1024 / 1024, (float)memory.used / 1024 / 1024}, {utilization.gpu, utilization.memory}};
        }
    }
    catch (const char *err)
    {
        qCritical() << err;
        return GPUInfos();
    }

    return all_gpus_info;
}
