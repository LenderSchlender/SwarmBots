import time
import asyncio
import pygame

import websockets
from websockets.asyncio.client import connect
import get_ip

from protobuf.wrapper_pb2 import Wrapper
import print_wrapper_content as print_wrp  # used for console prints

# import traceback


pygame.init()
# initialise the joystick module
pygame.joystick.init()

ip = get_ip.get_local_ip()
uri = f"ws://{ip}:8081"
# uri_bot1 = "ws://sb-guide.htlw10.at:80/ws"  # bot1

wrp_send = Wrapper()

stick1 = [0, 0]  # left stick
stick2 = [0, 0]  # right stick

# from 0 to 1 scale set the minimum dead zone in case of inacurate zero point
mindeadzone = 0.0045
# maxantideadzone_r2 = 0.9
# maxantideadzone = 0.9

max_expected_change = int(2147483647*0.75)  # per second based on +-
steer_modifier = 0.5  # change intensity of one stick steer

# create empty list to store joysticks
joysticks = []

# # code for the Linear Interpolation
# INT16_MIN = -32768
# INT16_MAX = 32767


# def lerp(start, middle, end, where):
#     if where < 0:
#         return start + (where-INT16_MIN) * (middle-start) / INT16_MAX
#     elif where > 0:
#         return middle + where * (end-middle) / INT16_MAX
#     else:
#         return middle


# def clamp(n, minn, maxn):
#     return max(min(n, maxn), minn)


def change_dampening(current_value, past_value,
                     current_time, past_time,
                     max_expected_change):
    # used for limiting the maximum rate of change in the detected time window
    delta_t = current_time - past_time

    relativevalue_change = (current_value - past_value)/(delta_t)
    relativeexpected_change = (max_expected_change/(delta_t))

    if (relativevalue_change) > relativeexpected_change:
        # catches exceeded positive change
        past_value = past_value + relativeexpected_change * delta_t
        past_time = current_time
    elif (relativevalue_change) < -relativeexpected_change:
        # catches exceeded negative change
        past_value = past_value - relativeexpected_change * delta_t
        past_time = current_time
    else:
        # passes value through
        past_value = current_value

    return past_value, past_time


async def ws_send_task(lock):
    """
    """
    global wrp_send
    while True:
        # try to reattempt connection whit bot
        try:
            print("connecting bot1")
            async with connect(uri) as websocket:

                while True:
                    async with lock:
                        # await async_event.wait()
                        print_wrp.move_cmd(wrp_send)
                        tosend = wrp_send.SerializeToString()
                    await websocket.send(tosend)

                    # async_event.clear()
                    await asyncio.sleep(1/30)
        except websockets.ConnectionClosed:
            print("Connection closed")
            # break  # break to close the task as it isn't needed anymore
        except TimeoutError:
            print("timeout_err")
            pass


async def game_loop(run):
    global max_expected_change
    global mindeadzone
    global joysticks
    global stick1
    global stick2
    global wrp_send

    # incase of unexpected continous rumbling during closing
    # so that i don't have to disconect the gamepad c:
    # doesn't seem to always work :c
    # used to stop inconsistent rumbling?
    for joystick in joysticks:
        joystick.stop_rumble()

    # setup to prevent Errors in the change_dampening() function
    current_time = time.process_time_ns()
    speed = 0
    past_time_speed = current_time - 0.000001
    steer = 0
    past_time_steer = current_time - 0.000001
    speed_2 = 0
    past_time_speed_2 = current_time - 0.000001
    steer_2 = 0
    past_time_steer_2 = current_time - 0.000001

    # async_event = asyncio.Event()
    async_lock = asyncio.Lock()

    asyncio.create_task(
        ws_send_task(async_lock))

    # game loop
    while run:

        # inputs
        for joystick in joysticks:
            # analogue sticks
            horiz_s1 = joystick.get_axis(0)
            vert_s1 = joystick.get_axis(1)
            if abs(vert_s1) > mindeadzone:
                stick1[1] = (-vert_s1 * 2147483647)
            else:
                stick1[1] = 0
            if abs(horiz_s1) > mindeadzone:
                stick1[0] = (horiz_s1 * 2147483647)
            else:
                stick1[0] = 0

            horiz_s2 = joystick.get_axis(2)
            vert_s2 = joystick.get_axis(3)
            if abs(vert_s2) > mindeadzone:
                stick2[1] = (-vert_s2 * 2147483647)
            else:
                stick2[1] = 0
            if abs(horiz_s2) > mindeadzone:
                stick2[0] = (horiz_s2 * 2147483647)
            else:
                stick2[0] = 0

            # Action buttons
            AorX = joystick.get_button(0)
            BorO = joystick.get_button(1)
            XorSquare = joystick.get_button(2)
            YorTriangle = joystick.get_button(3)
            if AorX:
                Controlscheme = 0
            if BorO:
                Controlscheme = 1
            if XorSquare:
                Controlscheme = 2
            if YorTriangle:
                Controlscheme = 3
            # Bumper
            BumperR1 = joystick.get_button(10)
            if BumperR1:
                Controlscheme = 4
                print("Stopping output")

        current_speed = stick1[1]
        current_steer = stick1[0]
        current_speed_2 = stick2[1]
        current_steer_2 = stick2[0]

        current_time = time.perf_counter_ns()  # or time.perf_counter_ns()

        # prevents high rate of change
        speed, past_time_speed = change_dampening(
            current_speed, speed, current_time, past_time_speed, max_expected_change)
        steer, past_time_steer = change_dampening(
            current_steer, steer, current_time, past_time_steer, max_expected_change)
        speed_2, past_time_speed_2 = change_dampening(
            current_speed_2, speed_2, current_time, past_time_speed_2, max_expected_change)
        steer_2, past_time_steer_2 = change_dampening(
            current_steer_2, steer_2, current_time, past_time_steer_2, max_expected_change)

        # sendstuff here
        async with async_lock:
            wrp_send.move_cmd.steer = int(steer)
            wrp_send.move_cmd.speed = int(speed)
            wrp_send.move_cmd.duration = 1
            # print("l")
        # print_wrp.move_cmd(wrp_send)

        # async_event.set()

        # this part of the code doesn't seem to work
        # event handler
        for event in pygame.event.get():
            if event.type == pygame.JOYDEVICEADDED:
                joy = pygame.joystick.Joystick(event.device_index)
                joysticks.append(joy)
                joy.stop_rumble()
            # quit program
            if event.type == pygame.QUIT:
                run = False

        # affects the loop time to prevent overflow to the websocket server
        if async_lock.locked():
            await asyncio.sleep(0)
        await asyncio.sleep(1/30)

    # pygame.quit()

if __name__ == "__main__":
    asyncio.run(game_loop(True))
