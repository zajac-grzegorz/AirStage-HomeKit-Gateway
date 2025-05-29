#include "AirConConfig.h"
#include "AirConData.h"
#include "HomeSpan.h"

struct FanOnlyModeSwitch : Service::Switch
{
   Characteristic::On onOff { Characteristic::On::OFF, true };
   Characteristic::ConfiguredName confName { "Fan Only Mode" };

   FanOnlyModeSwitch() : Service::Switch()
   {
      LOG1("*** Creating HomeSpan FanOnlyModeSwitch Switch ***\n");
   }

   void loop() override
   {
      
   }
};