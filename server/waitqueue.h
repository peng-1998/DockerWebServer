#pragma once
#include <QDateTime>
#include <QHash>
#include <QList>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QTimer>

struct Task
{
    int id;
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

typedef std::function<std::optional<int>(const MachineStatus &)> SchedulingStrategy;

class WaitQueue : public QObject
{
    Q_OBJECT
private:
    
    QHash<QString, MachineStatus> _status;
    int _taskId;
    SchedulingStrategy _schedulingStrategy;
    std::optional<int> defaultSchedulingStrategy(const MachineStatus &machine);
    void _stopTask(int taskId, const QString &machineId);
    void _cancelTask(int taskId, const QString &machineId);
    std::optional<int> tryStartTask(const QString &machineId);

public slots:
    void onTaskStopped(Task task);

public:
    WaitQueue(QObject *parent = nullptr);
    ~WaitQueue() = default;

    static WaitQueue& instance();
    int newTask(const int &userId, const QString &machineId, const QString &containerName, const QString &command, int duration, int gpuCount,  const QList<int> &gpuIds = QList<int>());
    void newMachine(const QString &machineId, int gpuCount);
    
    void cancelTask(int taskId, QString machineId = "", std::optional<bool> running = std::nullopt);
    QList<Task> getMachineTasks(const QString &machineId);
    QList<Task> getUserTasks(const int &userId);
    void setSchedulingStrategy(SchedulingStrategy &strategy);
signals:
    void taskStart(Task task);
    void taskStop(Task task);
};
