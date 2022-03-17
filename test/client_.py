import asyncio
import threading

async def tcp_echo_client(message):
    reader, writer = await asyncio.open_connection(
        '127.0.0.1', 8080)

    print(f'Send: {message!r}')
    writer.write(message.encode())
    await writer.drain()

    data = await reader.read(100)
    print(f'Received: {data.decode()!r}')

    print('Close the connection')
    writer.close()



def run_thread():
    tasks = [tcp_echo_client('Hello World!') for _ in range(1000)]
    asyncio.run(asyncio.wait(tasks))
    print('thread success')

class _run_thread(threading.Thread):
    def run(self):
        run_thread()

threads = []
for _ in range(7):
    tmp = _run_thread()
    tmp.start()
    threads.append(tmp)

run_thread()

for thread in threads:
    thread.join()
