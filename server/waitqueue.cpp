#include "waitqueue.h"


WaitQueue::WaitQueue(QObject *parent)
    : QObject{parent}, _status{}, _taskId{0}
{
    _schedulingStrategy = std::bind(&WaitQueue::defaultSchedulingStrategy, this, std::placeholders::_1);
    connect(this, &WaitQueue::taskStop, this, &WaitQueue::onTaskStopped);
}

std::optional<quint64> WaitQueue::defaultSchedulingStrategy(const MachineStatus &machine)
{
    auto &[waitingTasks, runningTasks, gpuStatus, isRunning] = machine;
    if (!isRunning or std::all_of(gpuStatus.begin(), gpuStatus.end(), [](bool b)
                                  { return b; }))
        return std::nullopt;
    auto availableGpuCount = std::count(gpuStatus.begin(), gpuStatus.end(), false);
    auto task = std::find_if(waitingTasks.begin(), waitingTasks.end(),
                             [availableGpuCount, &gpuStatus](const Task &task)
                             {return task.gpuCount <= availableGpuCount and 
                             (task.gpuIds.begin() == task.gpuIds.end() or 
                             std::all_of(task.gpuIds.begin(), task.gpuIds.end(), [&gpuStatus](int gpuId){ return !gpuStatus[gpuId]; }));
                             });
    return task == waitingTasks.end() ? std::nullopt : std::optional<quint64>{task->id};
}

void WaitQueue::_stopTask(quint64 taskId, const QString &machineId)
{
    auto &machine = _status[machineId];
    auto &task = machine.runningTasks[taskId];
    machine.runningTasks.remove(taskId);
    auto &gpuStatus = machine.gpuStatus;
    for (auto gpuId : task.gpuIds)
        gpuStatus[gpuId] = false;
    emit taskStop(task);
}

void WaitQueue::_cancelTask(quint64 taskId, const QString &machineId)
{
    _status[machineId].waitingTasks.remove(taskId);
}

WaitQueue& WaitQueue::instance()
{
    static WaitQueue _instance;
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
    while (tryStartTask(machineId).has_value());
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
    if (result.has_value())
    {
        auto &task = machine.waitingTasks[result.value()];
        machine.runningTasks[task.id] = task;
        machine.waitingTasks.remove(task.id);
        auto &gpuStatus = machine.gpuStatus;
        if (task.gpuIds.isEmpty())
            for (int i{0}, count{0}; i < gpuStatus.size() && count < task.gpuCount; ++i)
                if (!gpuStatus[i])
                {
                    gpuStatus[i] = true;
                    task.gpuIds.append(i);
                    ++count;
                }
        else
            for (auto gpuId : task.gpuIds)
                gpuStatus[gpuId] = true;
        task.startTime = QDateTime::currentDateTime();
        emit taskStart(task);
    }
    return result;
}

void WaitQueue::onTaskStopped(Task task)
{
    QTimer::singleShot(1000, [this,task]() //等待一段时间，用于gpu服务器清理资源
    {this->tryStartTask(task.machineId);});
}

void WaitQueue::cancelTask(quint64 taskId, QString machineId, std::optional<bool> running)
{
    if (machineId.isEmpty())
    {
        auto item = std::find_if(
            _status.begin(),
            _status.end(),
            [taskId](const auto &machine)
            { return machine.waitingTasks.contains(taskId) or machine.runningTasks.contains(taskId); });
        machineId = item.key();
    }
    auto running_ = false;
    if (running.has_value())
        running_ = running.value();
    else
        running_ = _status[machineId].runningTasks.contains(taskId);
    if (running_)
        _stopTask(taskId, machineId);
    else
        _cancelTask(taskId, machineId);
}

QList<Task> WaitQueue::getMachineTasks(const QString &machineId)
{
    auto &machine = _status[machineId];
    return machine.waitingTasks.values() + machine.runningTasks.values();
}

QList<Task> WaitQueue::getUserTasks(const int &userId)
{
    auto tasks = QList<Task>{};
    for (auto &machine : _status)
    {
        std::copy_if(
            machine.waitingTasks.begin(),
            machine.waitingTasks.end(),
            std::back_inserter(tasks),
            [userId](const Task &task)
            { return task.userId == userId; });
        std::copy_if(
            machine.runningTasks.begin(),
            machine.runningTasks.end(),
            std::back_inserter(tasks),
            [userId](const Task &task)
            { return task.userId == userId; });
    }
    return tasks;
}

void WaitQueue::setSchedulingStrategy(SchedulingStrategy &strategy)
{
    _schedulingStrategy = strategy;
}
