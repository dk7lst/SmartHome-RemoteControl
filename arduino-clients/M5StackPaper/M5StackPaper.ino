#include <M5EPD.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "secrets.h"

M5EPD_Canvas canvas(&M5.EPD);
//WiFiUDP udp;
boolean connected = false;
boolean connectedOld = true;

void setup() {
  M5.begin(true, false, false, true, true);
  M5.EPD.SetRotation(90);
  M5.EPD.Clear(true);

  M5.disableEXTPower(); // Switch off power to external ports

  canvas.createCanvas(540, 960);
  canvas.setTextSize(10);
  canvas.printf("Booting...\n");
  canvas.setTextSize(4);
  canvas.printf("Bat: %f V\n", M5.getBatteryVoltage() / 1000.0);
  canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);

  WiFi.disconnect(true);
  WiFi.onEvent(WiFiEventHandler);
  WiFi.begin(ssid, password);
}

void loop() {
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
  if (connected != connectedOld) {
    connectedOld = connected;
    canvas.setTextSize(4);
    canvas.printf("WiFi: %s\n", connected ? "CONNECTED" : "DISCONNECTED");
    canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
  }
  M5.update();
  delay(100);
}

void WiFiEventHandler(WiFiEvent_t event) {
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      //udp.begin(WiFi.localIP(), 8000);
      connected = true;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      connected = false;
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
    M5.enableMainPower();
    canvas.setTextSize(5);
    canvas.drawString("USB connected?", 30, 600);
    canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
    delay(1000);
  }
}
