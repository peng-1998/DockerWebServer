#include <QCoreApplication>
#include "server/webserver.h"
#include "common/globalconfig.h"

void global_init();

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    global_init();
    return a.exec();
}


void global_init(){
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
        globalEvent_ptr,
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
        globalEvent_ptr, 
        &GlobalEvent::onWSNewConnection);


    // 设置http服务器
    globalData.httpServer.listen(
        QHostAddress::Any,
        globalConfig["Http"]["port"].as<int>());
    // 设置跨域
    globalData.httpServer.afterRequest(
        [](QHttpServerResponse &&resp) { 
            resp.addHeader("Access-Control-Allow-Origin", "*");
            return resp;
        });
    // jwt装饰器
    
    auto jutDecoratedRoute = [&globalData](auto &&...args, auto && method) {
        globalData.httpServer.route(
            std::forward<decltype(args)>(args)..., 
            std::forward<decltype(method)>(method));
    };
    // 设置路由
    using Method = QHttpServer::Method;
    using StatusCode = QHttpServerResponder::StatusCode;
    globalData.httpServer.route("/api/auth/login", Method::Post, &GlobalEvent::onApiAuthLogin);
    globalData.httpServer.route("/api/auth/register", Method::Post, &GlobalEvent::onApiAuthRegister);
    jutDecoratedRoute("/api/auth/logout", Method::Post, &GlobalEvent::onApiAuthLogout);
    jutDecoratedRoute("/api/auth/session", Method::Get, &GlobalEvent::onApiAuthSession);
    jutDecoratedRoute("/api/user/set_profile", Method::Post, &GlobalEvent::onApiUserSetProfile);
    jutDecoratedRoute("/api/user/set_photo", Method::Post, &GlobalEvent::onApiUserSetPhoto);
    jutDecoratedRoute("/api/user/get_user", Method::Get, GlobalEvent::onApiUserGetUser);
    jutDecoratedRoute("/api/machines/info", Method::Get, &GlobalEvent::onApiMachinesInfo);
    jutDecoratedRoute("/api/admin/all_users", Method::Get, &GlobalEvent::onApiAdminAllUsers);
    jutDecoratedRoute("/api/admin/all_images", Method::Get, &GlobalEvent::onApiAdminAllImages);
    jutDecoratedRoute("/api/admin/all_containers", Method::Get, &GlobalEvent::onApiAdminAllContainers);
    jutDecoratedRoute("/api/task/cancel", Method::Post, &GlobalEvent::onApiTaskCancel);
}

