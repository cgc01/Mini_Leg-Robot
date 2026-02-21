#include <Arduino.h>
#include "Start.h"

void setup() {
  // put your setup code here, to run once:
  Start_SetUpAll();
}

void loop() {
  // put your main code here, to run repeatedly:
  vTaskDelay(pdMS_TO_TICKS(1000000));
}