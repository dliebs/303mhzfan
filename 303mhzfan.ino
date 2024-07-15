//
//
//  303 MHz Fan Controller - Version 1.0.1
//    This version HNB deployed 2024.07.02
//
//  Some changes to words
//
//  WEMOS D1 Mini ESP8266 Based
//    303 MHz Fan Controller
//
//

/*--------       Libraries        --------*/

#include <espHTTPServer.h>
#include <espWiFiUtils.h>

#include <RCSwitch.h>



/*--------    WiFi Credentials    --------*/

#ifndef STASSID
#define STASSID "Your-WiFi-SSID"  // SSID
#define STAPSK  "Your-WiFi-Pass"  // Password
#endif

#define WiFiHostname "fan"


/*-------- User-Defined Variables --------*/

// Webpage User Settings
#define PAGETITLE "Fan"
#define BGCOLOR "000"
#define TABBGCOLOR "111"
#define BUTTONCOLOR "222"
#define TEXTCOLOR "a40"
#define FONT "Helvetica"
#define TABHEIGHTEM "47"
#define REFRESHPAGE false
#define PORT 80

// espHTTPServer Object
espHTTPServer httpServer( PAGETITLE, BGCOLOR, TABBGCOLOR, BUTTONCOLOR, TEXTCOLOR, FONT, TABHEIGHTEM, REFRESHPAGE, PORT );;


/*--------          GPIO          --------*/

// 303 MHz Transmitter connected to D1 Mini D1 (GPIO5), GPIO13/TCK on D1 Mini ESP32
#define TX_PIN303         5
// Built in LED
#define LED               2

/*--------   Program Variables    --------*/

RCSwitch tx303 = RCSwitch();

// Codes for Home Decorations Mercer 54725 Ceiling Fan are 0, followed by DIP switches inverted, then 7 bits XXXXXXX
//                          Off        Low        Med        High       Light
const char* fanCodes[5] = {"1111101", "1110111", "1101111", "1011111", "1111110"};

String fanDIPSwitch = "0000";

/*--------     Main Functions     --------*/

void setup() {
  // Enable 303 MHz Transmitter on Pin 13 and setProtocol to 11
  tx303.enableTransmit(TX_PIN303);
  tx303.setProtocol(11);

  // Connect to WiFi, start OTA
  connectWiFi(STASSID, STAPSK, WiFiHostname, LED);
  initializeOTA(WiFiHostname, STAPSK);

  // Start HTML Server
  serverSetup();
}

void loop() {
  // Webserver
  httpServer.server.handleClient();

  // Arduino OTA
  ArduinoOTA.handle();

  // Let the ESP8266 do its thing
  yield();
}


/*--------    Server Functions    --------*/

// Codes for Home Decorations Mercer 54725 Ceiling Fan are 0, followed by DIP switches inverted, then 7 bits XXXXXXX
void sendFanCode() {
  int code = httpServer.server.arg("code").toInt();
  if ( code >= 0 && code <= 4 ) {
    char* codeToSend = new char[12];
    strcpy(codeToSend, "0");
    strcat(codeToSend, fanDIPSwitch.c_str());
    strcat(codeToSend, fanCodes[code]);
    tx303.send(codeToSend);
    httpServer.redirect();
  }
  else { handleNotFound(); }
}

void setFanDIP() {
  fanDIPSwitch = httpServer.server.arg("DIPSetting");
  httpServer.redirect();
}

/*--------         Webpage        --------*/

void serverSetup() {
  httpServer.server.on("/", handleRoot);
  httpServer.server.on("/sendFanCode", HTTP_GET, sendFanCode);
  httpServer.server.on("/setFanDIP", HTTP_GET, setFanDIP);
  httpServer.server.onNotFound(handleNotFound);
  httpServer.server.begin();
}

String body = "<div class=\"container\">\n"
                "<div class=\"centered-element\">\n"

                  "<form action=\"/sendFanCode\" method=\"GET\">\n"
                    "<button name=\"code\" class=\"inputButton\" onclick=\"this.form.submit()\" value=\"4\" style=\"width: 100%;\">Light</button>\n"
                    "<button name=\"code\" class=\"inputButton\" onclick=\"this.form.submit()\" value=\"3\" style=\"width: 100%;\">High</button>\n"
                    "<button name=\"code\" class=\"inputButton\" onclick=\"this.form.submit()\" value=\"2\" style=\"width: 100%;\">Medium</button>\n"
                    "<button name=\"code\" class=\"inputButton\" onclick=\"this.form.submit()\" value=\"1\" style=\"width: 100%;\">Low</button>\n"
                    "<button name=\"code\" class=\"inputButton\" onclick=\"this.form.submit()\" value=\"0\" style=\"width: 100%;\">Off</button>\n"
                  "</form><br>\n"

                  "<form action=\"/setFanDIP\" style=\"display: flex;\" method=\"GET\">\n"
                    "<input type=\"text\" class=\"textInput\" name=\"DIPSetting\" value=\"%DIPStub%\" style=\"width: 20%;\">\n"
                    "<input type=\"submit\" class=\"textInput\" style=\"width: 80%;\" value=\"Set DIP Switch\">\n"
                  "</form>\n"

                "</div>\n"
              "</div>\n";

void handleRoot() {
  String deliveredHTML = httpServer.assembleHTML(body);
  deliveredHTML.replace("%DIPStub%", fanDIPSwitch);
  httpServer.server.send(200, "text/html", deliveredHTML);
}

// Simple call back to espHTTPServer object for reasons
void handleNotFound() {
  httpServer.handleNotFound();
}
