#include <st7789v2.h>
#include "SPI.h"
#include "seeed.h"

st7789v2 Display;

void setup() {
  // put your setup code here, to run once:
  Display.SetRotate(0);
  Display.Init();
  Display.SetBacklight(100);
  Display.Clear(WHITE);
}

void loop() {
  // put your main code here, to run repeatedly:
  Display.DrawString_EN(0, 50, "  Heart Rate ", &Font24, WHITE, RED);
  Display.DrawImage(gImage_seeed, 0, 140, 240, 280);


//  Display.DrawNum(100, 220, 123456, &Font24, RED, BRED);
  Display.DrawFloatNum(80, 80, 72.00, 2, &Font24, BLACK, WHITE);
  Display.DrawString_EN(70, 110, "  BPM   ", &Font24, WHITE, RED);

  
}
