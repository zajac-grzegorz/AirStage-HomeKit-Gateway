#include "AirConConfig.h"
#include "AirConData.h"
#include "HomeSpan.h"

struct DryModeSwitch : Service::Switch
{
   Characteristic::On onOff { Characteristic::On::OFF, true };
   Characteristic::ConfiguredName confName { "Dry Mode" };

   DryModeSwitch() : Service::Switch()
   {
      LOG1("*** Creating HomeSpan DryModeSwitch Switch ***\n");
   }

   void loop() override
   {
      
   }
};