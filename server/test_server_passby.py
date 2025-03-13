"""
test code which passes packets between clients and bots
currently acts as a transfer which passes from client to one bot and from one bot to client

TODO:
- separate send and recv to separate task to prevent unnecessary blocking
    - should be done for recv as "checking" if data is available doesn't work otherwise
    - preferably trough a single websocket connection for each bot/client
    - expand to only execute recv and send code if something is actually available for full standby
    - separate task for sending to all clients?
- add more bots to be handled
    - bot 2 & 3
- error handling & closed connections
"""
import asyncio
import socket
# import queue

import websockets
from websockets.asyncio.server import serve
from websockets.asyncio.client import connect
# from websockets.exceptions import ConnectionClosedOK
from websockets.exceptions import ConnectionClosed

from protobuf.wrapper_pb2 import Wrapper
import print_wrapper_content as print_wrp  # used for console prints


def get_local_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        # doesn't even have to be reachable
        s.connect(('10.255.255.255', 1))
        IP = str(s.getsockname()[0])
    except Exception as b:
        print("error at figuring out local ip")
        print(b)
        exit
    finally:
        s.close()
    return IP


# ip = socket.gethostbyname(socket.gethostname())
# print(ip)
ip = get_local_ip()
DNSinfo = socket.gethostbyaddr(ip)
print(DNSinfo)
del DNSinfo  # not used anywhere so might as well

uri_bot1 = f"ws://{ip}:8083"  # bot1
uri_bot2 = f"ws://{ip}:8083"  # bot2
uri_bot3 = f"ws://{ip}:8083"  # bot3

# https://docs.python.org/3/library/asyncio-sync.html#asyncio.Event
# event_bot1 = asyncio.Event() # event to inform different task

# FIFO Buffers which are written to
# allows non blocking code execution while allowing constant receives
sendto_bot1 = asyncio.Queue()  # FIFO Buffer to send to bot1
sendto_bot2 = asyncio.Queue()  # FIFO Buffer to send to bot2
sendto_bot3 = asyncio.Queue()  # FIFO Buffer to send to bot3

# defines how long each recv are awaited for
# currently send and recv are done in the same task blocks operation
# in a future version this will be separated making this variable irrelevant.
# recv_wait_time = 0.0005

connected_clients = set()  # keep a list of connected clients

# async def handler(websocket):

#     # try:
#     await client(websocket)
#     # except ConnectionClosed:
#     #      connected_clients.remove(websocket)
#     #      print("client disconnected")

#     # match websocket.request.path:
#     #     case "/site":
#     #         await client(websocket)
#     #     case "/give":
#     #         # connected_clients.add(websocket)
#     #         await keep_alive_client(websocket)
#     #     case _:
#     #         # No handler for this path. Close the connection.
#     #         # return
#     #         pass


async def client(websocket):
    """
    receives packets and passes them to the given bot handler
    """
    connected_clients.add(websocket)

    wrp_recv = Wrapper()
    recv_for_client = asyncio.Queue()

    # receive_task_client =
    asyncio.create_task(
        ws_recv_task(recv_for_client, websocket))

    try:
        while True:

            for i in range(50):
                if not recv_for_client.empty():
                    wrp_recv.ParseFromString(
                        recv_for_client.get_nowait())

                    # print("from_client ", end='')
                    # print_wrp.seq(wrp_recv)

                    await sendto_bot1.put(wrp_recv)

            while recv_for_client.empty():  # and sendto_client.empty():
                # when nothing to do currently sleep
                # print ("sleep")
                await asyncio.sleep(0)
    except ConnectionClosed:
        await connected_clients.remove(websocket)


async def keep_alive_client(websocket):
    """keeps a connection open indefinitely in a while asyncio.sleep loop"""
    # wrp_send = Wrapper()
    while True:
        await asyncio.sleep(5)

    # await websocket.send(wrp_send.SerializeToString())


async def ws_send_task_to_clients(data, websocket):
    # future todo
    pass


async def ws_send_task_to_bots(data, websocket):
    # future todo
    pass


async def ws_recv_task(Queue, websocket):
    """
    task which await packets to be received from the websocket connection
    and puts them in the defined buffer until the Connection is closed

    Args:
        Queue (): The buffer which is being written to
        websocket (): the defined websocket connection to be received from
    """
    # lock = asyncio.Lock()
    # global recv_for_bot1 # have to test if defining it is necessary

    while True:
        try:
            # async with lock:
            message = await websocket.recv()
            await Queue.put(message)
        except websockets.ConnectionClosed:
            print("Connection closed")
            break  # break to allow the task to close
        except TimeoutError:
            pass


async def bot1_handler1():
    """
    handles receiving and sending packets rom bot1
    additionally sends packet to all clients
    """
    wrp_recv = Wrapper()
    recv_for_bot1 = asyncio.Queue()
    # global connected_clients
    while True:
        try:
            print("connecting bot1")
            async with connect(uri_bot1) as websocket:

                # receive_task_bot1 =
                asyncio.create_task(
                    ws_recv_task(recv_for_bot1, websocket))

                while True:
                    # await asyncio.sleep(2)
                    # if event_bot1.is_set():
                    #     event_bot1.clear()
                    #     await websocket.send(sendto_bot1.SerializeToString())
                    for i in range(50):
                        # print(i,end=' ')
                        if not sendto_bot1.empty():
                            wrp_send = sendto_bot1.get_nowait()
                            # print_wrp = wrp
                            await websocket.send(wrp_send.SerializeToString())
                        else:
                            break

                    for i in range(50):
                        if not recv_for_bot1.empty():
                            wrp_recv.ParseFromString(
                                recv_for_bot1.get_nowait())

                            # print("from_bot1 ", end='')
                            # # print_wrp.all(wrp_recv)
                            # print_wrp.seq(wrp_recv)

                            # create separate task for this?
                            for client in connected_clients:
                                try:
                                    wrp_send2 = wrp_recv
                                    await client.send(wrp_send2.SerializeToString())
                                except ConnectionClosed:
                                    # connected_clients.remove(client)
                                    # print("hold up")
                                    pass
                                # except:
                                #     pass
                        else:
                            break
                    while recv_for_bot1.empty() and sendto_bot1.empty():
                        # when nothing to do currently sleep
                        await asyncio.sleep(0)

        except ConnectionClosed:
            await asyncio.sleep(1)
            # print(f"closed {b}")


async def gathered_main(websocket):
    """
    how the two task are startet

    potential issue if bot handler falls completely, not a concern as of yet
    """
    # potentially causes issues if loses connection?
    # this waits for both to be finished
    await asyncio.gather(bot1_handler1(), client(websocket))


async def main():
    async with serve(gathered_main, ip, 8081):  # as server:
        # await server.serve_forever()  # run forever
        await asyncio.get_running_loop().create_future()  # run forever


if __name__ == "__main__":
    asyncio.run(main())
