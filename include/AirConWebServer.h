#include "HomeSpan.h"
#include <Preferences.h>
#include <WebServer.h>
#include "AirConConfig.h"

class AirConWebServer
{
   public:
      explicit AirConWebServer(int port);
      void start();
      void handleClient();

   private:
      std::unique_ptr<WebServer> server;
      Preferences prefs;
};