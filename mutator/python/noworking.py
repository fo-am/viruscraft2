import smbus, time

ADDR = 0x29
REG_MODEL_ID = 0xc0
REG_REV_ID = 0xc2
REG_SYSRANGE_START = 0x00
REG_SEQUENCE_CONFIG = 0x01
REG_RANGE_STATUS = 0x14
REG_I2C_MODE = 0x88
REG_GPIO_CONFIG = 0x0a
REG_DEVICE_ADDRESS = 0x8a
REG_VHV_EXTSUP = 0x89
REG_MSRC_CONFIG_CONTROL = 0x60
REG_RESULT_STATUS = 0x13
REG_INTERRUPT_CLEAR = 0x0b
bus = smbus.SMBus(1)

def write(reg,val):
    bus.write_byte_data(ADDR,reg,val)
    time.sleep(0.05)

def read(reg):
    v = bus.read_byte_data(ADDR,reg)
    time.sleep(0.05)
    return v

def write_fast(reg,val):
    bus.write_byte_data(ADDR,reg,val)
    
def read_fast(reg):
    v = bus.read_byte_data(ADDR,reg)
    return v

print("model should be 0xee = 0x%02x"%(read(REG_MODEL_ID),))
print("device address 0x%02x"%(read(REG_DEVICE_ADDRESS),))

global stop_variable
stop_variable = 0

def data_init():
    # switch to 2.8v mode i/o
    v = read(REG_VHV_EXTSUP)
    print("extsup 0x%02x"%(v,))
    v = (v&0xfe)|0x01
    write(REG_VHV_EXTSUP,v)
    print("extsup now 0x%02x"%(read(REG_VHV_EXTSUP),))

    write(0x88,0x00) # i2c standard mode
    write(0x80,0x01)
    write(0xff,0x01)
    write(0x00,0x00)
    stop_variable = read(0x91)
    print("stop_variable is 0x%02x"%(stop_variable,))
    #write(0x91,0x3c)
    write(0x00,0x01)
    write(0xff,0x00)
    write(0x80,0x00)

    # disable limit checks
    write(REG_MSRC_CONFIG_CONTROL,read(REG_MSRC_CONFIG_CONTROL)|0x12)

    write(REG_SEQUENCE_CONFIG,0xff)
    write(REG_SEQUENCE_CONFIG,0xe8)
    

def start(oneshot):

    write(0x80,0x01)
    write(0xff,0x01)
    write(0x00,0x00)
    write(0x91,stop_variable)
    write(0x00,0x01)
    write(0xff,0x00)
    write(0x80,0x00)
    

    if oneshot:
        print("oneshot mode")
        write_fast(REG_SYSRANGE_START,0x01)
        #print(read_fast(REG_SYSRANGE_START))
        print("%02X"%(read_fast(REG_RESULT_STATUS),))
        count = 0
        val = 0
        while count<100:
            val = read(REG_SYSRANGE_START)
            print(val)
            if not val & 0x01: count=100
            count+=1
        if not val & 0x01:
            print("ready")
        else:
            print("not ready")
        read_result()
    else:
        print("continuous mode")
        write(REG_SYSRANGE_START,0x02)


def read_result():
    while (read(REG_RESULT_STATUS)&0x07)==0:
        print(read(REG_RESULT_STATUS))
        pass
    pos=0
    d=[]
    while pos<12:
        d.append(read(REG_RANGE_STATUS+pos))
        pos+=1
    write(REG_INTERRuPT_CLEAR,0x01)
    return d


data_init()
start(True)

#while True:
#    print(read_result())
    
print("model id 0x%02x"%(read(REG_MODEL_ID),))
print("device address 0x%02x"%(read(REG_DEVICE_ADDRESS),))
