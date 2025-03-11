#include "Server.h"
#include "Logger.h"

int main(int argc, char* argv[])
{
    Logger::instance()->init("nativeRAG-Server");

    Server server(argc, argv);
    return server.run();
}
