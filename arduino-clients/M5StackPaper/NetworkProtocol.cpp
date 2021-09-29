#include <M5EPD.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "NetworkProtocol.h"

extern WiFiClient tcp;

enum SUH_FLAGS {
  SUH_FLAGS_CLEARSCREEN = 1
};

#pragma pack(1)
struct ScreenUpdateHeader {
  uint16_t x, y, width, height; // width and height can be 0 to only clear screen or flush existing canvas. x and width must be even numbers!
  uint8_t screenUpdateMode; // see m5epd_update_mode_t. Set to UPDATE_MODE_NONE to skip update.
  uint8_t flags; // See SUH_FLAGS.
};

struct TouchEventMessage {
  uint16_t x, y, size;

  void set(const tp_finger_t &finger) {
    x = finger.x;
    y = finger.y;
    size = finger.size;
  }
};
#pragma pack()

void sendHeader(MessageIds msgid) {
  PacketHeader hdr;
  hdr.magic = 0x48F3;
  hdr.msgid = msgid;
  tcp.write((byte *)&hdr, sizeof hdr);
}

void sendTouchEventMessage(const tp_finger_t &finger) {
  sendHeader(MSG_TouchEvent);
  TouchEventMessage tu;
  tu.set(finger);
  tcp.write((byte *)&tu, sizeof tu);
}

void sendButtonEventMessage(int button) {
  sendHeader(MSG_ButtonEvent);
  tcp.write((byte *)&button, 1); // Only send low-byte
}

void processScreenUpdateMessage(M5EPD_Canvas *canvas) {
  ScreenUpdateHeader suh;
  tcp.readBytes((byte *)&suh, sizeof suh);
  if (suh.flags & SUH_FLAGS_CLEARSCREEN) {
    canvas->fillCanvas(0);
    canvas->setCursor(0, 0);
  }
  if (suh.width > 0 && suh.height > 0) {
    const int bytesPerLine = M5EPD_PANEL_H >> 1;
    byte *line = (byte *)canvas->frameBuffer() + suh.y * bytesPerLine + (suh.x >> 1);
    int linesLeft = suh.height;
    while (--linesLeft >= 0) {
      tcp.readBytes(line, suh.width >> 1);
      line += bytesPerLine;
    }
  }
  if (suh.screenUpdateMode != UPDATE_MODE_NONE) canvas->pushCanvas(0, 0, (m5epd_update_mode_t)suh.screenUpdateMode);
}
