from driver import *
import time, crc, random

r = robot_virus()

r.boot()

#while True:
#    r.flash_all(10)

#r.test_one(0x13)
#r.freeze_one(0x0a)

#r.distribute_shapes(["tri","tri","tri","tri","tri"])

#while True: pass


def loop_test():
    while True:
        print("loop")
        r.clear_shapes()
        shapes = ["squ","tri","gui","cir"]
        random.shuffle(shapes)
        for shape in shapes:
            r.distribute_shapes([shape])
            time.sleep(2)

loop_test()
    
def lo_test():
    r.power_pin.on()
    time.sleep(1)
    crc_write(0x13,0x04,0x00)
    print(read(0x13,0xfa))
    print(read(0x13,0xfb),read(0x13,0xfc))
    time.sleep(1)
    crc_write(0x13,0x04,0x01)
    print(read(0x13,0xfa))
    print(read(0x13,0xfb),read(0x13,0xfc))
    time.sleep(1)
    crc_write(0x13,0x03,0x01)
    print(read(0x13,0xfa))
    time.sleep(1)
    crc_write(0x13,0x05,0x00)
    print(read(0x13,0xfa))
    time.sleep(1)
    crc_write(0x13,0x05,0x01)
    print(read(0x13,0xfa))
    time.sleep(1)
    crc_write(0x13,0x05,0x02)
    print(read(0x13,0xfa))
    time.sleep(1)
    crc_write(0x13,0x05,0x03)
    print(read(0x13,0xfa))
    time.sleep(1)
