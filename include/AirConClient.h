#pragma once

#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <Preferences.h>
#include <StreamString.h>
#include "AirConData.h"

#define STR_GET_PARAM "GetParam" 
#define STR_SET_PARAM "SetParam"

#define AC_CLIENT_MAX_RECEIVE_BUF 1000

inline const char* iu_onoff = "iu_onoff";
inline const char* iu_op_mode = "iu_op_mode";
inline const char* iu_fan_spd = "iu_fan_spd";
inline const char* iu_set_tmp = "iu_set_tmp";
inline const char* iu_indoor_tmp = "iu_indoor_tmp";
inline const char* iu_outdoor_tmp = "iu_outdoor_tmp";

class AirConClient
{
public:
   AirConClient();
   ~AirConClient();

   void start();
   void getParams();
   void setParams();
   void toJSON();
   void setAirConMode(bool on, int mode);

private:
   
   Preferences prefs;
   AsyncClient* client = nullptr;
   StreamString* content = nullptr;
   String host;
   uint16_t port;

   void connectHandler(void* arg, AsyncClient* client);
   void disconnectHandler(void* arg, AsyncClient* client);
   void errorHandler(void* arg, AsyncClient* client, int8_t error);
   void dataHandler(void* arg, AsyncClient* client, void* data, size_t len);
};
