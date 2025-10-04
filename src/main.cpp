#include <Arduino.h>
#include <vector>

using namespace std;
#include "credentials.h"
#include "timer.h"
#include "AcessOS.h"

void setup()
{
  Serial.begin(115200);
  // Serial.println("Hello World");
  accessOS.init();
  accessOS.FPdeviceInfo();
  // timer.set_new_event(5000, 1000, []()
  //                     {
  //    Serial.println("Hello World");
  //   return true; });
  timer.set_loop_function([]()
                          {
                            if (accessOS.osState == OS_STATE_IDLE)
                            {
                              accessOS.handle();
                              // Serial.println(accessOS.button_0);
                              switch (accessOS.button_0_press_count)
                              {
                              case 1:
                              accessOS.registerNewID();
                                break;
                              case 2:
                              accessOS.FPdeviceInfo();
                                break;
                              case 3:
                              accessOS.clearTemplateData();
                              break;

                              default:
                                break;
                              }
                              digitalWrite(LED_BUILTIN, accessOS.button_0);
                            } });
  accessOS.registerNewID();
}

void loop()
{

  timer.delay(1000000);
}
