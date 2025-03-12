#pragma once
#include <httplib.h>
#include <QCoreApplication>
#include <QScopedPointer>

namespace httplib { class Server; }

class Server : public QCoreApplication
{
    Q_OBJECT
public:
    Server(int argc, char* argv[]);
    ~Server();

    bool init();
    int run();

private:
    void initHttpServer(int port);
    void onChat(const httplib::Request& req, httplib::Response& rsp);

private:
    QScopedPointer<httplib::Server> httpServer;
    class Dataset* dataset = nullptr;
};
