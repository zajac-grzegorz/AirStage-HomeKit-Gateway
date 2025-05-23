///////////////////////////////////////////////////////
//                                                   //
//   HomeSpan Reference Sketch: Thermostat Service   //
//                                                   //
///////////////////////////////////////////////////////

#include "HomeSpan.h"
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <WebServer.h>                    // include WebServer library
#include "AirConConfig.h"
#include "AirConData.h"
#include "AirConClient.h"

Preferences prefs;

WebServer webServer(AIRCON_WEBSERVER_PORT);                  // create WebServer on port 80

AirConClient* client = nullptr;

SpanCharacteristic* outsideTemp;
SpanCharacteristic* insideTemp;

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

      currentTemp.setRange(AIRCON_MIN_TEMP, AIRCON_MAX_TEMP); // set all ranges the same to make sure Home App displays them correctly on the same dial
      targetTemp.setRange(AIRCON_MIN_TEMP, AIRCON_MAX_TEMP);
      heatingThreshold.setRange(AIRCON_MIN_TEMP, AIRCON_MAX_TEMP);
      coolingThreshold.setRange(AIRCON_MIN_TEMP, AIRCON_MAX_TEMP);
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

         // client->getParams();
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

      float temp = acSetTemp.load() / 10.0; // make it float
      
      // limit value to stay between MIN_TEMP and MAX_TEMP
      temp = constrain(temp, AIRCON_MIN_TEMP, AIRCON_MAX_TEMP);

      // if (temp < MIN_TEMP) 
      //    temp = MIN_TEMP;
      // if (temp > MAX_TEMP)
      //    temp = MAX_TEMP;

      if (currentTemp.timeVal() > 5000 && fabs(currentTemp.getVal<float>() - temp) > 0.25)
      { // if it's been more than 5 seconds since last update, and temperature has changed
         currentTemp.setVal(temp);
         LOG1("Current Temperature is now %s.", temp2String(currentTemp.getNewVal<float>()).c_str());
      }

      float inTemp = (acInsideTemp.load() - 5000) / 100.0; // normalize and make it float
      inTemp = constrain(inTemp, AIRCON_MIN_TEMP, AIRCON_MAX_TEMP);

      if (insideTemp->timeVal() > 5000 && fabs(insideTemp->getVal<float>() - inTemp) > 0.25)
      { // if it's been more than 5 seconds since last update, and temperature has changed
         insideTemp->setVal(inTemp);
         LOG1("Inside Temperature is now %s.", temp2String(insideTemp->getNewVal<float>()).c_str());
      }

      float outTemp = (acOutsideTemp.load() - 5000) / 100.0; // normalize and make it float
      outTemp = constrain(outTemp, AIRCON_MIN_TEMP, AIRCON_MAX_TEMP);

      if (outsideTemp->timeVal() > 5000 && fabs(outsideTemp->getVal<float>() - outTemp) > 0.25)
      { // if it's been more than 5 seconds since last update, and temperature has changed
         outsideTemp->setVal(outTemp);
         LOG1("Outside Temperature is now %s.", temp2String(outsideTemp->getNewVal<float>()).c_str());
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
};

void statusCallback(HS_STATUS status)
{
   if (status == HS_PAIRED)
   {
      WEBLOG("Starting AsyncTCP client");
      client = new AirConClient();
      // client->start();
   }
}
////////////////////////////////////////////////////////////////////////

void getCharacteristicsCallback(const char* getCharList)
{
   homeSpan.processSerialCommand("@g");
}
////////////////////////////////////////////////////////////////////////

void setupWeb(int ev)
{
   WEBLOG("Starting web server");
   webServer.begin();

   webServer.on("/", []() 
   {
      prefs.begin("CUST_DATA", false);
      String addr = prefs.getString("acaddr", AIRCON_DEFAULT_URL);
      String port = prefs.getString("acport", AIRCON_DEFAULT_PORT);
      prefs.end();

      String content = "<html><body><form action='/setup' method='POST'>Setup connection with Air Conditioner<br><br>";
      content += "URL address of Air Con:<input type='text' name='AIRCONADDR' placeholder='" + addr + "'><br>";
      content += "URL port of Air Con:<input type='text' name='AIRCONPORT' placeholder='" + port + "'><br>";
      content += "<input type='submit' name='SUBMIT' value='Submit'></form><br>";
      webServer.send(200, "text/html", content);
   });

   webServer.on("/setup", []() 
   {
      if (webServer.hasArg("AIRCONADDR"))
      {
         prefs.begin("CUST_DATA", false);
         prefs.putString("acaddr", webServer.arg("AIRCONADDR"));
         prefs.putString("acport", webServer.arg("AIRCONPORT"));
         prefs.end();

         String content = "<html><body><p>URL address and port saved...</p>";
         webServer.send(200, "text/html", content);

         WEBLOG("Saving the aircon URL address (%s) and port (%s)", 
            webServer.arg("AIRCONADDR").c_str(), webServer.arg("AIRCONPORT").c_str());
      }
   });

   webServer.onNotFound([]() 
   {
      String message = "File Not Found\n\n";
      message += "URI: ";
      message += webServer.uri();
      message += "\nMethod: ";
      message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
      message += "\nArguments: ";
      message += webServer.args();
      message += "\n";
    
      for (uint8_t i = 0; i < webServer.args(); i++) {
        message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
      }
    
      webServer.send(404, "text/plain", message);
    });

}

void setup()
{
   delay(3000);
   Serial.begin(115200);

   homeSpan.setStatusCallback(statusCallback);
   homeSpan.setGetCharacteristicsCallback(getCharacteristicsCallback);
   homeSpan.setStatusPixel(35, 200.0, 100.0, 100.0);
   homeSpan.setControlPin(41, PushButton::TRIGGER_ON_LOW);
   homeSpan.setStatusAutoOff(120);
   homeSpan.setPortNum(8080);
   homeSpan.setConnectionCallback(setupWeb);
   homeSpan.setLogLevel(1);
   homeSpan.enableWebLog(20,"pool.ntp.org", "UTC", "weblog");           // creates a web log on the URL /HomeSpan-[DEVICE-ID].local:[TCP-PORT]/myLog
   homeSpan.enableWatchdog(30); // enable HomeSpan watchdog with timeout of 30 seconds
   homeSpan.setPairingCode(AIRCON_HOMEKIT_PAIRING_CODE); // set pairing code to 1980-1980
   homeSpan.begin(Category::Bridges, AIRCON_HOMEKIT_BRIDGE_NAME);

   new SpanUserCommand('g', "get all parameters from AirCon", [](const char *buf) { 
      client->start(); 
   });

   new SpanAccessory();
      new Service::AccessoryInformation();
         new Characteristic::Identify();

   new SpanAccessory();
      new Service::AccessoryInformation();
         new Characteristic::Identify();
         new Characteristic::Name("Fujitsu AirCon"); 
      
      new Reference_Thermostat();
      
      new Service::Switch();
         new Characteristic::On();
         new Characteristic::ConfiguredName("Dry Mode");

      new Service::Switch();
         new Characteristic::On();
         new Characteristic::ConfiguredName("Fan Only Mode");   

      new Service::Fan();                             // Create the Fan Service
         new Characteristic::Active();                // This Service requires the "Active" Characterstic to turn the fan on and off
         new Characteristic::CurrentFanState();
         new Characteristic::TargetFanState();
         new Characteristic::RotationSpeed();
         new Characteristic::ConfiguredName("Fan Speed");

      new Service::TemperatureSensor();
         insideTemp = new Characteristic::CurrentTemperature(21.0);
         new Characteristic::ConfiguredName("Inside Temp"); 
      
      new Service::TemperatureSensor();
         outsideTemp = new Characteristic::CurrentTemperature(18.0);
         new Characteristic::ConfiguredName("Outside Temp"); 
}

////////////////////////////////////////////////////////////////////////

void loop()
{
   homeSpan.poll();
   webServer.handleClient();           // handle incoming web server traffic
}

////////////////////////////////////////////////////////////////////////
