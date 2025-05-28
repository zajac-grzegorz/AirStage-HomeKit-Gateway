///////////////////////////////////////////////////////
//                                                   //
//   HomeSpan Reference Sketch: Thermostat Service   //
//                                                   //
///////////////////////////////////////////////////////

#include "HomeSpan.h"
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <WebServer.h>
#include "AirConConfig.h"
#include "AirConData.h"
#include "AirConClient.h"
#include "DEV_AirConThermostat.h"
#include "Dev_InsideTemp.h"
#include "Dev_OutsideTemp.h"

Preferences prefs;

// Create WebServer on port 80
WebServer webServer(AIRCON_WEBSERVER_PORT);

AirConClient* client = nullptr;

void statusCallback(HS_STATUS status)
{
   if (status == HS_PAIRED)
   {
      WEBLOG("Starting AsyncTCP client");
      client = new AirConClient();
   }
}

void getCharacteristicsCallback(const char* getCharList)
{
   homeSpan.processSerialCommand("@g");
}

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
    
      for (uint8_t i = 0; i < webServer.args(); i++) 
      {
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

   // Creates a web log on the URL /HomeSpan-[DEVICE-ID].local:[TCP-PORT]/myLog
   homeSpan.enableWebLog(20,"pool.ntp.org", "UTC", "weblog");
   
   // Enable HomeSpan watchdog with timeout of 30 seconds
   homeSpan.enableWatchdog(30);
   homeSpan.setPairingCode(AIRCON_HOMEKIT_PAIRING_CODE);
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
      
      new AirConThermostat();
      
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

      new InsideTemperature();
      new OutsideTemperature();
}

void loop()
{
   homeSpan.poll();

   // Handle incoming web server traffic
   webServer.handleClient();
}
