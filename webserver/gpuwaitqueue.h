#ifndef GPUWAITQUEUE_H
#define GPUWAITQUEUE_H

#include <QHash>
#include <QDateTime>
#include <QQueue>
#include "machinepool.h"
#include <QTimer>
#include <QSettings>
class TaskWaitQueue : public QObject {
public:
    TaskWaitQueue(MachinePool *machinePool, const QSettings *settings);
    class Task {
    public:
        Task(QList<int> gpuid, QString user, int duration, QString reason = "", QString container = "", QString cmd = "") {
            this->gpus = QSet<int>(gpuid.begin(), gpuid.end());
            this->duration = duration;
            this->user = user;
            this->cmd = cmd;
            this->container = container;
            this->reason = reason;
            this->submitTime = QDateTime::currentDateTime();
            this->gpuCount = -1;
        }
        Task(int gpuCount, QString user, int duration, QString reason = "", QString container = "", QString cmd = "") {
            this->gpuCount = gpuCount;
            this->duration = duration;
            this->user = user;
            this->cmd = cmd;
            this->container = container;
            this->reason = reason;
            this->submitTime = QDateTime::currentDateTime();
        }
        QSet<int> gpus;
        int duration;
        QString user;
        QString cmd;
        QString container;
        QString reason;
        QDateTime startTime;
        QDateTime endTime;
        QDateTime submitTime;
        int gpuCount;
    };
    void newTask(QString machine, QString user, int duration, QList<int> gpuid = QList<int>(), int gpucount = 0, QString reason = "", QString container = "", QString cmd = "");
    void tryStartTask(const QString &machine);
    void destroyTask(QString machine, int taskid);

    QString currentUser(QString machine, int gpuid) const;
    int waitTime(QString machine, int gpuid);
    TaskWaitQueue::Task nextTask(QString machine);

signals:
    void taskFinished(Task *task);
    void taskStarted(Task *task);

private:
    class MachineWaitQueue {
    public:
        QList<Task *> queue;
        QSet<int> gpus;
        QSet<int> availableGpus;
    };
    const QSettings *_settings;
    MachinePool *_machinePool;
    QHash<QString, MachineWaitQueue> _queues;
    QHash<QString, QList<Task *>> _runningTasks;

    bool canStartTask(const QSet<int> &availableGpus, const Task *task);
    QList<int> findCanStartTasks(MachineWaitQueue &mqueue);
    QList<QPair<int, float>> responseRatioOrder(QList<int> indices, MachineWaitQueue &mqueue);

private slots:
    void newMachine(QString machine);
    void delMachine(QString machine);
};

#endif // GPUWAITQUEUE_H
