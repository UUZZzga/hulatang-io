#include "hulatang/base/Log.hpp"
#include "hulatang/base/String.hpp"
#include "hulatang/io/EventLoopThreadPool.hpp"
#include "hulatang/io/TCPClient.hpp"

#include <atomic>
#include <chrono>

#include <cstdio>
#include <functional>
#include <string>

using namespace hulatang::base;
using namespace hulatang::io;

class Client;

class Session
{
public:
    Session(EventLoop *loop, const InetAddress &serverAddr, const std::string &name, Client *owner)
        : client_(loop, InetAddress::copyFromNative(serverAddr.getSockaddr(), serverAddr.sockaddrLength()))
        , owner_(owner)
        , bytesRead_(0)
        , bytesWritten_(0)
        , messagesRead_(0)
    {
        client_.setConnectionCallback(std::bind(&Session::onConnection, this, std::placeholders::_1));
        client_.setMessageCallback(std::bind(&Session::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    }

    void start()
    {
        client_.connect();
    }

    void stop()
    {
        client_.disconnect();
    }

    int64_t bytesRead() const
    {
        return bytesRead_;
    }

    int64_t messagesRead() const
    {
        return messagesRead_;
    }

private:
    void onConnection(const TCPConnectionPtr &conn);

    void onMessage(const TCPConnectionPtr &conn, Buffer *buf)
    {
        ++messagesRead_;
        bytesRead_ += buf->size();
        bytesWritten_ += buf->size();
        conn->send({const_cast<char *>(buf->data()), buf->size()});
        buf->retrieveAll();
    }

    TCPClient client_;
    Client *owner_;
    int64_t bytesRead_;
    int64_t bytesWritten_;
    int64_t messagesRead_;
};

class Client
{
public:
    Client(EventLoop *loop, const InetAddress &serverAddr, int blockSize, int sessionCount, int timeout, int threadCount)
        : loop_(loop)
        , threadPool_(loop, "pingpong-client")
        , sessionCount_(sessionCount)
        , timeout_(timeout)
    {
        loop->runAfter(std::chrono::seconds(timeout), [this] { handleTimeout(); });
        if (threadCount > 1)
        {
            threadPool_.setThreadNum(threadCount);
        }
        threadPool_.start();

        for (int i = 0; i < blockSize; ++i)
        {
            message_.push_back(static_cast<char>(i % 128));
        }

        for (int i = 0; i < sessionCount; ++i)
        {
            char buf[32];
            snprintf(buf, sizeof buf, "C%05d", i);
            Session *session = new Session(threadPool_.getNextLoop(), serverAddr, buf, this);
            session->start();
            sessions_.emplace_back(session);
        }
    }

    const std::string &message() const
    {
        return message_;
    }

    void onConnect()
    {
        if (numConnected_.fetch_add(1) == sessionCount_)
        {
            HLT_WARN("all connected");
        }
    }

    void onDisconnect(const TCPConnectionPtr &conn)
    {
        if (numConnected_.fetch_sub(1) == 1)
        {
            HLT_WARN("all disconnected");

            int64_t totalBytesRead = 0;
            int64_t totalMessagesRead = 0;
            for (const auto &session : sessions_)
            {
                totalBytesRead += session->bytesRead();
                totalMessagesRead += session->messagesRead();
            }
            HLT_WARN("{} total bytes read", totalBytesRead);
            HLT_WARN("{} total messages read", totalMessagesRead);
            HLT_WARN("{} average message size", static_cast<double>(totalBytesRead) / static_cast<double>(totalMessagesRead));
            HLT_WARN("{} MiB/s throughput", static_cast<double>(totalBytesRead) / (timeout_ * 1024 * 1024));
            conn->getLoop()->queueInLoop([this] { quit(); });
        }
    }

private:
    void quit()
    {
        loop_->queueInLoop([this] { loop_->stop(); });
    }

    void handleTimeout()
    {
        HLT_WARN("stop");
        for (auto &session : sessions_)
        {
            session->stop();
        }
    }

    EventLoop *loop_;
    EventLoopThreadPool threadPool_;
    int sessionCount_;
    int timeout_;
    std::vector<std::unique_ptr<Session>> sessions_;
    std::string message_;
    std::atomic_int32_t numConnected_;
};

void Session::onConnection(const TCPConnectionPtr &conn)
{
    if (conn->isConnected())
    {
        const auto &msg = owner_->message();
        conn->send({const_cast<char *>(msg.c_str()), msg.size()});
        owner_->onConnect();
    }
    else
    {
        owner_->onDisconnect(conn);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 7)
    {
        fprintf(stderr, "Usage: client <host_ip> <port> <threads> <blocksize> ");
        fprintf(stderr, "<sessions> <time>\n");
    }
    else
    {
        Log::init();
        HLT_INFO("pid = {}, tid = {}", getpid(), to_string(std::this_thread::get_id()));

        const char *ip = argv[1];
        auto port = static_cast<uint16_t>(atoi(argv[2]));
        int threadCount = atoi(argv[3]);
        int blockSize = atoi(argv[4]);
        int sessionCount = atoi(argv[5]);
        int timeout = atoi(argv[6]);

        EventLoop loop;
        InetAddress serverAddr(InetAddress::fromHostnameAndPort(ip, port));

        Client client(&loop, serverAddr, blockSize, sessionCount, timeout, threadCount);
        loop.run();
    }
}
