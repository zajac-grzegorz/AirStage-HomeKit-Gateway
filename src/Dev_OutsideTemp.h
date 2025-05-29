#include "AirConConfig.h"
#include "AirConData.h"
#include "HomeSpan.h"

struct OutsideTemperature : Service::TemperatureSensor
{
   Characteristic::CurrentTemperature outsideTemp { 20.0, true };
   Characteristic::ConfiguredName confName { "Outside Temp" };

   OutsideTemperature() : Service::TemperatureSensor()
   {
      LOG1("*** Creating HomeSpan OutsideTemperature Sensor ***\n");
   }

   void loop() override
   {
      // If it's been more than X seconds since last update, and temperature has changed
      if (outsideTemp.timeVal() > AIRCON_UPDATE_TIME_ELAPSED)
      {
         // Normalize and make it float
         float outTemp = (acOutsideTemp.load() - AIRCON_TEMP_MODIFIER) / AIRCON_TEMP_DIVIDER;

         if (fabs(outsideTemp.getVal<float>() - outTemp) > AIRCON_UPDATE_DELTA)
         {
            outsideTemp.setVal(outTemp);
            LOG1("Outside Temperature is now %.2f\n", outsideTemp.getNewVal<float>());
         }
      }
   }
};