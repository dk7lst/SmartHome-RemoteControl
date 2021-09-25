#!/usr/bin/python3
import os
import socketserver
import struct
import time

class ConnectionHandler(socketserver.BaseRequestHandler):
  def handle(self):
    print("connected!");
    x = 0
    y = 0
    w = 960
    h = 540
    bytes = struct.pack("<HBHHHHBB", 0x48F3, 1, x, y, w, h, 2, 1) + os.urandom(int(w * h / 2))
    self.request.send(bytes)
    exit(0)

socketserver.TCPServer.allow_reuse_address = True
socketserver.TCPServer(("0.0.0.0", 21001), ConnectionHandler).serve_forever();
