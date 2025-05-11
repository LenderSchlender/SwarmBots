import pygame
# import serial
import math
# import time

import asyncio

SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600
# https://www.tutorialspoint.com/creating-a-radar-sweep-animation-using-pygame-in-python
pygame.init()
screen = pygame.display.set_mode([SCREEN_WIDTH, SCREEN_HEIGHT])

# ser = serial.Serial("/dev/ttyUSB0", baudrate=115200);
# ser.flush() # clear the port
# print("SERIAL DONE")

running = True
MAX_MEASUREMENTS = 1000
count = 0
# array of (angle, distance) with placeholders values
measurements = [(0, -1, 255) for x in range(MAX_MEASUREMENTS)]
measurements[0] = (0, 50, 255)
measurements[1] = (90, 150, 255)
measurements[2] = (90, 500, 255)

radar_circle_radius = SCREEN_HEIGHT / 2 - 25
radar_circle_center_x = int(SCREEN_WIDTH / 2)
radar_circle_center_y = int(SCREEN_HEIGHT / 2)

background = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT))
background.fill((5, 15, 15))
pygame.draw.circle(background, (0, 35, 15), (radar_circle_center_x,
                   radar_circle_center_y), radar_circle_radius, 2)


def render():
    screen.blit(background, (0, 0))  # apply background

    # Draw radar sweep line
    radar_sweep_angle, _, _ = measurements[count % MAX_MEASUREMENTS]
    x = (radar_circle_center_x + radar_circle_radius *
         math.sin(math.radians(radar_sweep_angle)))
    y = (radar_circle_center_y + radar_circle_radius *
         math.cos(math.radians(radar_sweep_angle)))
    pygame.draw.line(screen, (127, 127, 127), (radar_circle_center_x,
                                               radar_circle_center_y), (x, y), 3)

    # Draw measurements
    for measurement in measurements:
        angle, distance, intensity = measurement
        if distance <= 0:
            continue
        if intensity <= 0:  # fallback
            blue = 255
        else:
            blue = 0
        # TODO find unit for distance (assume mm for now)
        distancePx = distance / 20  # One px = 5cm
        x = distancePx * math.cos(math.radians(angle)) + radar_circle_center_x
        y = distancePx * math.sin(math.radians(angle)) + radar_circle_center_y
        pygame.draw.circle(screen, (intensity, 0, blue), (x, y), 1)

    # Flip the display
    pygame.display.flip()


async def main(running, Queue):
    global count
    global measurements
    render()
    print("STARTING LOOP")
    while running:
        # Did the user click the window close button?
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

        try:
            # if (ser.in_waiting == 0):
            #     time.sleep(0.001)
            #     continue
            # received = ser.readline().decode('utf-8')
            # data = received.replace('\r', '').replace('\n', '')
            # vals = [float(ii) for ii in data.split(':')]
            # if len(vals) != 2:
            #     continue
            # angle, dist = vals

            # measurements[count % MAX_MEASUREMENTS] = vals
            # if count % 10 == 0:  # update every 10 measurements
            #     render()
            #     # print(count)
            # count += 1

            while Queue.empty():
                await asyncio.sleep(0)
            received = Queue.get_nowait()

            for m in received:
                vals = (m.angle, m.distance, m.intensity)
                # angle, dist = vals

                measurements[count % MAX_MEASUREMENTS] = vals
                if count % 10 == 0:  # update every 10 measurements
                    render()
                    # print(count)
                count += 1

        except KeyboardInterrupt:
            # ser.close()
            print('Keyboard Interrupt')
            break

    pygame.quit()

    # Done! Time to quit.


if __name__ == "__main__":
    asyncio.run(main(True))
