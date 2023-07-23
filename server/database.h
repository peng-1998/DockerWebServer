#pragma once

#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QSharedPointer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>
class DataBase : public QObject
{
    Q_OBJECT
public:
    static QSharedPointer<DataBase> instance();
    ~DataBase() = default;
    void insertUser(const QString &account, const QString &password, const QString &salt, QString nickname = "", QString email = "", QString phone = "", QString photo = "");
    std::optional<QHash<QString, QVariant>> getUser(const int id, const std::optional<QStringList> &select = std::nullopt);
    std::optional<QHash<QString, QVariant>> getUser(const QString &account, const std::optional<QStringList> &select = std::nullopt);
    QList<QHash<QString, QVariant>> getUser(QList<QPair<QString, QVariant>> &where, const std::optional<QStringList> &select = std::nullopt);
    QList<QHash<QString, QVariant>> getUserAll(const std::optional<QStringList> &select = std::nullopt);
    void updateUser(const int id, const QList<QPair<QString, QVariant>> &set);
    void updateUser(const QString &account, const QList<QPair<QString, QVariant>> &set);
    void deleteUser(const int id);
    void deleteUser(const QString &account);
    bool containsUser(const QString &account);

    void insertImage(const QString &imagename, const QString &showname, const QJsonObject &init_args, const QString &description);
    std::optional<QHash<QString, QVariant>> getImage(const int id, const std::optional<QStringList> &select = std::nullopt);
    std::optional<QHash<QString, QVariant>> getImage(const QString &imagename, const std::optional<QStringList> &select = std::nullopt);
    QList<QHash<QString, QVariant>> getImageAll(const std::optional<QStringList> &select = std::nullopt);
    void updateImage(const int id, const QList<QPair<QString, QVariant>> &set);
    void updateImage(const QString &imagename, const QList<QPair<QString, QVariant>> &set);
    void deleteImage(const int id);
    void deleteImage(const QString &imagename);
    bool containsImage(const QString &imagename);

    void insertContainer(const QString &containername, const QString &showname, const int imageid, const int userid, const QString &machineId, const QJsonArray portlist, const bool running = false);
    void insertContainer(const QString &containername, const QString &showname, const QString &imagename, const QString &account, const QString &machineId, const QJsonArray portlist, const bool running = false);
    std::optional<QHash<QString, QVariant>> getContainer(const int id, const std::optional<QStringList> &select = std::nullopt);
    std::optional<QHash<QString, QVariant>> getContainer(const QString &containername, const std::optional<QStringList> &select = std::nullopt);
    QList<QHash<QString, QVariant>> getContainerAll(const std::optional<QStringList> &select = std::nullopt);
    QList<QHash<QString, QVariant>> getContainerUser(const QString &account, const std::optional<QStringList> &select = std::nullopt);
    QList<QHash<QString, QVariant>> getContainerUser(const int id, const std::optional<QStringList> &select = std::nullopt);
    QList<QHash<QString, QVariant>> getContainerImage(const QString &image, const std::optional<QStringList> &select = std::nullopt);
    QList<QHash<QString, QVariant>> getContainerImage(const int id, const std::optional<QStringList> &select = std::nullopt);
    QList<QHash<QString, QVariant>> getContainerMachine(const QString &machineId, const std::optional<QStringList> &select = std::nullopt);
    void updateContainer(const int id, const QList<QPair<QString, QVariant>> &set);
    void updateContainer(const QString &containername, const QList<QPair<QString, QVariant>> &set);
    void updateContainerRunning(const QString &containername, const bool running = false);
    void deleteContainer(const int id);
    void deleteContainer(const QString &containername);

    void insertMachine(const QString &id, const QString &ip, const QJsonObject &gpu, const QJsonObject &cpu, const QJsonObject &memory, const QJsonObject &disk, const bool online = true);
    std::optional<QHash<QString, QVariant>> getMachine(const QString &id, const std::optional<QStringList> &select = std::nullopt);
    QList<QHash<QString, QVariant>> getMachineAll(const std::optional<QStringList> &select = std::nullopt);
    void updateMachine(const QString &id, const QList<QPair<QString, QVariant>> &set);
    void deleteMachine(const QString &id);
    bool containsMachine(const QString &id);

signals:

private:
    explicit DataBase(QObject *parent = nullptr);

    DataBase(const DataBase &) = delete;
    DataBase &operator=(const DataBase &) = delete;
    DataBase(DataBase &&) = delete;
    QSqlDatabase _db;
    QSqlQuery _query;
    static QSharedPointer<DataBase> _instance;
    void creatTable();
    static QStringList _user_column;
    static QStringList _image_column;
    static QStringList _container_column;
    static QStringList _machine_column;
    static QString formatWhere(const QList<QPair<QString, QVariant>> &where);
    static QString formatSet(const QList<QPair<QString, QVariant>> &set);
    static QHash<QString, QVariant> formatResult(const QSqlQuery &query, const QStringList &select);
    static QList<QPair<QString, QVariant>> formatInput(const QList<QPair<QString, QVariant>> &input);
};
