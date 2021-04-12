import smbus, time, random

bus = smbus.SMBus(1)
ADDR = 0x0a

def write(reg,val):
    bus.write_byte_data(ADDR,reg,val)
    time.sleep(0.05)

def read(reg):
    v = bus.read_byte_data(ADDR,reg)
    time.sleep(0.05)
    return v

REG_VERSION=0
REG_ALIVE=1
REG_ADDR=2
REG_MODE=3
MODE_NORMAL=0
MODE_CALI=1
MODE_DEMO=2
REG_LED=4
REG_SHOW_ID=5 # 0,1,2,3 or anything else = hide all 
REG_SERVO_SPEED=6
REG_SERVO_INTERPOLATION=7
REG_SERVO_START=8
REG_EEINIT=0xfd
REG_EESAVE=0xfe

REG_SERVO_A_ANGLE=0x08 
REG_SERVO_A_ACTIVE=0x09 
REG_SERVO_A_HIDE=0x0a
REG_SERVO_A_SHOW=0x0b 
REG_SERVO_B_ANGLE=0x0c 
REG_SERVO_B_ACTIVE=0x0d 
REG_SERVO_B_HIDE=0x0e
REG_SERVO_B_SHOW=0x0f 
REG_SERVO_C_ANGLE=0x10 
REG_SERVO_C_ACTIVE=0x11 
REG_SERVO_C_HIDE=0x12
REG_SERVO_C_SHOW=0x13 
REG_SERVO_D_ANGLE=0x14 
REG_SERVO_D_ACTIVE=0x15 
REG_SERVO_D_HIDE=0x16
REG_SERVO_D_SHOW=0x17 


default_a_hide_angle=15
default_a_show_angle=60
default_b_hide_angle=90
default_b_show_angle=45
default_c_hide_angle=0
default_c_show_angle=45
default_d_hide_angle=50 
default_d_show_angle=10


#write(REG_SERVO_A_HIDE,10)
#write(REG_SERVO_A_SHOW,70)
#write(REG_SERVO_B_HIDE,100)
#write(REG_SERVO_C_SHOW,50)
#write(REG_SERVO_C_HIDE,0)
#write(REG_SERVO_D_SHOW,0)
#write(REG_EESAVE,1)

write(REG_MODE,MODE_NORMAL)

last=5
while True:    
    chosen = random.randrange(0,6)
    if chosen!=last:
        try:
            write(REG_SHOW_ID,chosen)
        except:
            print("i2c error")
        last=chosen
    time.sleep(1)
