#pragma once
#include <QDateTime>
#include <QHash>
#include <QList>
#include <QObject>
#include <QSharedPointer>
#include <QString>


struct Task
{
    quint64 id;
    int userId;
    int duration;
    QString machineId;
    QList<int> gpuIds;
    int gpuCount;
    QString containerName;
    QString command;
    std::optional<QDateTime> startTime;
};

typedef QList<bool> GpuStatus;
typedef QHash<int, Task> TaskSet;

struct MachineStatus
{
    TaskSet waitingTasks;
    TaskSet runningTasks;
    GpuStatus gpuStatus;
    bool isRunning;
};

class WaitQueue : public QObject
{
    Q_OBJECT
private:
    WaitQueue(QObject *parent = nullptr);
    static QSharedPointer<WaitQueue> _instance;
    QHash<QString, MachineStatus> _status;
    quint64 _taskId;
    std::function<std::optional<quint64>(const MachineStatus &)> _schedulingStrategy;
    std::optional<quint64> defaultSchedulingStrategy(const MachineStatus &machine);

signals:
    void taskStart(Task task);

public:
    ~WaitQueue() = default;

    static QSharedPointer<WaitQueue> instance();
    quint64 newTask(const int &userId, const QString &machineId, const QString &containerName, const QString &command, int duration, int gpuCount,  const QList<int> &gpuIds = QList<int>());
    void newMachine(const QString &machineId, int gpuCount);
    std::optional<quint64> tryStartTask(const QString &machineId);
};
