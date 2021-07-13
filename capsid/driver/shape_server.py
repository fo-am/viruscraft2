#!/usr/bin/env python

import SimpleHTTPServer
import SocketServer
import os
import time
import random
from driver import robot_virus

global transmitter

# sudo lsof -i:8888 

virus = robot_virus()

virus.boot(True)

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

        if self.path=="/test_shapes":
            virus.clear_shapes()
            shapes = ["squ","tri","gui","cir"]
            random.shuffle(shapes)
            for shape in shapes:
                virus.distribute_shapes([shape])
                time.sleep(2)
                
            virus.clear_shapes()
            self.send_response(200)
            return

        if self.path=="/halt":
            os.system("sudo halt")
        
        if self.path=="/shapes":
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

