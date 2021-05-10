#!/usr/bin/env python

import SimpleHTTPServer
import SocketServer
import os
import time
from driver import robot_virus

global transmitter

# sudo lsof -i:8888 
# virus = robot_virus([0x0a,
#                      0x0d,
#                      0x0e,
#                      0x0f,
#                      #0x11,
#                      0x12])

virus = robot_virus([0x0c])

virus.boot()

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

        if self.path=="/shapes":
            data_len = int(self.headers['Content-Length'])
            data_string = self.rfile.read(data_len)
            print("data_string is: "+data_string)
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

