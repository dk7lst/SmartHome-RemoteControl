#include <M5EPD.h>

M5EPD_Canvas canvas(&M5.EPD);

void setup() {
  M5.begin(true, false, false, true, true);
  M5.EPD.SetRotation(90);
  M5.EPD.Clear(true);

  canvas.createCanvas(540, 960);
  canvas.setTextSize(3);
  canvas.drawString("Hello World 123!", 50, 350);
  canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
}

void loop() {
}
