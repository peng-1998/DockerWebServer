#include <gpuinfo.h>

NvidiaGPU::NvidiaGPU()
{
    result = nvmlInit();
    result = nvmlDeviceGetCount(&gpu_device_count);
    if (NVML_SUCCESS != result0)
    {
        std::cout << "Failed to initialize NVML:" << nvmlErrorString(result0) << "\n"
                  << endl;
        exit(1);
    }
}
NvidiaGPU::~NvidiaGPU()
{
    result = nvmlShutdown();
    if (NVML_SUCCESS != result0)
    {
        std::cout << "Failed to shutdown NVML:" << nvmlErrorString(result0) << "\n"
                  << endl;
        exit(1);
    }
}
NvidiaGPU::getMemory(int gpuid)
{
    nvmiMemory_t memory;
    result = nvmlDeviceGetHandleByIndex(gpuid, &device);
    result = nvmlDeviceGetMemoryInfo(device, &memory);
    if (NVML_SUCCESS != result0)
    {
        std::cout << "Failed to get handle for device:" << nvmlErrorString(result0) << "\n"
                  << endl;
        exit(1);
    }
    return { memory.total / 1024 / 1024, memory.used / 1024 / 1024 }
}
NvidiaGPU::getUtilization(int gpuid)
{
    nvmlUtilization_t utilization;
    result = nvmlDeviceGetHandleByIndex(gpuid, &device);
    result = nvmlDeviceGetUtilizationRates(device, &utilization);
    if (NVML_SUCCESS != result0)
    {
        std::cout << "Failed to get handle for device:" << nvmlErrorString(result0) << "\n"
                  << endl;
        exit(1);
    }
    return {utilization.gpu, utilization.memory};
}
NvidiaGPU::getProcess(int gpuid)
{
    nvmlProcessInfo_t process[MAX_PROCESS_PER_GPU];
    unsigned int infoCount = 0;
    result = nvmlDeviceGetHandleByIndex(gpuid, &device);
    result = nvmlDeviceGetComputeRunningProcesses_v3(device, &infoCount, &process);
    if (NVML_SUCCESS != result0)
    {
        std::cout << "Failed to get handle for device:" << nvmlErrorString(result0) << "\n"
                  << endl;
        exit(1);
    }
    return {gpuid, map<int, float>(process, process + infoCount, [](nvmlProcessInfo_t p)
                                   { return {p.pid, p.usedGpuMemory / 1024 / 1024}; })};
}
NvidiaGPU::getAllGPUsInfo()
{
    std::map<int, GPUInfo> all_gpus_info;
    char name[NVML_DEVICE_NAME_BUFFER_SIZE];
    nvmlMemory_t memory;
    nvmlUtilization_t utilization;
    for (int i = 0; i < gpu_device_count; i++)
    {
        result = nvmlDeviceGetHandleByIndex(i, &device);
        result = nvmlDeviceGetName(device, &name, NVML_DEVICE_NAME_BUFFER_SIZE);
        result = nvmlDeviceGetMemoryInfo(device, &memory);
        result = nvmlDeviceGetUtilizationRates(device, &utilization);
        all_gpus_info[i] = GPUInfo(name, {memory.total / 1024 / 1024, memory.used / 1024 / 1024}, {utilization.gpu, utilization.memory});
    }
    return all_gpus_info;
}
NvidiaGPU::jsonfy()
{
}