///////////////////////////////////////////////////////
//                                                   //
//   HomeSpan Reference Sketch: Thermostat Service   //
//                                                   //
///////////////////////////////////////////////////////

#include "HomeSpan.h"
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <asyncHTTPrequest.h>

#include <WebServer.h>                    // include WebServer library
WebServer webServer(80);                  // create WebServer on port 80

#define MIN_TEMP 18  // minimum allowed temperature in celsius
#define MAX_TEMP 25 // maximum allowed temperature in celsius

// AsyncClient* tcpClient;

asyncHTTPrequest request;

////////////////////////////////////////////////////////////////////////

// Here we create a dummmy temperature sensor that can be used as a real sensor in the Thermostat Service below.
// Rather than read a real temperature sensor, this structure allows you to change the current temperature via the Serial Monitor

struct DummyTempSensor
{
   static float temp;

   DummyTempSensor(float t)
   {
      temp = t;
      new SpanUserCommand('f', "<temp> - set the temperature, where temp is in degrees F", [](const char *buf)
                          { temp = (atof(buf + 1) - 32.0) / 1.8; });
      new SpanUserCommand('c', "<temp> - set the temperature, where temp is in degrees C", [](const char *buf)
                          { temp = atof(buf + 1); });
   }

   float read() { return (temp); }
};

float DummyTempSensor::temp;

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

   DummyTempSensor tempSensor{22}; // instantiate a dummy temperature sensor with initial temp=22 degrees C

   Reference_Thermostat() : Service::Thermostat()
   {
      Serial.printf("\n*** Creating HomeSpan Thermostat***\n");

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
            Serial.printf("Thermostat turning OFF\n");
            break;
         case 1:
            Serial.printf("Thermostat set to HEAT at %s\n", temp2String(targetTemp.getVal<float>()).c_str());
            break;
         case 2:
            Serial.printf("Thermostat set to COOL at %s\n", temp2String(targetTemp.getVal<float>()).c_str());
            break;
         case 3:
            Serial.printf("Thermostat set to AUTO from %s to %s\n", temp2String(heatingThreshold.getVal<float>()).c_str(), temp2String(coolingThreshold.getVal<float>()).c_str());
            break;
         }
      }

      if (heatingThreshold.updated() || coolingThreshold.updated())
         Serial.printf("Temperature range changed to %s to %s\n", temp2String(heatingThreshold.getNewVal<float>()).c_str(), temp2String(coolingThreshold.getNewVal<float>()).c_str());

      else if (targetTemp.updated())
         Serial.printf("Temperature target changed to %s\n", temp2String(targetTemp.getNewVal<float>()).c_str());

      if (displayUnits.updated())
         Serial.printf("Display Units changed to %c\n", displayUnits.getNewVal() ? 'F' : 'C');

      return (true);
   }

   // Here's where all the main logic exists to turn on/off heating/cooling by comparing the current temperature to the Thermostat's settings

   void loop() override
   {

      float temp = tempSensor.read(); // read temperature sensor (which in this example is just a dummy sensor)

      if (temp < MIN_TEMP) // limit value to stay between MIN_TEMP and MAX_TEMP
         temp = MIN_TEMP;
      if (temp > MAX_TEMP)
         temp = MAX_TEMP;

      if (currentTemp.timeVal() > 5000 && fabs(currentTemp.getVal<float>() - temp) > 0.25)
      { // if it's been more than 5 seconds since last update, and temperature has changed
         currentTemp.setVal(temp);
         Serial.printf("Current Temperature is now %s.\n", temp2String(currentTemp.getNewVal<float>()).c_str());
      }

      switch (targetState.getVal())
      {

      case 0:
         if (currentState.getVal() != 0)
         {
            Serial.printf("Thermostat OFF\n");
            currentState.setVal(0);
         }
         break;

      case 1:
         if (currentTemp.getVal<float>() < targetTemp.getVal<float>() && currentState.getVal() != 1)
         {
            Serial.printf("Turning HEAT ON\n");
            currentState.setVal(1);
         }
         else if (currentTemp.getVal<float>() >= targetTemp.getVal<float>() && currentState.getVal() == 1)
         {
            Serial.printf("Turning HEAT OFF\n");
            currentState.setVal(0);
         }
         else if (currentState.getVal() == 2)
         {
            Serial.printf("Turning COOL OFF\n");
            currentState.setVal(0);
         }
         break;

      case 2:
         if (currentTemp.getVal<float>() > targetTemp.getVal<float>() && currentState.getVal() != 2)
         {
            Serial.printf("Turning COOL ON\n");
            currentState.setVal(2);
         }
         else if (currentTemp.getVal<float>() <= targetTemp.getVal<float>() && currentState.getVal() == 2)
         {
            Serial.printf("Turning COOL OFF\n");
            currentState.setVal(0);
         }
         else if (currentState.getVal() == 1)
         {
            Serial.printf("Turning HEAT OFF\n");
            currentState.setVal(0);
         }
         break;

      case 3:
         if (currentTemp.getVal<float>() < heatingThreshold.getVal<float>() && currentState.getVal() != 1)
         {
            Serial.printf("Turning HEAT ON\n");
            currentState.setVal(1);
         }
         else if (currentTemp.getVal<float>() >= heatingThreshold.getVal<float>() && currentState.getVal() == 1)
         {
            Serial.printf("Turning HEAT OFF\n");
            currentState.setVal(0);
         }

         if (currentTemp.getVal<float>() > coolingThreshold.getVal<float>() && currentState.getVal() != 2)
         {
            Serial.printf("Turning COOL ON\n");
            currentState.setVal(2);
         }
         else if (currentTemp.getVal<float>() <= coolingThreshold.getVal<float>() && currentState.getVal() == 2)
         {
            Serial.printf("Turning COOL OFF\n");
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

void sendRequest()
{
    if(request.readyState() == 0 || request.readyState() == 4){
        request.open("GET", "192.168.68.125:3000");
        request.send();
    }
}

void statusCallback(HS_STATUS status)
{
   if (status == HS_PAIRED)
   {
      Serial.println("Starting AsyncTCP client");
      // tcpClient = new AsyncClient();

      // tcpClient->onConnect([](void* arg, AsyncClient* client) {
      //    Serial.println("AsyncTCP connected to host");

      //    client->onData([](void* arg, AsyncClient* client, void* data, size_t len) {
      //       Serial.printf("** data received by client: %" PRIu16 ": len=%u\n", client->localPort(), len);

      //       std::string str = std::string((char*) data, len);
      //       Serial.println("Received: ");
      //       Serial.println(String(str.c_str()));
      //    });

      //    client->write("GET /\r\n\r\n");
      // });

      // IPAddress addr("192.168.68.123");
      // tcpClient->connect("192.168.68.123", 3000);

      sendRequest();
   }
}
////////////////////////////////////////////////////////////////////////

void requestCB(void* optParm, asyncHTTPrequest* request, int readyState)
{
    if(readyState == 4)
    {
        String res = request->responseText();
        Serial.println(res);
        Serial.println();
        request->setDebug(false);

        JsonDocument doc;
        
        deserializeJson(doc, res.c_str());
        serializeJson(doc, Serial);
    }
}

void setupWeb(int ev)
{
   Serial.println("Starting web server");
   webServer.begin();

   webServer.on("/", []() 
   {
      webServer.send(200, "text/html", "Hello, world");
   });
}

void setup()
{
   delay(3000);
   Serial.begin(115200);

   homeSpan.setStatusCallback(statusCallback);
   homeSpan.setStatusPixel(35, 200.0, 100.0, 100.0);
   homeSpan.setPortNum(8080);
   homeSpan.setConnectionCallback(setupWeb);
   homeSpan.enableWebLog(10,"pool.ntp.org","UTC","myLog");           // creates a web log on the URL /HomeSpan-[DEVICE-ID].local:[TCP-PORT]/myLog
   homeSpan.begin(Category::Thermostats, "ReApartment Thermostat");

   new SpanAccessory();
      new Service::AccessoryInformation();
         new Characteristic::Identify();

      new Service::Switch();
         new Characteristic::On();
         new Characteristic::ConfiguredName("Aircon On/Off");

      new Reference_Thermostat();
   
      new Service::Fan();                             // Create the Fan Service
         new Characteristic::Active();                   // This Service requires the "Active" Characterstic to turn the fan on and off
         new Characteristic::CurrentFanState();
         new Characteristic::TargetFanState();
         new Characteristic::RotationSpeed();

      new Service::TemperatureSensor();
         new Characteristic::CurrentTemperature(21.0);
         new Characteristic::ConfiguredName("Inside"); 
      
      new Service::TemperatureSensor();
         new Characteristic::CurrentTemperature(18.0);
         new Characteristic::ConfiguredName("Outside"); 

   request.setDebug(true);
   request.onReadyStateChange(requestCB);
}

////////////////////////////////////////////////////////////////////////

void loop()
{
   homeSpan.poll();
   webServer.handleClient();           // handle incoming web server traffic
}

////////////////////////////////////////////////////////////////////////
