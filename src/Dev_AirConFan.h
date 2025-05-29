#include "AirConConfig.h"
#include "AirConData.h"
#include "HomeSpan.h"

struct AirConFan : Service::Fan
{
   Characteristic::Active onOff { Characteristic::Active::INACTIVE, true };
   Characteristic::CurrentFanState currentFanState { Characteristic::CurrentFanState::INACTIVE, true };
   Characteristic::TargetFanState targetFanState { Characteristic::TargetFanState::AUTO, true };
   Characteristic::RotationSpeed fanSpeed { 0, true };
   Characteristic::ConfiguredName confName { "Fan Speed" };

   AirConFan() : Service::Fan()
   {
      LOG1("*** Creating HomeSpan AirconFan Fan ***\n");
      fanSpeed.setRange(0, 100, 25);
   }

   void loop() override
   {
      
   }
};