#include "AirConWebServer.h"

AirConWebServer::AirConWebServer(int port)
{
   server = std::make_unique<WebServer>(port);
}

void AirConWebServer::start()
{
   WEBLOG("Starting web server");
   server->begin();

   server->on("/", [&]() 
   {
      prefs.begin(AIRCON_PREFS_CUSTOM_DATA, false);
      String addr = prefs.getString(AIRCON_KEY_ADDR, AIRCON_DEFAULT_URL);
      String port = prefs.getString(AIRCON_KEY_PORT, AIRCON_DEFAULT_PORT);
      prefs.end();

      String content = "<html><body><form action='/setup' method='POST'>Setup connection with Air Conditioner<br><br>";
      content += "URL address of Air Con:<input type='text' name='AIRCONADDR' placeholder='" + addr + "'><br>";
      content += "URL port of Air Con:<input type='text' name='AIRCONPORT' placeholder='" + port + "'><br>";
      content += "<input type='submit' name='SUBMIT' value='Submit'></form><br>";
      server->send(200, "text/html", content);
   });

   server->on("/setup", [&]() 
   {
      if (server->hasArg("AIRCONADDR"))
      {
         prefs.begin(AIRCON_PREFS_CUSTOM_DATA, false);
         prefs.putString(AIRCON_KEY_ADDR, server->arg("AIRCONADDR"));
         prefs.putString(AIRCON_KEY_PORT, server->arg("AIRCONPORT"));
         prefs.end();

         String content = "<html><body><p>URL address and port saved...</p>";
         server->send(200, "text/html", content);

         WEBLOG("Saving the aircon URL address (%s) and port (%s)", 
            server->arg("AIRCONADDR").c_str(), server->arg("AIRCONPORT").c_str());
      }
   });

   server->onNotFound([&]() 
   {
      String message = "File Not Found\n\n";
      message += "URI: ";
      message += server->uri();
      message += "\nMethod: ";
      message += (server->method() == HTTP_GET) ? "GET" : "POST";
      message += "\nArguments: ";
      message += server->args();
      message += "\n";
    
      for (uint8_t i = 0; i < server->args(); i++) 
      {
         message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
      }
    
      server->send(404, "text/plain", message);
    });
}

void AirConWebServer::handleClient()
{
   server->handleClient();
}