#include <M5Stack.h>
#include <M5ezModified.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "secrets.h"

char *getLine(bool allowNULL, char *str = NULL);

WiFiUDP udp;
boolean connected = false;
String homeScreenButtonDef = "_initreq|Init # ~ # ~ # ~ # _cfg|Cfg # _pwroff|OFF";
unsigned long returnToHomescreenTime = 0;

void setup() {
  //M5.begin();
  //M5.Power.begin();
  ez.begin();

  WiFi.disconnect(true);
  WiFi.onEvent(WiFiEventHandler);
  WiFi.begin(ssid, password);

  showHomeScreen();

  ez.clock.tz.setLocation("Europe/Berlin"); // Does not work, remains UTC
}

void loop() {
  // Soft buttons below the display:
  String btn = ez.buttons.poll();
  if (btn == "_initreq") sendStr("INITREQ");
  else if (btn == "_cfg") {
    ez.settings.menu();
    showHomeScreen();
  }
  else if (btn == "_pwroff") M5.powerOFF();
  else if (btn.length() > 0) sendStr("BTN " + btn);

  // Received UDP packets:
  int size = udp.parsePacket();
  if (size > 0) {
    char rxBuf[1024];
    int len = udp.read(rxBuf, sizeof rxBuf);
    rxBuf[len] = 0;
    char *p = getLine(false, rxBuf);
    if (!strcmp(p, "MSG")) { // Simple dialogs with OK button
      ez.msgBox("Message", getLine(false), "Ok");
      showHomeScreen();
    }
    else if (!strcmp(p, "MSGBOX")) { // Dialogs with adjustable header, text and buttons that send back the selected button
      sendStr("MSGBOXBTN " + ez.msgBox(getLine(false), getLine(false), getLine(false)));
      showHomeScreen();
    }
    else if (!strcmp(p, "TOAST")) { // Short, non-blocking text overlays
      ez.msgBox(getLine(false), getLine(false), homeScreenButtonDef, false);
      returnToHomescreenTime = millis() + 2000;
    }
    else if (!strcmp(p, "LEVEL")) { // Progress bar to show things like volume as bars
      ezProgressBar pb(getLine(false), getLine(false), homeScreenButtonDef);
      pb.value(atof(getLine(false)));
      returnToHomescreenTime = millis() + 2000;
    }
    else if (!strcmp(p, "HOMESCREENBUTTONDEF")) { // Define home screen buttons
      homeScreenButtonDef = getLine(false);
      showHomeScreen();
    }
    else if (!strcmp(p, "MENU")) { // Show menu
      String menuName = getLine(false);
      ezMenu menu(menuName);
      menu.buttons("up # Back # select # ~ # down # ~");
      while (p = getLine(true)) menu.addItem(p);
      if (menu.runOnce() > 0) sendStr("MENUITEM " + menu.pickName());
      else sendStr("MENUEXIT " + menuName);
      showHomeScreen();
    }
    else if (!strcmp(p, "PING")) sendStr("PONG"); // Connection check by server
  }

  // After toasts or the like, automatically return to the home screen after the time has elapsed:
  if (returnToHomescreenTime > 0 && millis() >= returnToHomescreenTime) showHomeScreen();
}

char *getLine(bool allowNULL, char *str) {
  char *p = strtok(str, "\n");
  return p || allowNULL ? p : (char *)"";
}

void showHomeScreen() {
  ez.canvas.reset();
  ez.header.show("LSNET Remote");
  ez.buttons.show(homeScreenButtonDef);
  ez.canvas.font(&FreeMono9pt7b);
  ez.canvas.printf("Battery: %d%%", M5.Power.getBatteryLevel());
  ez.canvas.println(); // "\n" is ignored by printf()?
  if (connected) {
    ez.canvas.printf("Connected to WiFi \"%s\"", ssid);
    ez.canvas.println();
    ez.canvas.print("IP: ");
    ez.canvas.println(WiFi.localIP());
  }
  else ez.canvas.println("No WiFi connection!");
  ez.canvas.println();
  if (M5.Power.isCharging()) ez.canvas.println("Charging...");
  returnToHomescreenTime = 0;
}

void sendStr(const String str) {
  if(connected) {
    udp.beginPacket("s0", 21001);
    udp.printf("%s\n", str.c_str());
    udp.endPacket();
  }
}

void WiFiEventHandler(WiFiEvent_t event) {
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      udp.begin(WiFi.localIP(), 8000);
      connected = true;
      showHomeScreen();
      sendStr("INITREQ");
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      connected = false;
      showHomeScreen();
      break;
  }
}
