"""prints out defined wrapper data to the console

print_wrp.*(wrapper)

used for debug/testing (nothing fancy)

Example from print_wrp.all():
seq             0
ping_pong       0
move_cmd        0 0 0ms
---led_cmd:
[]
encoder_data    0 0ms
---lidar_data:
[]
imu_data        acc:000 rot:000 tmp:0
"""
# from pprint import pprint


def all(wrp):
    """prints out all the contents of the wrapper
    """
    seq(wrp)
    ping_pong(wrp)
    move_cmd(wrp)
    led_cmd(wrp)
    encoder_data(wrp)
    lidar_data(wrp)
    imu_data(wrp)

    pass  # display list contents


def seq(wrp):
    print("seq", end='\t\t')
    print(wrp.seq)


def ping_pong(wrp):
    print("ping_pong", end='\t')
    print(wrp.ping_pong)


def move_cmd(wrp):
    print("move_cmd", end='\t')
    print(wrp.move_cmd.speed, end=' ')
    print(wrp.move_cmd.steer, end=' ')
    print(wrp.move_cmd.duration, end='')
    print('ms')


def led_cmd(wrp):
    print("---led_cmd:")
    # print(wrp.led_cmd.number, end=' ')
    # print(wrp.led_cmd.rgb, end=' ')
    print(wrp.led_cmd.states)


def lidar_data(wrp):
    print("---lidar_data:")
    # idk
    print(wrp.lidar_data.measurements)
    pass


def encoder_data(wrp):
    print("encoder_data", end='\t')
    print(wrp.encoder_data.pulses, end=' ')
    print(wrp.encoder_data.duration, end='')
    print("ms")


def imu_data(wrp):
    print("imu_data", end='\t')
    print('acc:', end='')
    print(wrp.imu_data.acceleration_x, end='')
    print(wrp.imu_data.acceleration_y, end='')
    print(wrp.imu_data.acceleration_z, end=' ')

    print('rot:', end='')
    print(wrp.imu_data.rotation_x, end='')
    print(wrp.imu_data.rotation_y, end='')
    print(wrp.imu_data.rotation_z, end=' ')

    print('tmp:', end='')
    print(wrp.imu_data.temperature)
