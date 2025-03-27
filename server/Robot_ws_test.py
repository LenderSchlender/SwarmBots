"""
test code to test single robot websocket connection

some parameter can be changed see below
as well as print formats directly before and after queue

uri_bot1 can be changed too if necessary, currently set to use local IP
"""
import asyncio
import socket

import websockets
from websockets.asyncio.server import serve
from websockets.asyncio.client import connect
# from websockets.exceptions import ConnectionClosedOK
from websockets.exceptions import ConnectionClosed

from protobuf.wrapper_pb2 import Wrapper
import print_wrapper_content as print_wrp  # used for console prints

# some parameter for testing
SEND_BACK = True  # sends the received data back
CODE_IF_SOCKET_RECV = True  # if something has been received
CODE_AT_QUEUE_EMPTY = False  # after queue being emptied


def code_at_queue_empty(protobuff_wrapper):
    # passes current protobuff_wrapper
    # redefine print format or make your own
    # aka don't bother searching where you need to change it
    print_wrp.all(protobuff_wrapper)


def code_at_socket_recv(websocket_data):
    # currently takes the packet and puts into a queue
    # use
    # 'wrp_recv.ParseFromString(websocket_data)'
    # if you wanna read protobuff contents
    print("received")


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

# FIFO Buffers which are written to
# allows non blocking code execution while allowing constant receives
sendto_bot1 = asyncio.Queue()  # FIFO Buffer to send to bot1


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
    while True:
        try:
            message = await websocket.recv()

            if CODE_IF_SOCKET_RECV:
                code_at_socket_recv(message)

            await Queue.put(message)
        except websockets.ConnectionClosed:
            print("Connection closed")
            break  # break to close the task as it isn't needed anymore
        except TimeoutError:
            pass


async def bot1_handler1():
    """
    handles receiving and sending packets rom bot1
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
                    if SEND_BACK:
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
                            t = recv_for_bot1.get_nowait()
                            wrp_recv.ParseFromString(t)

                            if CODE_AT_QUEUE_EMPTY:
                                # print here if necessary
                                code_at_queue_empty(wrp_recv)
                                pass

                            if SEND_BACK:
                                sendto_bot1.put(t)

                        else:
                            break  # leave loop early if empty

                    while recv_for_bot1.empty() and sendto_bot1.empty():
                        # when nothing to do currently sleep
                        await asyncio.sleep(0)

        except ConnectionClosed:
            await asyncio.sleep(1)  # wait time before repeated attempt


if __name__ == "__main__":
    asyncio.run(bot1_handler1())
