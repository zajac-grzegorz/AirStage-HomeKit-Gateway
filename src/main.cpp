#include "HomeSpan.h"
#include <AsyncTCP.h>
#include <Preferences.h>
#include <WebServer.h>
#include "AirConConfig.h"
#include "AirConData.h"
#include "AirConClient.h"
#include "AirConWebServer.h"
#include "Dev_AirConFan.h"
#include "Dev_AirConThermostat.h"
#include "Dev_DryModeSwitch.h"
#include "Dev_FanOnlyModeSwitch.h"
#include "Dev_InsideTemp.h"
#include "Dev_OutsideTemp.h"

// Create WebServer on port 80
AirConWebServer webServer(AIRCON_WEBSERVER_PORT);

AirConClient* client = nullptr;
AirConFan* fanService = nullptr;

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
   LOG1("Characteristics: %s\n", getCharList);
   
   LOG1("FOUND: %d\n", fanService->fanSpeed.foundIn(getCharList));

   homeSpan.processSerialCommand("@g");
}

void setupWeb(int ev)
{
   webServer.start();
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
      new DryModeSwitch();
      new FanOnlyModeSwitch();
      fanService = new AirConFan();
      new InsideTemperature();
      new OutsideTemperature();
}

void loop()
{
   homeSpan.poll();

   // Handle incoming web server traffic
   webServer.handleClient();
}
