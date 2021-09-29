#pragma once

class M5EPD_Canvas;

enum MessageIds {
  MSG_Init = 0,  // M5STACK -> PC
  MSG_ScreenUpdate, // PC -> M5STACK
  MSG_TouchEvent // M5STACK -> PC
};

#pragma pack(1)
struct PacketHeader {
  uint16_t magic; // Magic number to check packet is really for us and stream is still in sync - always 0x48F3
  uint8_t msgid; // see MessageIds
};
#pragma pack()

void sendHeader(MessageIds msgid);
void sendTouchEventMessage(const tp_finger_t &finger);

void processScreenUpdateMessage(M5EPD_Canvas *canvas);
