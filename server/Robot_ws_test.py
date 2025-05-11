"""
test code to test single robot websocket connection

some parameter can be changed see below
as well as print formats directly before and after queue

uri_bot1 can be changed too if necessary, currently set to use local IP
"""
import asyncio
# import socket
import random

import get_ip

import websockets
# from websockets.asyncio.server import serve
from websockets.asyncio.client import connect
# from websockets.exceptions import ConnectionClosedOK
from websockets.exceptions import ConnectionClosed

from protobuf.wrapper_pb2 import Wrapper
import print_wrapper_content as print_wrp  # used for console prints

import lidar_gui

# some parameter for testing
SEND_BACK = False  # sends the received data back
CODE_IF_SOCKET_RECV = True  # if something has been received
CODE_AT_QUEUE_EMPTY = True  # after queue being emptied


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
    # print("received")
    pass


# ip = socket.gethostbyname(socket.gethostname()) # doesn't work sometimes
ip = get_ip.get_local_ip()


uri_bot1 = f"ws://{ip}:8083"  # bot1
# uri_bot1 = "ws://sb-guide.htlw10.at:80/ws"  # bot1


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
    wrp_send = Wrapper()
    recv_for_bot1 = asyncio.Queue()  # FIFO Buffer for received packs
    lidar_que = asyncio.Queue()

    i = 0  # increment value

    while True:
        # try to reattempt connection whit bot
        try:
            print("connecting bot1")
            async with connect(uri_bot1) as websocket:
                print("connected :O")
                # receive_task_bot1 =
                asyncio.create_task(
                    ws_recv_task(recv_for_bot1, websocket))

                asyncio.create_task(
                    lidar_gui.main(True, lidar_que))

                await sendto_bot1.put(wrp_recv)

                while True:
                    if SEND_BACK:
                        # empty buffer tasks by given amount before continuing to rest of code
                        for buffer in range(50):
                            if not sendto_bot1.empty():
                                # sends packets in buffer to bot

                                wrp_send = sendto_bot1.get_nowait()

                                # wrp_send.seq = i

                                wrp_send.move_cmd.steer = -1
                                wrp_send.move_cmd.speed = 69
                                wrp_send.move_cmd.duration = 30

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
                                # print(t.hex())
                                pass

                            if SEND_BACK:
                                await sendto_bot1.put(wrp_recv)

                            await lidar_que.put(wrp_recv.lidar_data.measurements)

                        else:
                            break  # leave loop early if empty

                    while recv_for_bot1.empty() and sendto_bot1.empty():
                        # when nothing to do currently sleep
                        await asyncio.sleep(0)

                    a = random.randint(0, 1)
                    a = 1
                    if a == 1:
                        wrp_send = Wrapper()
                        i = i+1
                        wrp_send.seq = i

                        # wrp_send.ping_pong = 1

                        steer = random.randint(-1, 1)
                        wrp_send.move_cmd.steer = steer
                        wrp_send.move_cmd.speed = 69
                        wrp_send.move_cmd.duration = 300

                        print(i)
                        # print("\n---------------send_to_server------------------")
                        # print_wrp.seq(wrp_send)
                        await websocket.send(wrp_send.SerializeToString())
                        await asyncio.sleep(3)
                    else:
                        await asyncio.sleep(3)

        except ConnectionClosed:
            print("closed :C")
            await asyncio.sleep(1)  # wait time before repeated attempt


if __name__ == "__main__":
    asyncio.run(bot1_handler1())
