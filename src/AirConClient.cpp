#include "AirConConfig.h"
#include "AirConClient.h"
#include "HomeSpan.h"

AirConClient::AirConClient()
{
   content = new StreamString();
   content->reserve(AC_CLIENT_MAX_RECEIVE_BUF);

   prefs.begin("CUST_DATA", true);
   host = prefs.getString("acaddr", AIRCON_DEFAULT_URL);
   port = prefs.getString("acport", AIRCON_DEFAULT_PORT).toInt();
   prefs.end();

   LOG1("AirCon url: %s:%d\n", host.c_str(), port);
}

AirConClient::~AirConClient()
{
   if (content)
   {
      delete content;
   }
}

void AirConClient::start()
{
   client = new AsyncClient();

   // register event handlers
   client->onConnect(std::bind(&AirConClient::connectHandler, this, 
      std::placeholders::_1, std::placeholders::_2), NULL);
   client->onDisconnect(std::bind(&AirConClient::disconnectHandler, this, 
      std::placeholders::_1, std::placeholders::_2), NULL);
   client->onError(std::bind(&AirConClient::errorHandler, this, 
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), NULL);
   client->onData(std::bind(&AirConClient::dataHandler, this, 
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), NULL);
   
   client->setRxTimeout(20000);
   // client->setAckTimeout(10000);
   client->setNoDelay(true);

   if (!client->connect(host.c_str(), port)) 
   {
      LOG1("[%s] Failed to connect!\n", host.c_str());
   }
}

void AirConClient::getParams()
{
   if (!client)
   {
      LOG1("CLIENT not initialized\n");
      return;
   }

   JsonDocument doc;

   doc["device_id"] = AIRCON_DEVICE_ID;
   doc["device_sub_id"] = 0;
   doc["req_id"] = "1";
   doc["modified_by"] = "";
   doc["set_level"] = "03";
   doc["list"].add(iu_onoff);
   doc["list"].add(iu_op_mode);
   doc["list"].add(iu_fan_spd);
   doc["list"].add(iu_set_tmp);
   doc["list"].add(iu_indoor_tmp);
   doc["list"].add(iu_outdoor_tmp);

   String req;
   serializeJson(doc, req);
   LOG1("JSON DATA = %s\n\n", req.c_str());

   String initLine;
   initLine.reserve(50);

   initLine = "POST /";
   initLine += STR_GET_PARAM;
   initLine += " HTTP/1.1\r\n";
   
   String hostHeaderLine;
   hostHeaderLine.reserve(50);

   hostHeaderLine = "Host: ";
   hostHeaderLine += host + ":" + port + "\r\n";

   String contentLengthHeaderLine;
   contentLengthHeaderLine.reserve(50);

   contentLengthHeaderLine = "Content-Length: ";
   contentLengthHeaderLine += req.length();
   contentLengthHeaderLine += "\r\n";
   LOG1(contentLengthHeaderLine.c_str());

   client->write(initLine.c_str());
   client->write(hostHeaderLine.c_str());
   client->write("Content-Type: application/json\r\n");
   client->write(contentLengthHeaderLine.c_str());
   client->write("\r\n");
   
   client->write(req.c_str());
}

void AirConClient::setParams()
{
}

void AirConClient::setAirConMode(bool on, int mode)
{
   LOG1("On/Off: %d, mode %d\n", on, mode);
}

void AirConClient::connectHandler(void *arg, AsyncClient *client)
{
   LOG1("Connected to AirCon host\n");
   getParams();
}

void AirConClient::disconnectHandler(void *arg, AsyncClient *client)
{
   LOG1("Disconnected from AirCon host\n");
   delete client;

   LOG1("Final data:\n");

   String res = content->substring(content->lastIndexOf("\r\n") + 2).c_str();
   content->clear();

   LOG1("%s\n\n", res.c_str());

   JsonDocument doc;

   deserializeJson(doc, res.c_str());
   serializeJsonPretty(doc, Serial);
   LOG1("\n\n");

   String inTmpStr = doc["value"]["iu_indoor_tmp"];
   String outTmpStr = doc["value"]["iu_outdoor_tmp"];
   String setTmpStr = doc["value"]["iu_set_tmp"];
   String onOffStr = doc["value"]["iu_onoff"];
   String opModeStr = doc["value"]["iu_op_mode"];
   String fanSpdStr = doc["value"]["iu_fan_spd"];

   acInsideTemp.store(inTmpStr.toInt());
   acOutsideTemp.store(outTmpStr.toInt());
   acSetTemp.store(setTmpStr.toInt());
   acFanSpeed.store(fanSpdStr.toInt());
   setAirConMode(onOffStr.toInt(), opModeStr.toInt());

   LOG1("Temp in: %d, temp out %d, temp set %d\n", acInsideTemp.load(), acOutsideTemp.load(), acSetTemp.load());
}

void AirConClient::errorHandler(void *arg, AsyncClient *client, int8_t error)
{
   LOG1("Error AirCon host [%d]\n", error);
}

void AirConClient::dataHandler(void *arg, AsyncClient *client, void *data, size_t len)
{
   LOG1("Data received from AirCon host\n");
   LOG1("[%s] Received %u bytes...\n", host.c_str(), len);

   content->write((const uint8_t*) data, len);
}
