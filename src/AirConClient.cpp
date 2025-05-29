#include "AirConConfig.h"
#include "AirConClient.h"
#include "HomeSpan.h"

AirConClient::AirConClient()
{
   content = new StreamString();
   content->reserve(AC_CLIENT_MAX_RECEIVE_BUF);

   prefs.begin(AIRCON_PREFS_CUSTOM_DATA, true);
   host = prefs.getString(AIRCON_KEY_ADDR, AIRCON_DEFAULT_URL);
   port = prefs.getString(AIRCON_KEY_PORT, AIRCON_DEFAULT_PORT).toInt();
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
   if (nullptr != client)
   {
      LOG1("WARNING: One request already sent\n");
      return;
   }

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
      LOG1("ERROR: Failed to connect [%s]\n", host.c_str());
   }
}

void AirConClient::getParams()
{
   if (!client)
   {
      LOG1("ERROR: Client not initialized\n");
      return;
   }

   JsonDocument doc;

   doc["device_id"] = AIRCON_JSON_DEVICE_ID;
   doc["device_sub_id"] = 0;
   doc["req_id"] = "1";
   doc["modified_by"] = "";
   doc["set_level"] = AIRCON_JSON_LEVEL_GET;
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
   initLine += AIRCON_GET_PARAM;
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

void AirConClient::connectHandler(void *arg, AsyncClient *cl)
{
   LOG1("Connected to AirCon host\n");
   getParams();
}

void AirConClient::disconnectHandler(void *arg, AsyncClient *cl)
{
   LOG1("Disconnected from AirCon host\n");
   delete client;
   client = nullptr;

   String statusLine = content->substring(0, content->indexOf("\r\n"));
   String statusCode = statusLine.substring(statusLine.indexOf(" ") + 1, statusLine.lastIndexOf(" "));
   String statusReason = statusLine.substring(statusLine.lastIndexOf(" ") + 1);
   LOG1("Header status: %s, %s\n\n", statusCode.c_str(), statusReason.c_str());

   LOG1("Final data:\n");
   String res = content->substring(content->lastIndexOf("\r\n") + 2);
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

void AirConClient::errorHandler(void *arg, AsyncClient *cl, int8_t error)
{
   LOG1("ERROR: AirCon host [%d]\n", error);
}

void AirConClient::dataHandler(void *arg, AsyncClient *cl, void *data, size_t len)
{
   LOG1("Received %u bytes from AirCon host [%s]\n", len, host.c_str());

   content->write((const uint8_t*) data, len);
}
