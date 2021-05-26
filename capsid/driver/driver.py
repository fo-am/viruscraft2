import smbus, time, random, copy, crc
from gpiozero import LED

##############################################
# i2c interface

bus = smbus.SMBus(1)

max_tries = 5

do_reboot = False

def lo_write(dev,reg,val):
    global do_reboot
    tries = 0
    while tries<max_tries:
        try:
            bus.write_byte_data(dev,reg,val)
            time.sleep(0.01)
            return
        except:
            time.sleep(0.25)
            tries+=1
    do_reboot=True
    print("i2c error writing: "+hex(dev))


def write(dev,reg,val):
    tries = 0
    while True:
        check = crc.crc16([0x42,reg,val])
        REG_CRCL = 0xfb
        REG_CRCH = 0xfc
        REG_CRC_STATUS = 0xfa
        lo_write(dev,REG_CRCL,check&0xff)
        lo_write(dev,REG_CRCH,check>>8)
        lo_write(dev,reg,val)
        if not read(dev,REG_CRC_STATUS) or tries>max_tries: return
        print("crc failed, retry... "+hex(dev))
        tries+=1
    
def read(dev,reg):
    try:
        v = bus.read_byte_data(dev,reg)
        time.sleep(0.05)
        return v
    except:
        #print("i2c error reading: "+str(dev))
        return 0


# return all addresses present
def scan_i2c():
    ret = []
    for scan in range(0,5):
        # i2c range: 0x07 -> 0x78 - reserve 0x7x for the sensors
        for addr in range(0x07,0x6f):
            # attempt reading the version        
            if read(addr,0x00)>0:
                if not addr in ret:
                    ret.append(addr)            
    return ret
    
###################################################
# controller for a robot virus

class robot_virus:
    def __init__(self):
        self.addresses = []
        # well ok rpi - I have to pretend everything is an LED?
        self.power_pin = LED(14)
        self.state = {}

    def scan_addresses(self):
        self.addresses = scan_i2c()
        print("found: ")
        print(self.addresses)
        for addr in self.addresses:
            self.state[addr]="none"

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

    def calibrate(self,dev,hide,show):
        write(dev,self.REG_SERVO_A_HIDE,hide[0])
        write(dev,self.REG_SERVO_A_SHOW,show[0])
        write(dev,self.REG_SERVO_B_HIDE,hide[1])
        write(dev,self.REG_SERVO_B_SHOW,show[1])
        write(dev,self.REG_SERVO_C_HIDE,hide[2])
        write(dev,self.REG_SERVO_C_SHOW,show[2])
        write(dev,self.REG_SERVO_D_HIDE,hide[3])
        write(dev,self.REG_SERVO_D_SHOW,show[3])
        write(dev,self.REG_EESAVE,1)

    def factory_reset(self,dev):
        write(dev,self.REG_EEINIT,1)

    def freeze_one(self,dev):
        write(dev,self.REG_MODE,2)
        
    def test_one(self,dev):
        choice=0
        while True:    
            write(dev,self.REG_MODE,2)
            time.sleep(0.1)
            write(dev,self.REG_MODE,1)
            time.sleep(0.05)
            write(dev,self.REG_SHOW_ID,choice)
            print(choice)
            choice+=1
            if choice>3: choice=0
            time.sleep(1)

    def test_all(self):
        choice=0
        dev=0
        while True:    
            dev=dev+1
            if dev>=len(self.addresses):
                dev=0
                choice+=1
                if choice>3: choice=0

            # clear just in case
            #write(self.addresses[dev],self.REG_MODE,2)
            #time.sleep(0.1)
            #write(self.addresses[dev],self.REG_MODE,1)
            #time.sleep(0.05)
            write(self.addresses[dev],self.REG_SHOW_ID,random.randrange(0,5))
            time.sleep(1)
            
    def flash_all(self,count):
        choice=0
        dev=0
        c=0
        while c<count:    
            dev=dev+1
            if dev>=len(self.addresses):
                dev=0
                choice+=1
                if choice>1: choice=0
            write(self.addresses[dev],self.REG_LED,choice)
            time.sleep(0.1)


    def cali(self):
        print("recalibrating")
        if 0x0a in self.addresses:
            self.calibrate(0x0a,[15,90,5,45],[60,50,60,0])  # <- dodgy plug a
        if 0x0b in self.addresses:
            self.calibrate(0x0b,[20,85,5,70],[65,40,50,10])
        if 0x0c in self.addresses:
            self.calibrate(0x0c,[5,85,10,50],[45,40,60,0])       
        if 0x0d in self.addresses:
            self.calibrate(0x0d,[15,95,10,50],[60,50,50,0]) # <- bit iffy
        if 0x0e in self.addresses:
            self.calibrate(0x0e,[10,80,10,50],[60,35,55,0])
        if 0x0f in self.addresses:
            self.calibrate(0x0f,[20,85,5,50],[70,40,55,0])
        if 0x10 in self.addresses:
            self.calibrate(0x10,[15,85,10,55],[60,30,60,0]) # <- fixed 4 show distance
        if 0x11 in self.addresses:
            self.calibrate(0x11,[15,85,0,55],[60,40,40,0])       
        if 0x12 in self.addresses:
            self.calibrate(0x12,[20,85,0,60],[70,40,45,0])
        if 0x13 in self.addresses:
            self.calibrate(0x13,[25,85,0,70],[75,35,50,0])

    def reboot(self):
        print("power down")
        self.power_pin.off()
        time.sleep(5)
        print("power up")
        self.boot(False)
        
    def boot(self,do_cali=True):
        self.power_pin.on()
        time.sleep(5)
        print("scanning...")
        self.scan_addresses()
        if do_cali: self.cali()
        print("ready...")
        for addr in self.addresses:
            write(addr,self.REG_MODE,1)

    def clear_triangle(self,addr):
        ##write(addr,self.REG_MODE,2)
        #time.sleep(0.1)
        #write(addr,self.REG_MODE,1)
        write(addr,self.REG_SHOW_ID,6)

    def set_triangle(self,addr,shape_id):
        global do_reboot
        # clear just in case
        write(addr,self.REG_MODE,2)
        time.sleep(0.2)
        write(addr,self.REG_MODE,1)
        write(addr,self.REG_SHOW_ID,shape_id)
        if do_reboot:
            print("rebooting")
            self.reboot()
            do_reboot = False
        #time.sleep(0.5)
        #print("setting "+str(addr)+" to "+str(shape_id))

    def shape_to_id(self,shape_str):
        if shape_str=="squ": return 0
        if shape_str=="gui": return 1
        if shape_str=="tri": return 2
        if shape_str=="cir": return 3
        return 4

    def clear_shapes(self):
        print("clearing shapes...")
        for addr in self.addresses:
            self.clear_triangle(addr)

    
    def distribute_shapes(self,shapes):               
        global do_reboot
        if len(self.addresses)<1:
            print("rebooting")
            self.reboot()
            return
        tris_per_shape = int(len(self.addresses)/len(shapes))
        if tris_per_shape<1: tris_per_shape=1
        tris = copy.copy(self.addresses)
        for shape in shapes:
            for tri_num in range(0,tris_per_shape):
                if len(tris)>0:
                    choice = tris[random.randrange(0,len(tris))]
                    tris.remove(choice)
                    if self.state[choice]!=shape:
                        self.set_triangle(choice,self.shape_to_id(shape))
                        self.state[choice]=shape
                        #time.sleep(0.1)
                    if do_reboot:
                        print("rebooting")
                        self.reboot()
                        do_reboot = False
            

        
