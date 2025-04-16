"""basic testcode simulating a sending and receiving wrappers"""
import asyncio
import get_ip
from websockets.asyncio.client import connect
from websockets.exceptions import ConnectionClosed


from protobuf.wrapper_pb2 import Wrapper
import print_wrapper_content as print_wrp

recv_wait_time = 0.001

wrp_send = Wrapper()
wrp_recv = Wrapper()

ip = '172.16.2.116'

# ip = socket.gethostbyname(socket.gethostname())
# print(ip)
ip = get_ip.get_local_ip()


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


async def hello():
    uri = f"ws://{ip}:8081/site"
    recv_for_bot1 = asyncio.Queue()
    async with connect(uri) as websocket:
        with open('Dataw', "rb") as f:
            wrp_send.ParseFromString(f.read())

            i = 0

        asyncio.create_task(
            ws_recv_task(recv_for_bot1, websocket))
        while True:
            print(i)
            i = i+1

            # first_led = wrp_send.led_cmd.states.add()
            # first_led.number = 1
            # # a = b'\x00\x10'
            # first_led.rgb = (20).to_bytes(length=4, byteorder='big')

            # second_led = wrp_send.led_cmd.states.add()
            # second_led.number = 2
            # second_led.rgb = (255).to_bytes(length=4, byteorder='big')
            await asyncio.sleep(0.01)
            await websocket.send(wrp_send.SerializeToString())
            # print(f"sending: {i}")

            # await asyncio.sleep(1)

            for buffer in range(50):
                if not recv_for_bot1.empty():
                    wrp_recv.ParseFromString(
                        recv_for_bot1.get_nowait())
                    print("receiving ", end='')
                    # print_wrp.all(wrp_recv)
                    print_wrp.seq(wrp_recv)

            # try:
            #     pack = await asyncio.wait_for(websocket.recv(), timeout=recv_wait_time)
            #     wrp_recv.ParseFromString(pack)
            #     # print("\n---------------receiving------------------")
            #     print("receiving ", end='')
            #     # print_wrp.all(wrp_recv)
            #     print_wrp.seq(wrp_recv)
            # except asyncio.TimeoutError:
            #     # print("nothing received server")
            #     pass

        pass
        pack = await websocket.recv()
        wrp_recv.ParseFromString(pack)
        # print(wrp_recv.encoder_data.pulses)
        # print(wrp_recv.encoder_data.duration)
        print_wrp.encoder_data(wrp_recv)
        print_wrp.seq(wrp_recv)

if __name__ == "__main__":
    asyncio.run(hello())
