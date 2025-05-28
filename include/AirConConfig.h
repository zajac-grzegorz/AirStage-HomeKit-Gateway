#pragma once

#define AIRCON_WEBSERVER_PORT 80
#define AIRCON_DEFAULT_URL "192.168.1.22"
#define AIRCON_DEFAULT_PORT "8080"

#define AIRCON_JSON_DEVICE_ID "CC4740088A63"
#define AIRCON_JSON_LEVEL_SET "02"
#define AIRCON_JSON_LEVEL_GET "03"

#define AIRCON_HOMEKIT_BRIDGE_NAME "ReApartment Bridge"
#define AIRCON_HOMEKIT_PAIRING_CODE "19801980"

#define AIRCON_MIN_TEMP 18  // minimum allowed temperature in celsius
#define AIRCON_MAX_TEMP 25  // maximum allowed temperature in celsius

#define AIRCON_UPDATE_TIME_ELAPSED 5000 // time elapsed (in millis) since value was last updated
#define AIRCON_UPDATE_DELTA 0.25

#define AIRCON_TEMP_MODIFIER 5000
#define AIRCON_TEMP_DIVIDER 100.0