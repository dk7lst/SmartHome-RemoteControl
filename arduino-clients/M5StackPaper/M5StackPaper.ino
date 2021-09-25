#include <M5EPD.h>
#include <WiFi.h>
#include <WiFiClient.h>

#include "ScreenUpdate.h"
#include "secrets.h"

M5EPD_Canvas canvas(&M5.EPD);
WiFiClient tcp;

enum CONNECTSTATE {CS_DISCONNECTED, CS_WIFICONNECTED, CS_TCPCONNECTED} constate = CS_DISCONNECTED, constateOld = CS_DISCONNECTED;

#pragma pack(1)
enum CMD {
  CMD_ScreenUpdate = 1
};

struct PacketHeader {
  uint16_t magic; // Magic number to check packet is really for us and stream is still in sync - always 0x48F3
  uint8_t cmd; // see CMD;
};
#pragma pack()

void setup() {
  M5.begin(true, false, false, true, true);
  M5.disableEXTPower(); // Switch off power to external ports

  M5.EPD.SetRotation(90);
  M5.EPD.Clear(true);

  canvas.createCanvas(540, 960);
  canvas.setTextSize(10);
  canvas.printf("Booting...\n");
  canvas.setTextSize(4);
  canvas.printf("Bat: %f V\n", M5.getBatteryVoltage() / 1000.0);
  canvas.printf("Free Heap: %d/%d KB\nFree PSRAM: %d/%d KB\n", ESP.getFreeHeap() / 1024, ESP.getHeapSize() / 1024, ESP.getFreePsram() / 1024, ESP.getPsramSize() / 1024);
  canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);

  WiFi.onEvent(WiFiEventHandler);
  WiFi.begin(ssid, password);
}

void loop() {
  M5.update();

  if (M5.BtnP.wasPressed()) powerOff();
  if (M5.BtnL.wasPressed()) {
    canvas.setTextSize(4);
    canvas.printf("Btn L\n");
    canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
  }
  if (M5.BtnR.wasPressed()) {
    canvas.setTextSize(4);
    canvas.printf("Btn R\n");
    canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
  }

  if (constate == CS_WIFICONNECTED && tcp.connect(serverhost, 21001)) constate = CS_TCPCONNECTED;
  if (constate == CS_TCPCONNECTED && !tcp.connected()) constate = CS_WIFICONNECTED;

  if (constate != constateOld) {
    constateOld = constate;
    canvas.setTextSize(4);
    switch(constate) {
      case CS_DISCONNECTED:
        canvas.printf("WiFi disconnected");
        break;
      case CS_WIFICONNECTED:
        canvas.printf("WiFi connected");
        canvas.print(" IP: "); canvas.println(WiFi.localIP());
        break;
      case CS_TCPCONNECTED:
        canvas.printf("TCP connected");
    }
    canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
  }

  if (tcp.available() >= sizeof (PacketHeader)) {
    PacketHeader hdr;
    tcp.readBytes((byte *)&hdr, sizeof hdr);
    if (hdr.magic == 0x48F3) {
      switch(hdr.cmd) {
        case CMD_ScreenUpdate: cmdScreenUpdate(&canvas); break;
      }
    }
  }
  else delay(100);
}

void WiFiEventHandler(WiFiEvent_t event) {
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      constate = CS_WIFICONNECTED;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      tcp.stop();
      constate = CS_DISCONNECTED;
      break;
  }
}

void powerOff() {
  while(true) {
    canvas.fillCanvas(0);
    canvas.setTextSize(10);
    canvas.drawString("[POWER OFF]", 30, 500);
    canvas.setTextSize(3);
    canvas.drawString("(Press side-button for 2s)", 30, 800);
    canvas.pushCanvas(0, 0, UPDATE_MODE_GLR16);
    M5.RTC.clearIRQ(); // Clear IRQ keeping master power on
    delay(1000); // Time for display refresh?
    M5.shutdown(0);
    delay(1000); // Should never be executed unless USB connected
    M5.enableMainPower(); // Re-enable power to prevent "USB connected?"-message to be displayed permanently when USB cable is removed while message is displayed
    canvas.setTextSize(5);
    canvas.drawString("USB connected?", 30, 600);
    canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
    delay(1000);
  }
}
