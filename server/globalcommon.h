#pragma once

#include <QObject>
#include <QMap>
#include <QPair>
#include <QByteArray>
#include <QSharedPointer>
#include <QJsonObject>
#include <QHash>
#include <QThread>
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
    static QJsonObject stringToObject(const QbyteArray &string);
    template<typename F, typename... Args>
    static QSharedPointer<Woker> runTaskAtBackground(F&& f, Args&&... args){
        auto worker = QSharedPointer<Worker>::create(std::forward<F>(f), std::forward<Args>(args)...);
        worker->start();
        return worker;
    }

    class Worker: public QThread
    {
    public:
        template<typename F, typename... Args>
        Worker(F&& f, Args&&... args)
        {
            _task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        }
        ~Worker()=default;
        void run()
        {
            _task();
            emit finished();
        }
    public signals:
        void finished();
    private:
        std::function<void()> _task;
    };
    
    
    

private:
    explicit GlobalCommon(QObject *parent = nullptr);
    GlobalCommon(const GlobalCommon&) = delete;
    GlobalCommon& operator=(const GlobalCommon&) = delete;
    GlobalCommon(GlobalCommon&&) = delete;
    static QSharedPointer<GlobalCommon> _instance;

};
