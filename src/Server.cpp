#include "Server.h"
#include "Logger.h"
#include "Dataset.h"
#include <QTimer>
#include <httplib.h>

Server::Server(int argc, char* argv[])
    : QCoreApplication(argc, argv)
    , httpServer(new httplib::Server())
    , dataset(new Dataset(this))
{
}

Server::~Server()
{
    if (!httpServer.isNull()) {
        httpServer->stop();
    }
}

bool Server::init()
{
    setupHttpServer();
    dataset->loadConfig();
    return true;
}

int Server::run()
{
    if (!init()) {
        LOG(err) << "Server initialization failed";
        return -1;
    }

    LOG(info) << "Server started";
    return exec();
}

void Server::setupHttpServer()
{
    // 设置基本路由
    httpServer->Get("/", [this](const httplib::Request&, httplib::Response& rsp) {
        rsp.set_content("Hello from nativeRAG Server!", "text/plain");
    });

    httpServer->Post("/api/chat", [this](const httplib::Request& req, httplib::Response& rsp) {
        onChat(req, rsp);
    });

    // 监听所有网络接口的56789端口
    QTimer::singleShot(0, this, [this]() {
        if (!httpServer->listen("0.0.0.0", 56789)) {
            LOG(err) << "Failed to start HTTP server on port 56789";
            quit();
        } else {
            LOG(info) << "HTTP server listening on 0.0.0.0:56789";
        }
    });
}

void Server::onChat(const httplib::Request& req, httplib::Response& rsp)
{
}
