#include "waitqueue.h"

QSharedPointer<WaitQueue> WaitQueue::_instance = nullptr;

WaitQueue::WaitQueue(QObject *parent)
    : QObject{parent}, _status{}, _taskId{0}
{
    _schedulingStrategy = [](const MachineStatus &machine) -> std::optional<quint64> {
        return std::nullopt;
    };
}

std::optional<quint64> WaitQueue::defaultSchedulingStrategy(const MachineStatus &machine)
{
    auto &[waitingTasks, runningTasks, gpuStatus, isRunning] = machine;
    if (!isRunning or std::all_of(gpuStatus.begin(), gpuStatus.end(), [](bool b) { return b; }))
        return std::nullopt;
    auto availableGpuCount = std::count(gpuStatus.begin(), gpuStatus.end(), false);
    auto task = std::find_if(waitingTasks.begin(), waitingTasks.end(), 
    [availableGpuCount, &gpuStatus](const Task &task) { 
        return task.gpuCount <= availableGpuCount and (task.gpuIds.begin() == task.gpuIds.end() or std::all_of(task.gpuIds.begin(), task.gpuIds.end(), [&gpuStatus](int gpuId) { return !gpuStatus[gpuId]; }));
    });
    return task == waitingTasks.end() ? std::nullopt : std::optional<quint64>{task->id};
}

QSharedPointer<WaitQueue> WaitQueue::instance()
{
    if (_instance.isNull())
        _instance = QSharedPointer<WaitQueue>(new WaitQueue());
    return _instance;
}

quint64 WaitQueue::newTask(const int &userId, const QString &machineId, const QString &containerName, const QString &command, int duration, int gpuCount, const QList<int> &gpuIds)
{
    Task task{
        _taskId++,
        userId,
        duration,
        machineId,
        gpuIds,
        gpuCount,
        containerName,
        command,
        std::nullopt};
    _status[machineId].waitingTasks[task.id] = task;
    while(tryStartTask(machineId).has_value());
    return task.id;
}

void WaitQueue::newMachine(const QString &machineId, int gpuCount)
{
    if (_status.contains(machineId))
        return (_status[machineId].isRunning = true, void());
    _status[machineId] = MachineStatus{TaskSet{}, TaskSet{}, GpuStatus(gpuCount, false), true};
}

std::optional<quint64> WaitQueue::tryStartTask(const QString &machineId)
{
    auto &machine = _status[machineId];
    auto result = _schedulingStrategy(machine);
    if(result.has_value()){
        auto &task = machine.waitingTasks[result.value()];
        machine.runningTasks[task.id] = task;
        machine.waitingTasks.remove(task.id);
        auto &gpuStatus = machine.gpuStatus;
        if(task.gpuIds.isEmpty()){
            for(int i {0},count{0}; i < gpuStatus.size() && count < task.gpuCount ; ++i)
                if(!gpuStatus[i]){
                    gpuStatus[i] = true;
                    task.gpuIds.append(i);
                    ++count;
                }
        }else{
            for(auto gpuId : task.gpuIds)
                gpuStatus[gpuId] = true;
        }
        task.startTime = QDateTime::currentDateTime();
        emit taskStart(task);
    }
    return result;
}
