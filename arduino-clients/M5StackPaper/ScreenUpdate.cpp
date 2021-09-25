#include <M5EPD.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "ScreenUpdate.h"

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
#pragma pack()

void cmdScreenUpdate(M5EPD_Canvas *canvas) {
  ScreenUpdateHeader suh;
  tcp.readBytes((byte *)&suh, sizeof suh);
  if (suh.flags & SUH_FLAGS_CLEARSCREEN) canvas->fillCanvas(0);
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
