#pragma once

#include <QObject>
#include <QMap>
#include <QPair>
#include <QByteArray>
#include <QSharedPointer>
#include <QJsonObject>
#include <QHash>
#include <QThread>
#include "jsonwebtoken/src/qjsonwebtoken.h"

class Worker: public QThread
{
    Q_OBJECT
public:
    template<typename Function, typename... Args>
    static Worker * newTask(Function &&f, Args &&... args)
    {
        using DecayedFunction = typename std::decay<Function>::type;
        auto threadFunction =
        [f = static_cast<DecayedFunction>(std::forward<Function>(f))](auto &&... largs) mutable -> void
        {
            (void)std::invoke(std::move(f), std::forward<decltype(largs)>(largs)...);
        };
        auto task = new Worker(std::async(std::launch::deferred,
                std::move(threadFunction),
                std::forward<Args>(args)...));
        return task;
    };
    ~Worker()=default;
    Worker(const Worker&)=delete;
    Worker& operator=(const Worker&)=delete;
    Worker(Worker&&)=default;
signals:
    void taskFinished();
private:
    Worker(std::future<void> &&task):_task(std::move(task)){};
    std::future<void> _task;
    void run() override
    {
        _task.get();
        emit taskFinished();
    };
};

class GlobalCommon : public QObject
{
    Q_OBJECT
public:
    static QMap<QString, QString> parseHeaders(const QList<QPair<QByteArray, QByteArray>> &headers);
    static QString hashPassword(const QString &password, const QString &salt);
    static std::tuple<QString, QString> generateSaltAndHash(const QString &password);
    static QJsonObject hashToJsonObject(const QHash<QString, QVariant> &hash);
    static QString generateRandomString(int length = 32);
    static QString objectToString(const QJsonObject &object);
    static QJsonObject stringToObject(const QByteArray &string);
    static QString getJwtToken(QSharedPointer<QJsonWebToken>, const QString &);
    template<typename F, typename... Args>
    static QSharedPointer<Worker> runTaskAtBackground(F&& f, Args&&... args){
        auto worker = QSharedPointer<Worker>::create(std::forward<F>(f), std::forward<Args>(args)...);
        worker->start();
        return worker;
    };
private:
    explicit GlobalCommon(QObject *parent = nullptr);
    GlobalCommon(const GlobalCommon&) = delete;
    GlobalCommon& operator=(const GlobalCommon&) = delete;
    GlobalCommon(GlobalCommon&&) = delete;
    static QSharedPointer<GlobalCommon> _instance;

};
