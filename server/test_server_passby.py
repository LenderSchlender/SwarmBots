"""
test code which passes packets between clients and bots
currently acts as a transfer which passes from client to one bot and from one bot to client

can be slightly modified to check if client or bot sends wrappers to the server
(potential issue of a FIFO buffer filling up but never emptying for client)

print_wrp.*(wrapper) is used to print out contents of a protobuf wrapper
(basic print operations already set up to be used)
recommended to be done in when the FIFO Queues are being emptied or the recv task

TODO & NOTES:
- separate send and recv to separate task to prevent unnecessary blocking
    - [done] should be done for recv as "checking" if data is available doesn't work otherwise
    - preferably trough a single websocket connection for each bot/client
    - [done with Queue check] expand to only execute recv and send code if something is actually available for full standby
    - separate task for sending to all clients?
- add more bots to be handled
    - bot 2 & 3
- error handling & closed connections
- making sure the bot and server task can be run and started independent from each other
- define how large the FIFO buffers should at most be? (currently no limit)
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
    """
    opens a temporary socket connection and retrieves the local IP
    """
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


# ip = socket.gethostbyname(socket.gethostname()) # doesn't work sometimes
ip = get_local_ip()
DNSinfo = socket.gethostbyaddr(ip)
print(DNSinfo)
del DNSinfo  # not needed anywhere so might as well

uri_bot1 = f"ws://{ip}:8083"  # bot1
uri_bot2 = f"ws://{ip}:8083"  # bot2
uri_bot3 = f"ws://{ip}:8083"  # bot3

# https://docs.python.org/3/library/asyncio-sync.html#asyncio.Event
# not needed currently but left as a note for possible use

# FIFO Buffers which are written to
# allows non blocking code execution while allowing constant receives
sendto_bot1 = asyncio.Queue()  # FIFO Buffer to send to bot1
sendto_bot2 = asyncio.Queue()  # FIFO Buffer to send to bot2
sendto_bot3 = asyncio.Queue()  # FIFO Buffer to send to bot3

connected_clients = set()  # keep a list of connected clients


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

    print could be added here to check what packets are being received
    not done here as i haven't figured out how to clearly mark which task it comes from

    Args:
        Queue (): The buffer which is being written to
        websocket (): the defined websocket connection to be received from
    """
    # lock = asyncio.Lock() # only necessary if multiple task use same socket

    while True:
        try:
            # async with lock:
            message = await websocket.recv()
            # print here if necessary
            await Queue.put(message)
        except websockets.ConnectionClosed:
            print("Connection closed")
            break  # break to close the task as it isn't needed anymore
        except TimeoutError:
            pass


async def client(websocket):
    """
    receives packets and passes them to the given bot Queue
    """
    connected_clients.add(websocket)  # keep track

    wrp_recv = Wrapper()  # wrapper that has been received and to be processed
    recv_for_client = asyncio.Queue()  # FIFO Buffer for received packs

    # receive_task_client =
    asyncio.create_task(
        ws_recv_task(recv_for_client, websocket))  # start recv task

    try:
        while True:

            # empty buffer tasks by given amount before continuing to rest of code
            for buffer in range(50):
                if not recv_for_client.empty():
                    wrp_recv.ParseFromString(
                        recv_for_client.get_nowait())

                    # print here if necessary
                    # print("from_client ", end='')
                    # print_wrp.seq(wrp_recv)

                    await sendto_bot1.put(wrp_recv)
                else:
                    break  # leave loop early if empty

            while recv_for_client.empty():  # and sendto_client.empty():
                # when nothing to do currently sleep
                await asyncio.sleep(0)

    except ConnectionClosed:
        await connected_clients.remove(websocket)


async def bot1_handler1():
    """
    handles receiving and sending packets rom bot1
    additionally sends packet to all clients from connected_clients list
    """
    wrp_recv = Wrapper()   # wrapper that has been received and to be processed
    recv_for_bot1 = asyncio.Queue()  # FIFO Buffer for received packs

    while True:
        # try to reattempt connection whit bot
        try:
            print("connecting bot1")
            async with connect(uri_bot1) as websocket:
                # receive_task_bot1 =
                asyncio.create_task(
                    ws_recv_task(recv_for_bot1, websocket))

                while True:
                    # empty buffer tasks by given amount before continuing to rest of code
                    for buffer in range(50):
                        if not sendto_bot1.empty():
                            # sends packets in buffer to bot
                            wrp_send = sendto_bot1.get_nowait()
                            await websocket.send(wrp_send.SerializeToString())
                        else:
                            break  # leave loop early if empty

                    # empty buffer tasks by given amount before continuing to rest of code
                    for buffer in range(50):
                        if not recv_for_bot1.empty():
                            wrp_recv.ParseFromString(
                                recv_for_bot1.get_nowait())

                            # print here if necessary
                            # print("from_bot1 ", end='')
                            # print_wrp.seq(wrp_recv)

                            # create separate task for this?
                            # sends to all known clients
                            # should probably be replaced with broadcast if kept
                            for client in connected_clients:
                                try:
                                    wrp_send2 = wrp_recv
                                    await client.send(wrp_send2.SerializeToString())
                                except ConnectionClosed:
                                    # connected_clients.remove(client)
                                    # print("hold up")
                                    pass
                        else:
                            break  # leave loop early if empty

                    while recv_for_bot1.empty() and sendto_bot1.empty():
                        # when nothing to do currently sleep
                        await asyncio.sleep(0)

        except ConnectionClosed:
            await asyncio.sleep(1)  # wait time before repeated attempt


async def gathered_main(websocket):
    """
    gathers tasks to start them both

    potential issue if bot handler falls completely, not a concern as of yet

    as the bot handler isn't "server code" to handle clients
    but instead separate "client code" as it tries to connect with the bots.

    it has to be changed so these tasks can be run and started independent from each other
    """
    # potentially causes issues if loses connection?
    # this waits for both to be finished
    await asyncio.gather(bot1_handler1(), client(websocket))


async def main():
    """
    makes sure the server keeps running

    change gathered_main for client() to run client code exclusively if necessary
    """
    async with serve(gathered_main, ip, 8081):  # as server:
        # await server.serve_forever()  # run forever
        await asyncio.get_running_loop().create_future()  # run forever


if __name__ == "__main__":
    """
    change main() for bot_handler to run bot_handler code exclusively if necessary
    """
    asyncio.run(main())
