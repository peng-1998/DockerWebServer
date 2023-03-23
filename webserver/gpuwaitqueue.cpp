#include "gpuwaitqueue.h"

TaskWaitQueue::TaskWaitQueue(MachinePool *machinePool, const QSettings *settings) {
    _machinePool = machinePool;
    _settings = settings;
    connect(_machinePool, &MachinePool::newMachine, this, &TaskWaitQueue::newMachine);
    connect(_machinePool, &MachinePool::delMachine, this, &TaskWaitQueue::delMachine);
}

void TaskWaitQueue::newTask(QString machine, QString user, int duration, QList<int> gpuid, int gpucount, QString reason, QString container, QString cmd) {
    Task *task = nullptr;
    if (gpucount == 0)
        task = new Task(gpuid, user, duration, reason, container, cmd);
    else
        task = new Task(gpucount, user, duration, reason, container, cmd);
    this->_queues[machine].queue.append(task);
    this->tryStartTask(machine);
}

void TaskWaitQueue::tryStartTask(const QString &machine) {
    auto &queueInfo = this->_queues[machine];
    auto &queue = queueInfo.queue;
    auto now = QTime::currentTime();

    if (queue.isEmpty())
        return;

    auto canStartTasks = this->findCanStartTasks(queueInfo);
    if (canStartTasks.isEmpty())
        return;

    auto responseRatioOrder = this->responseRatioOrder(canStartTasks, queueInfo);
    if (now.hour() < 8 || now.hour() > 22) {
        // 删除没有cmd的任务
        for (auto it = responseRatioOrder.begin(); it != responseRatioOrder.end();) {
            if (queue.at(it->first)->cmd.isEmpty()) {
                it = responseRatioOrder.erase(it);
            } else {
                ++it;
            }
        }
    }
    if (responseRatioOrder.isEmpty())
        return;

    auto index = responseRatioOrder.first().first;
    auto task = queue.takeAt(index);
    this->_runningTasks[machine].append(task);
    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::hours(task->duration)).count();
    QTimer::singleShot(durationMs, this, [=]() {
        this->_runningTasks[machine].removeOne(task);
        emit this->taskFinished(task);
        this->tryStartTask(machine);
    });

    emit this->taskStarted(task);
}

QString TaskWaitQueue::currentUser(QString machine, int gpuid) const {
    auto &runningTasks = this->_runningTasks[machine];
    for (auto task : runningTasks) {
        if (task->gpus.contains(gpuid))
            return task->user;
    }
    return QString();
}

int TaskWaitQueue::waitTime(QString machine, int gpuid) {
}

TaskWaitQueue::Task TaskWaitQueue::nextTask(QString machine) {
}

bool TaskWaitQueue::canStartTask(const QSet<int> &availableGpus, const Task *task) {
    return task->gpuCount == -1 ? availableGpus.contains(task->gpus) : availableGpus.size() >= task->gpuCount;
}

QList<int> TaskWaitQueue::findCanStartTasks(MachineWaitQueue &mqueue) {
    QList<Task *> &queue = mqueue.queue;
    QSet<int> &availableGpus = mqueue.availableGpus;
    QList<int> canStartTasks;
    for (int i = 0; i < queue.size(); i++) {
        if (canStartTask(availableGpus, queue.at(i)))
            canStartTasks.append(i);
    }
    return canStartTasks;
}

QList<QPair<int, float>> TaskWaitQueue::responseRatioOrder(QList<int> indices, MachineWaitQueue &mqueue) {
    QList<QPair<int, float>> result;
    QList<Task *> &queue = mqueue.queue;
    QDateTime currentDateTime = QDateTime::currentDateTime();
    for (auto index : indices) {
        Task *task = queue.at(index);
        int waitTime = task->submitTime.secsTo(currentDateTime);
        int taskTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours(task->duration)).count();
        float ratio = (float)(waitTime + taskTime) / taskTime;
        result.append(qMakePair(index, ratio));
    }
    std::sort(result.begin(), result.end(), [](const QPair<int, float> &a, const QPair<int, float> &b) {
        return a.second > b.second;
    });
    return result;
}

void TaskWaitQueue::delMachine(QString machine) {
    this->_queues.remove(machine);
    this->_runningTasks.remove(machine);
}

void TaskWaitQueue::newMachine(QString machine) {
    this->_queues.insert(machine, MachineWaitQueue());
    this->_runningTasks.insert(machine, QList<Task *>());
    auto m = this->_machinePool->pool().value(machine);
    for (auto gpu : m->gpus()) {
        this->_queues[machine].gpus.insert(gpu.gpuid());
    }
}