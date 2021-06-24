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
        "squ":0x70,
        "gui":0x71,
        "tri":0x72,
        "cir":0x73
        }
    debounce_addr = 0x01
    last_sent_none = False
    while True:
        found_event = False
        for shape,i2c in devices.items():
            r = await read(i2c,debounce_addr)
            if r==1:
                found_event = True
                last_sent_none = False
                await websocket.send(shape)
                
        if not found_event and not last_sent_none:
            await websocket.send("none")
            last_sent_none=True
  
start_server = websockets.serve(loop, "192.168.1.1", 8890)
asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()


