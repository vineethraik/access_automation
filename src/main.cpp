#include <Arduino.h>
#include <vector>
using namespace std;
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
    // accessOS.handle(); 
  });
  accessOS.registerNewID();
}

void loop()
{
  
  timer.delay(1000000);
}
