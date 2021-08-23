import asyncio
import websockets
import smbus
import time
from gpiozero import LED

bus = smbus.SMBus(1)
power_pin = LED(14)

power_pin.on()

async def read(dev,reg):
    try:
        v = bus.read_byte_data(dev,reg)
        await asyncio.sleep(0.05)
        return v
    except:
        print("i2c error reading: "+str(dev))
        return 0
    
async def loop(websocket,path):
    print("starting...")
    devices = {
#        "squ":0x70,
        "gui":0x71,
        "tri":0x72,
        "cir":0x73
    }
    err_count = {
        #        0x70:0,
        0x71:0,
        0x72:0,
        0x73:0,
    }
    ignore = {}
    debounce_addr = 0x01
    last_sent_none = False
    running = True
    while running:
        found_event = False
        for shape,i2c in devices.items():
            if not i2c in ignore:
                r = await read(i2c,debounce_addr)
                if r==1:
                    found_event = True
                    last_sent_none = False
                    try:
                        await websocket.send(shape)
                    except websockets.exceptions.ConnectionClosed as e:
                        running = False

                if r!=0 and r!=1:
                    # if we get a dodgy value, add this to the
                    # ignore list for 8 seconds in order to force
                    # it to reset itself on the other end
                    err_count[i2c]+=1
                    if err_count[i2c]>100:
                        print(str(r)+" <- ignoring "+str(i2c))
                        err_count[i2c]=0
                        ignore[i2c]=time.time()
                else:
                    err_count[i2c]=0

        if not found_event and not last_sent_none:
            await websocket.send("none")
            last_sent_none=True

        # clear out the ignored sensors to see if they are back up
        max_ignore=8
        to_remove = []
        for i2c,t in ignore.items():
            now = time.time()
            if now-t>max_ignore:
                to_remove.append(i2c)
        for i2c in to_remove:                
            ignore.pop(i2c)

#start_server = websockets.serve(loop, "192.168.1.1", 8890)
start_server = websockets.serve(loop, "192.168.1.83", 8890)
asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()


