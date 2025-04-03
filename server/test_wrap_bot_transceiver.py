"""basic test code to simulate one of the bots sending and receiving a wrappers"""
import asyncio
import socket
from websockets.asyncio.server import serve
from websockets.exceptions import ConnectionClosed


from protobuf.wrapper_pb2 import Wrapper
import print_wrapper_content as print_wrp

import random

recv_wait_time = 0.001


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

wrp_send = Wrapper()
wrp_recv = Wrapper()


async def sending():
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
        except ConnectionClosed:
            print("Connection closed")
            break  # break to allow the task to close
        except TimeoutError:
            pass


async def hello(websocket):
    a = 0
    i = 0

    recv_for_bot1 = asyncio.Queue()
    asyncio.create_task(
        ws_recv_task(recv_for_bot1, websocket))
    while True:
        # print ("wow")
        # try:
        #     pack = await asyncio.wait_for(websocket.recv(), timeout=recv_wait_time)
        #     wrp_recv.ParseFromString(pack)
        #     # print("\n---------------receiving------------------")
        #     print("receiving ", end='')
        #     # print_wrp.all(wrp_recv)
        #     print_wrp.seq(wrp_recv)
        # except asyncio.TimeoutError:
        #     # print("nothing received client")
        #     pass

        for buffer in range(50):
            if not recv_for_bot1.empty():
                wrp_recv.ParseFromString(
                    recv_for_bot1.get_nowait())
                print("receiving ", end='')
                # print_wrp.all(wrp_recv)
                print_wrp.all(wrp_recv)
                # print(wrp_recv)

        a = random.randint(0, 12)
        # a = 1
        # print(a)
        if a == 1:
            wrp_send = wrp_recv
            i = i+1
            wrp_send.seq = i

            wrp_send.ping_pong = 1

            wrp_send.move_cmd.steer = -1
            wrp_send.move_cmd.speed = 69
            wrp_send.move_cmd.duration = 30

            # print(i)
            # print("\n---------------send_to_server------------------")
            # print_wrp.seq(wrp_send)
            await websocket.send(wrp_send.SerializeToString())
            # await asyncio.sleep(0.02)
        else:
            await asyncio.sleep(0)


async def main():
    async with serve(hello, ip, 8083):
        await asyncio.get_running_loop().create_future()  # run forever

if __name__ == "__main__":
    asyncio.run(main())
