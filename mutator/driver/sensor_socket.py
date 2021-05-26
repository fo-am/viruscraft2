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
    debounce_addr = 0x06
    while True:
        for shape,i2c in devices.items():
            if await read(i2c,debounce_addr)==1:
                await websocket.send(shape)
  
start_server = websockets.serve(loop, "192.168.1.68", 8890)
asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()


