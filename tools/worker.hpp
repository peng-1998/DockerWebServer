#pragma once

#include <QByteArray>
#include <QHash>
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QSharedPointer>
#include <QThread>

class Worker : public QThread
{
    Q_OBJECT
public:
    template <typename Function, typename... Args>
    static Worker *newTask(Function &&f, Args &&...args)
    {
        using DecayedFunction = typename std::decay<Function>::type;
        auto threadFunction =
            [f = static_cast<DecayedFunction>(std::forward<Function>(f))](
                auto &&...largs) mutable -> void
        {
            (void)std::invoke(std::move(f), std::forward<decltype(largs)>(largs)...);
        };
        return new Worker(std::async(std::launch::deferred,
                                     std::move(threadFunction),
                                     std::forward<Args>(args)...));
    };
    ~Worker() = default;
    Worker(const Worker &) = delete;
    Worker &operator=(const Worker &) = delete;
    Worker(Worker &&) = default;

signals:
    void taskFinished();

private:
    Worker(std::future<void> &&task) : _task(std::move(task)){};
    std::future<void> _task;
    void run() override
    {
        _task.get();
        emit taskFinished();
        quit();
        wait();
        deleteLater();
    };
};