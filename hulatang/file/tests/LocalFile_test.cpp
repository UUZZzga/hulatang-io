#include "hulatang/base/Buffer.hpp"
#include "hulatang/base/File.hpp"
#include "hulatang/base/Log.hpp"
#include "hulatang/file/LocalFile.hpp"
#include "hulatang/io/EventLoop.hpp"

#include <system_error>

using hulatang::base::Buffer;
using hulatang::base::Log;
using hulatang::base::O_READ;
using hulatang::base::O_WRITE;
using hulatang::file::LocalFile;
using hulatang::io::EventLoop;
using std::error_condition;

int main(int _argc, const char **_argv)
{
    const auto *fileName = "123";
    auto data = std::string_view("abcdefgh");

    Log::init();
    EventLoop loop;

    LocalFile file(&loop);
    file.setErrorCallback([](error_condition &ec) { assert(ec); });

    std::unique_ptr<LocalFile> fileRead;
    file.setWriteCallback([&file, &loop, &fileName, &data, &fileRead] {
        file.close();

        fileRead = std::make_unique<LocalFile>(&loop);
        fileRead->setErrorCallback([](error_condition &ec) { assert(ec); });
        fileRead->setReadCallback([&loop, &data](Buffer &buf) {
            if (buf.readableBytes() >= data.size())
            {
                assert(buf.toString_view() == data);
                loop.stop();
            }
        });

        {
            fileRead->open(fileName, O_READ);
        }

        fileRead->read(data.size());
    });
    {
        file.create(fileName, O_WRITE);
    }
    file.write(data);
    loop.run();
    return 0;
}