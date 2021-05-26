#!/usr/bin/env python

import SimpleHTTPServer
import SocketServer
import os
import time

bus = smbus.SMBus(1)

# sudo lsof -i:8888 

def read(dev,reg):
    try:
        v = bus.read_byte_data(dev,reg)
        time.sleep(0.05)
        return v
    except:
        print("i2c error reading: "+str(dev))
        return 0

class server(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def send_response(self, *args, **kwargs):
        SimpleHTTPServer.SimpleHTTPRequestHandler.send_response(self, *args, **kwargs)
        self.send_header('Access-Control-Allow-Origin', '*')

    def do_OPTIONS(self):
        self.send_response(200)
        
    def do_POST(self):
        if self.path=="/ping":
            self.send_response(200)
            return

        if self.path=="/sensors":
            data_len = int(self.headers['Content-Length'])
            data_string = self.rfile.read(data_len)
            virus.distribute_shapes(data_string.strip().split(' '))
            self.send_response(200)
            return

        self.send_response(200)
        
    def do_GET(self):
        return SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)


    
PORT = 8889
SocketServer.TCPServer.allow_reuse_address = True
httpd = SocketServer.TCPServer(("", PORT), server)
httpd.serve_forever()

