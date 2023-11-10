#include "server/globaldata.hpp"
#include "server/globalevent.h"
#include "tools/globalconfig.hpp"
#include <QCoreApplication>

void global_init();

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    global_init();
    return a.exec();
}

void global_init()
{
    // 全局类初始化
    auto &globalConfig = GlobalConfig::instance();
    globalConfig.init("config.yaml");
    auto &globalData = GlobalData::instance();
    auto &globalEvent = GlobalEvent::instance();
    auto globalEventPtr = &globalEvent;

    // 创建tcp服务器
    GlobalData::instance().tcpServer.listen(
        QHostAddress::Any,
        globalConfig["TCP"]["port"].as<int>());
    // 设置tcp请求处理函数
    QObject::connect(
        &(globalData.tcpServer),
        &QTcpServer::newConnection,
        globalEventPtr,
        &GlobalEvent::onNewTcpConnection);
    // 设置心跳定时器
    QObject::connect(
        &(globalData.heartbeatTimer),
        &QTimer::timeout,
        &GlobalEvent::instance(),
        &GlobalEvent::onCheckHeartbeat);

    // 设置jwt
    globalData.jwt.setSecret(
        QString::fromStdString(
            globalConfig["JWT"]["secret"].as<std::string>()));
    globalData.jwt.setAlgorithmStr(
        QString::fromStdString(
            globalConfig["JWT"]["algorithm"].as<std::string>()));
    globalData.jwt.appendClaim(
        "iss",
        QString::fromStdString(
            globalConfig["JWT"]["iss"].as<std::string>()));

    // 设置websocket服务器
    globalData.wsServer.listen(
        QHostAddress::Any,
        globalConfig["WebSocket"]["port"].as<int>());
    // 设置websocket连接处理函数
    QObject::connect(
        &(globalData.wsServer),
        &QWebSocketServer::newConnection,
        globalEventPtr,
        &GlobalEvent::onWSNewConnection);

    // 设置http服务器
    globalData.httpServer.listen(
        QHostAddress::Any,
        globalConfig["Http"]["port"].as<int>());
    // 设置跨域
    globalData.httpServer.afterRequest(
        [](QHttpServerResponse &&resp)
        {
            resp.addHeader("Access-Control-Allow-Origin", "*");
            return std::move(resp);
        });
    // jwt装饰器
    using Method = QHttpServerRequest::Method;
    using StatusCode = QHttpServerResponder::StatusCode;
    auto sessionDecoratedRoute = [&globalData, &globalEvent](const QString &url, Method m, auto &&method)
    {
        globalData.httpServer.route(
            url, m,
            globalEvent.sessionDecorator(std::forward<decltype(method)>(method)));
    };
    // 设置路由
    globalData.httpServer.route("/api/auth/login", Method::Post, &GlobalEvent::onApiAuthLogin);
    globalData.httpServer.route("/api/auth/register", Method::Post, &GlobalEvent::onApiAuthRegister);
    sessionDecoratedRoute("/api/auth/logout", Method::Post, &GlobalEvent::onApiAuthLogout);
    sessionDecoratedRoute("/api/auth/session", Method::Get, &GlobalEvent::onApiAuthSession);
    sessionDecoratedRoute("/api/user/set_profile", Method::Post, &GlobalEvent::onApiUserSetProfile);
    sessionDecoratedRoute("/api/user/set_photo", Method::Post, &GlobalEvent::onApiUserSetPhoto);
    sessionDecoratedRoute("/api/user/get_user", Method::Get, GlobalEvent::onApiUserGetUser);
    sessionDecoratedRoute("/api/machines/info", Method::Get, &GlobalEvent::onApiMachinesInfo);
    sessionDecoratedRoute("/api/admin/all_users", Method::Get, &GlobalEvent::onApiAdminAllUsers);
    sessionDecoratedRoute("/api/admin/all_images", Method::Get, &GlobalEvent::onApiAdminAllImages);
    sessionDecoratedRoute("/api/admin/all_containers", Method::Get, &GlobalEvent::onApiAdminAllContainers);
    sessionDecoratedRoute("/api/task/cancel", Method::Post, &GlobalEvent::onApiTaskCancel);

    // 更改qt的日志输出
    qInstallMessageHandler(
        [](QtMsgType type, const QMessageLogContext &context, const QString &msg)
        {
            static Logger *const logger_ptr = &GlobalData::instance().logger;
            logger_ptr->write(type, context, msg);
        });
}
