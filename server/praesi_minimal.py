"""
minimal code for presenting the function of the LiDAR
"""
import asyncio
import get_ip

import websockets
# from websockets.asyncio.server import serve
from websockets.asyncio.client import connect
# from websockets.exceptions import ConnectionClosedOK
from websockets.exceptions import ConnectionClosed

from protobuf.wrapper_pb2 import Wrapper
import lidar_gui


# ip = socket.gethostbyname(socket.gethostname()) # doesn't work sometimes
ip = get_ip.get_local_ip()

uri_bot1 = f"ws://192.168.0.154:80/ws"  # bot1
# uri_bot1 = "ws://sb-guide.htlw10.at:80/ws"  # bot1


async def ws_recv_task(queue, websocket):
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
            try:
                wrp_recv = Wrapper()
                wrp_recv.ParseFromString(message)
                await queue.put(wrp_recv.lidar_data.measurements)
            except:
                    pass # do nothing
        except websockets.ConnectionClosed:
            print("Connection closed")
            break  # break to close the task as it isn't needed anymore
        except TimeoutError:
            pass


async def bot1_handler1():
    """
    handles receiving and sending packets rom bot1
    """
    lidar_que = asyncio.Queue()

    while True:
        # try to reattempt connection whit bot
        try:
            print("connecting...")
            async with connect(uri_bot1) as websocket:
                print("connected :O")
                asyncio.create_task(
                    ws_recv_task(lidar_que, websocket))

                asyncio.create_task(
                    lidar_gui.main(True, lidar_que))

                while True:
                    await asyncio.sleep(0)

        except ConnectionClosed:
            print("closed :C")
            await asyncio.sleep(1)  # wait time before repeated attempt


if __name__ == "__main__":
    asyncio.run(bot1_handler1())