#include "HomeSpan.h"

#define MIN_TEMP 18  // minimum allowed temperature in celsius
#define MAX_TEMP 25 // maximum allowed temperature in celsius

////////////////////////////////////////////////////////////////////////
struct Reference_Thermostat : Service::Thermostat
{
   // Create characteristics, set initial values, and set storage in NVS to true

   Characteristic::CurrentHeatingCoolingState currentState{0, true};
   Characteristic::TargetHeatingCoolingState targetState{0, true};
   Characteristic::CurrentTemperature currentTemp{22, true};
   Characteristic::TargetTemperature targetTemp{22, true};
   Characteristic::HeatingThresholdTemperature heatingThreshold{22, true};
   Characteristic::CoolingThresholdTemperature coolingThreshold{22, true};
   Characteristic::TemperatureDisplayUnits displayUnits{0, true}; // this is for changing the display on the actual thermostat (if any), NOT in the Home App

   Reference_Thermostat() : Service::Thermostat()
   {
      LOG1("*** Creating HomeSpan Thermostat***");

      currentTemp.setRange(MIN_TEMP, MAX_TEMP); // set all ranges the same to make sure Home App displays them correctly on the same dial
      targetTemp.setRange(MIN_TEMP, MAX_TEMP);
      heatingThreshold.setRange(MIN_TEMP, MAX_TEMP);
      coolingThreshold.setRange(MIN_TEMP, MAX_TEMP);
   }

   boolean update() override
   {
      if (targetState.updated())
      {
         switch (targetState.getNewVal())
         {
         case 0:
            LOG1("Thermostat turning OFF");
            break;
         case 1:
            LOG1("Thermostat set to HEAT at %s", temp2String(targetTemp.getVal<float>()).c_str());
            break;
         case 2:
            LOG1("Thermostat set to COOL at %s", temp2String(targetTemp.getVal<float>()).c_str());
            break;
         case 3:
            LOG1("Thermostat set to AUTO from %s to %s", temp2String(heatingThreshold.getVal<float>()).c_str(), temp2String(coolingThreshold.getVal<float>()).c_str());
            break;
         }

         // sendRequest();
      }

      if (heatingThreshold.updated() || coolingThreshold.updated())
         LOG1("Temperature range changed to %s to %s", temp2String(heatingThreshold.getNewVal<float>()).c_str(), temp2String(coolingThreshold.getNewVal<float>()).c_str());

      else if (targetTemp.updated())
         LOG1("Temperature target changed to %s", temp2String(targetTemp.getNewVal<float>()).c_str());

      if (displayUnits.updated())
         LOG1("Display Units changed to %c", displayUnits.getNewVal() ? 'F' : 'C');

      return (true);
   }

   // Here's where all the main logic exists to turn on/off heating/cooling by comparing the current temperature to the Thermostat's settings

   void loop() override
   {

      float temp = 22; //tempSensor.read(); // read temperature sensor (which in this example is just a dummy sensor)

      if (temp < MIN_TEMP) // limit value to stay between MIN_TEMP and MAX_TEMP
         temp = MIN_TEMP;
      if (temp > MAX_TEMP)
         temp = MAX_TEMP;

      if (currentTemp.timeVal() > 5000 && fabs(currentTemp.getVal<float>() - temp) > 0.25)
      { // if it's been more than 5 seconds since last update, and temperature has changed
         currentTemp.setVal(temp);
         LOG1("Current Temperature is now %s.", temp2String(currentTemp.getNewVal<float>()).c_str());
      }

      switch (targetState.getVal())
      {

      case 0:
         if (currentState.getVal() != 0)
         {
            LOG1("Thermostat OFF");
            currentState.setVal(0);
         }
         break;

      case 1:
         if (currentTemp.getVal<float>() < targetTemp.getVal<float>() && currentState.getVal() != 1)
         {
            LOG1("Turning HEAT ON");
            currentState.setVal(1);
         }
         else if (currentTemp.getVal<float>() >= targetTemp.getVal<float>() && currentState.getVal() == 1)
         {
            LOG1("Turning HEAT OFF");
            currentState.setVal(0);
         }
         else if (currentState.getVal() == 2)
         {
            LOG1("Turning COOL OFF");
            currentState.setVal(0);
         }
         break;

      case 2:
         if (currentTemp.getVal<float>() > targetTemp.getVal<float>() && currentState.getVal() != 2)
         {
            LOG1("Turning COOL ON");
            currentState.setVal(2);
         }
         else if (currentTemp.getVal<float>() <= targetTemp.getVal<float>() && currentState.getVal() == 2)
         {
            LOG1("Turning COOL OFF");
            currentState.setVal(0);
         }
         else if (currentState.getVal() == 1)
         {
            LOG1("Turning HEAT OFF");
            currentState.setVal(0);
         }
         break;

      case 3:
         if (currentTemp.getVal<float>() < heatingThreshold.getVal<float>() && currentState.getVal() != 1)
         {
            LOG1("Turning HEAT ON");
            currentState.setVal(1);
         }
         else if (currentTemp.getVal<float>() >= heatingThreshold.getVal<float>() && currentState.getVal() == 1)
         {
            LOG1("Turning HEAT OFF");
            currentState.setVal(0);
         }

         if (currentTemp.getVal<float>() > coolingThreshold.getVal<float>() && currentState.getVal() != 2)
         {
            LOG1("Turning COOL ON");
            currentState.setVal(2);
         }
         else if (currentTemp.getVal<float>() <= coolingThreshold.getVal<float>() && currentState.getVal() == 2)
         {
            LOG1("Turning COOL OFF");
            currentState.setVal(0);
         }
         break;
      }
   }

   // This "helper" function makes it easy to display temperatures on the serial monitor in either F or C depending on TemperatureDisplayUnits
   String temp2String(float temp)
   {
      String t = displayUnits.getVal() ? String(round(temp * 1.8 + 32.0)) : String(temp);
      t += displayUnits.getVal() ? " F" : " C";
      return (t);
   }
} ReThermostat;