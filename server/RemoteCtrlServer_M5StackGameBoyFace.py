#!/usr/bin/python3
import time
import socket

# Send command via the IRTrans IR remote transceiver as an IR remote control code.
# The IRTrans has a database that is used to map remote control names and device commands to IR codes.
# The packet is sent over the main socket so that the responses (starting with "**") are received below.
def sendIR(remoteName, remoteCmd = ""):
  sock.sendto(bytes("snd " + remoteName + "," + remoteCmd, "utf-8"), ("irtrans", 21000))

def sendMenu(title, items):
  sock.sendto(bytes("MENU\n" + title + "\n" + "\n".join(items), "utf-8"), addr)

def sendToast(title, text):
  sock.sendto(bytes("TOAST\n" + title + "\n" + text, "utf-8"), addr)

def sendLevel(title, text, level):
  sock.sendto(bytes("LEVEL\n" + title + "\n" + text + "\n" + str(max(min(level, 100), 0)), "utf-8"), addr)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", 21001))

M5addr = None

while True:
  data, addr = sock.recvfrom(1024)
  data = data.decode('utf-8').replace("\n", "")
  print("received message: \"%s\"" % data)
  if data.startswith("**"):
    addr = M5addr # Do not send response to IRTrans but to the last known address of the M5Stack
    if M5addr != None: sendToast("IRTrans", data[8:])
    M5addr = None
    continue

  if data == "INITREQ": sock.sendto(b"HOMESCREENBUTTONDEF\nScenes # ~ # Devices # ~ # _cfg|Cfg # _pwroff|OFF", addr)
  elif data == "BTN Scenes": sendMenu("Scenes", ["IR:TSR700,Scene1|FireTV Cube", "Scene_FTVold|FireTV (old)", "Scene_Bluray|Bluray", "UHD-Bluray", "IR:TSR700,Scene2|Radio SSL", "IR:TSR700,Bluetooth|Bluetooth", "All Off"])
  elif data == "BTN Devices": sendMenu("Devices", ["AVR", "Bluray Player", "UHD Player", "Projector", "Lights", "Fan", "Screen"])
  elif data == "MENUITEM AVR": sendMenu("AVR", ["IR:TSR700,Power|Power", "IR:TSR700,PowerZone2|Power Zone 2", "IR:TSR700,Mute|Mute"])
  elif data == "MENUITEM Bluray Player": sendMenu("AVR", ["IR:BDMP1,Power|Power", "IR:BDMP1,Stop|Stop", "IR:BDMP1,OpenClose|Open/Close Tray"])
  elif data == "MENUITEM Projector": sendMenu("Projector", ["IR:HU70LS,Power|Power"])
  elif data.startswith("MENUITEM IR:"):
    sendIR(data[12:]) # Send directly via IR
    M5addr = addr # Send IRTrans reply only for generic commands so as not to overwrite the command-specific toast
  elif data == "BTN left":
    sendToast("AVR", "Previous Input")
    sendIR("TSR700", "InputPrev")
  elif data == "BTN right":
    sendToast("AVR", "Next Input")
    sendIR("TSR700", "InputNext")
  elif data == "BTN up":
    sendToast("AVR", "Volume Up")
    sendIR("TSR700", "VolumeUp")
  elif data == "BTN down":
    sendToast("AVR", "Volume Down")
    sendIR("TSR700", "VolumeDown")
  elif data == "BTN start":
    sendToast("BD Player", "BD Play")
    sendIR("BDMP1", "Play")
  elif data == "BTN select":
    sendToast("BD Player", "BD Pause")
    sendIR("BDMP1", "Pause")
  elif data == "MENUITEM Scene_Bluray":
    sendToast("AVR", "Switch to Bluray")
    sendIR("TSR700", "Scene1")
    time.sleep(.5)
    sendIR("TSR700", "InputNext")
  elif data == "MENUITEM Scene_FTVold":
    sendLevel("AVR", "Switch to FireTV (old)", 0)
    sendIR("TSR700", "Scene1")
    time.sleep(.5)
    sendLevel("AVR", "Switch to FireTV (old)", 50)
    sendIR("TSR700", "InputNext")
    time.sleep(.5)
    sendIR("TSR700", "InputNext")
    sendLevel("AVR", "Switch to FireTV (old)", 100)
