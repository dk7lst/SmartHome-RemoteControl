#!/usr/bin/python3
import os
import socketserver
import struct
import time
import cv2 # apt install python3-opencv

def expandImage(cvimage): # Add border if necessary to make image width an even number
  height, width = cvimage.shape
  if width % 2: cvimage = cv2.copyMakeBorder(cvimage, top=0, bottom=0, left=0, right=1, borderType=cv2.BORDER_CONSTANT, value=0) #cvimage = cv2.resize(cvimage, (216, 900))
  return cvimage

def sendHeader(sock, cmd):
  sock.send(struct.pack("<HB", 0x48F3, cmd))

def sendScreenUpdateRaw(sock, x, y, width, height, screenUpdateMode, flags, rawdata): # Send 4 Bit/Pixel raw data. x and width must be even numbers!
  sendHeader(sock, 1)
  sock.send(struct.pack("<HHHHBB", x, y, width, height, screenUpdateMode, flags) + rawdata)

def sendScreenUpdate(sock, x, y, screenUpdateMode, flags, cvimage):
  height, width = cvimage.shape
  rawdata = bytearray(width // 2 * height)
  for row in range(height):
    for col in range(width // 2):
      rawdata[row * width // 2 + col] = (cvimage[row, col * 2] & 0xF0) | (cvimage[row, col * 2 + 1] >> 4)
  sendScreenUpdateRaw(sock, x, y, width, height, screenUpdateMode, flags, rawdata)

class ConnectionHandler(socketserver.BaseRequestHandler):
  def handle(self):
    print("connected!");
    while True:
      hdr = self.request.recv(3)
      #print(hdr.hex())
      if len(hdr) == 0:
        print("Disconnect!")
        return
      if len(hdr) != 3:
        print("Header mismatch!")
        return
      magic, msgid = struct.unpack("<HB", hdr)
      if magic != 0x48F3:
        print("Header magic value mismatch!")
        return
      print("msgid=" + str(msgid))
      if msgid == 0: # Init
        #x = 100
        #y = 100
        #w = 540 - x * 2
        #h = 960 - y * 2
        #sendScreenUpdateRaw(self.request, x, y, w, h, 2, 1, os.urandom(w * h // 2))
        sendScreenUpdate(self.request, 100, 25, 2, 1, image)
      elif msgid == 2: # Touch-Event
        data = self.request.recv(6)
        x, y, size = struct.unpack("<HHH", data)
        print("Touch-Event: x=" + str(x) + " y=" + str(y) + " size=" + str(size))
      elif msgid == 3: # Button-Event
        data = self.request.recv(1)
        button = struct.unpack("<B", data)[0]
        print("Button-Event: " + str(button))

image = expandImage(cv2.imread("test.png", cv2.IMREAD_GRAYSCALE))
th, image = cv2.threshold(image, 1, 255, cv2.THRESH_BINARY);
height, width = image.shape
print("w=" + str(width) + " h=" + str(height))

socketserver.TCPServer.allow_reuse_address = True
socketserver.TCPServer(("0.0.0.0", 21001), ConnectionHandler).serve_forever();
