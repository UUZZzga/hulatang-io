#include "hulatang/base/Log.hpp"
#include "hulatang/base/String.hpp"
#include "hulatang/io/TCPServer.hpp"

#include "hulatang/io/TCPConnection.hpp"

#include <utility>

using namespace hulatang::base;
using namespace hulatang::io;

void onConnection(const TCPConnectionPtr &conn)
{
    // if (conn->isConnected())
    // {
    //     conn->setTcpNoDelay(true);
    // }
}

void onMessage(const TCPConnectionPtr &conn, const Buf &buf)
{
    conn->send(buf);
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, "Usage: server <address> <port> <threads>\n");
    }
    else
    {
        Log::init();
        HLT_INFO("pid = {}, tid = {}", getpid(), to_string(std::this_thread::get_id()));

        const char *ip = argv[1];
        auto port = static_cast<uint16_t>(atoi(argv[2]));

        int threadCount = atoi(argv[3]);

        EventLoop loop;

        TCPServer server(&loop, ip, port);

        server.setConnectionCallback(onConnection);
        server.setMessageCallback(onMessage);

        if (threadCount > 1)
        {
            server.setThreadNum(threadCount);
        }

        server.start();
        loop.run();
    }
}
