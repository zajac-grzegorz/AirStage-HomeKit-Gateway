#include "AirConConfig.h"
#include "AirConData.h"
#include "HomeSpan.h"

struct InsideTemperature : Service::TemperatureSensor
{
   Characteristic::CurrentTemperature insideTemp { 20.0, true };
   Characteristic::ConfiguredName confName { "Inside Temp" };

   InsideTemperature() : Service::TemperatureSensor()
   {
      LOG1("*** Creating HomeSpan InsideTemperature Sensor ***\n");
   }

   void loop() override
   {
      // Normalize and make it float
      // float inTemp = (acInsideTemp.load() - AIRCON_TEMP_MODIFIER) / AIRCON_TEMP_DIVIDER; 

      // If it's been more than X seconds since last update, and temperature has changed
      // if (insideTemp.timeVal() > AIRCON_UPDATE_TIME_ELAPSED && fabs(insideTemp.getVal<float>() - inTemp) > AIRCON_UPDATE_DELTA)
      if (insideTemp.timeVal() > AIRCON_UPDATE_TIME_ELAPSED) 
      {
         // Normalize and make it float
         float inTemp = (acInsideTemp.load() - AIRCON_TEMP_MODIFIER) / AIRCON_TEMP_DIVIDER;

         if (fabs(insideTemp.getVal<float>() - inTemp) > AIRCON_UPDATE_DELTA)
         {
            insideTemp.setVal(inTemp);
            LOG1("Inside Temperature is now %.2f\n", insideTemp.getNewVal<float>());
         }
      }
   }
};