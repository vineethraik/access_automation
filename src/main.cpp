#ifndef ARDUINO_H
#include <Arduino.h>
#include <DNSServer.h>
#define ARDUINO_H
#endif
using namespace std;
#include "AcessOS.h"

void setup()
{
  Serial.begin(115200);
  timer.delay(500);
  // Serial.println("Hello World");
  accessOS.init();
  accessOS.FPdeviceInfo();

  // timer.set_new_event(5000, 1000, []()
  //                     {
  //    Serial.println("Hello World");
  //   return true; });
  timer.set_loop_function(
      []()

      {
        if (accessOS.osState == OS_STATE_IDLE)
        {
          // accessOS.handle();
          // Serial.println(accessOS.button_0);
          switch (accessOS.button_0_press_count)
          {
          case 1:
            Serial.println("register new");

            accessOS.registerNewID();
            break;
          case 2:
            Serial.println("device info");
            accessOS.FPdeviceInfo();

            break;
          case 3:
            Serial.println("Clearing templates");
            accessOS.clearTemplates();
            break;

          default:
            break;
          }
          digitalWrite(LED_BUILTIN, accessOS.button_0);
          if (accessOS.button_up_press_count > 0)
          {
            Serial.println("up:" + String(accessOS.button_up_press_count));
            accessOS.button_up_press_count =0;
          }
          if (accessOS.button_down_press_count > 0)
          {
            
            Serial.println("down:" + String(accessOS.button_down_press_count));
            accessOS.button_down_press_count = 0;
          }
          if (accessOS.button_right_press_count > 0)
          {
            
            Serial.println("right:" + String(accessOS.button_right_press_count));
            accessOS.button_right_press_count = 0;
          }
          if (accessOS.button_left_press_count > 0)
          {
            
            Serial.println("left:" + String(accessOS.button_left_press_count));
            accessOS.button_left_press_count = 0;
          }
        }
      });

  TaskHandle_t loops;

  xTaskCreatePinnedToCore(
      [](void *p)
      {while (true)
  {
    if (accessOS.osState == OS_STATE_IDLE)
    {
      accessOS.handle();
    }} }, "runtasks", 5000, NULL, 0, &loops, !xPortGetCoreID());
}

void loop()
{

  timer.delay(1000000);
}

// void setup()
// {
//   Serial.begin(115200);
//   Serial.print("setup() running on core ");
//   Serial.println(xPortGetCoreID());
// }

// void loop()
// {
//   Serial.print("loop() running on core ");
//   Serial.println(xPortGetCoreID());
// }