import smbus, time, random

bus = smbus.SMBus(1)
ADDR = [0x0a,
        0x0d,
        0x0e,
        0x0f,
        #0x11,
        0x12]
#ADDR = [0x11,0x12]

def write(dev,reg,val):
    try:
        bus.write_byte_data(dev,reg,val)
        time.sleep(0.05)
    except:
        print("i2c error writing: "+str(dev))

def read(dev,reg):
    try:
        v = bus.read_byte_data(ADDR[dev],reg)
        time.sleep(0.05)
        return v
    except:
        print("i2c error reading: "+str(dev))
        return 0

REG_VERSION=0
REG_ALIVE=1
REG_ADDR=2
REG_MODE=3
MODE_POWER_OFF=0
MODE_NORMAL=1
MODE_CALI=2
MODE_DEMO=3
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
default_b_hide_angle=90
default_c_hide_angle=0
default_d_hide_angle=50 

default_a_show_angle=60
default_b_show_angle=45
default_c_show_angle=45
default_d_show_angle=10

def calibrate(dev,hide,show):
    write(dev,REG_SERVO_A_HIDE,hide[0])
    write(dev,REG_SERVO_A_SHOW,show[0])
    write(dev,REG_SERVO_B_HIDE,hide[1])
    write(dev,REG_SERVO_B_SHOW,show[1])
    write(dev,REG_SERVO_C_HIDE,hide[2])
    write(dev,REG_SERVO_C_SHOW,show[2])
    write(dev,REG_SERVO_D_HIDE,hide[3])
    write(dev,REG_SERVO_D_SHOW,show[3])
    write(dev,REG_EESAVE,1)

def factory_reset(dev):
    write(dev,REG_EEINIT,1)

def test_one(dev):
    choice=0
    while True:    
        write(dev,REG_MODE,2)
        time.sleep(0.1)
        write(dev,REG_MODE,1)
        time.sleep(0.05)
        write(dev,REG_SHOW_ID,choice)
        print(choice)
        choice+=1
        if choice>3: choice=0
        time.sleep(1)

def test_all():
    choice=0
    dev=0
    while True:    
        dev=dev+1
        if dev>=len(ADDR):
            dev=0
            choice+=1
        if choice>3: choice=0

        # clear just in case
        write(ADDR[dev],REG_MODE,2)
        time.sleep(0.1)
        write(ADDR[dev],REG_MODE,1)
        time.sleep(0.05)
        write(ADDR[dev],REG_SHOW_ID,random.randrange(0,5))
        time.sleep(1)

def flash_all():
    choice=0
    dev=0
    while True:    
        dev=dev+1
        if dev>=len(ADDR):
            dev=0
            choice+=1
        if choice>1: choice=0
        write(ADDR[dev],REG_LED,choice)
        time.sleep(0.1)

# turn all on
for dev in range(0,len(ADDR)): 
    write(ADDR[dev],REG_MODE,MODE_NORMAL)
    time.sleep(0.2)

def cali():
    calibrate(0x0a,[10,90,0,50],[70,40,60,0])  # <- doge plug a
    calibrate(0x0b,[10,90,0,70],[80,30,60,10])
    calibrate(0x0c,[0,90,0,50],[60,40,60,0])       
    calibrate(0x0d,[10,100,0,60],[80,30,60,0]) 
    calibrate(0x0e,[10,90,0,50],[80,30,60,0])
    calibrate(0x0f,[10,90,0,50],[80,30,60,0])
    calibrate(0x10,[10,90,0,60],[80,30,60,0]) # <- fixed 4 show distance
    calibrate(0x11,[10,90,0,50],[70,40,60,0])       
    calibrate(0x12,[10,90,0,50],[80,30,60,0])

cali()    

#calibrate(0x0b,[10,90,0,70],[80,30,60,10])
#test_one(0x0d)

test_all()
#flash_all()
