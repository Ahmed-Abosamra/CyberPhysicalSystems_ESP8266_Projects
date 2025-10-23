//Ahmed Mohamed Abosamra- 2025
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ======= YOUR WIFI CREDENTIALS =======
const char* apSSID="ESP8266_LED_AP";
const char* apPASS="AASTTEST";

// ======= LED PIN =======
#define RED D1
#define GREEN D2
bool redon = false;
bool greenon = false;

// Web server on port 80
ESP8266WebServer server(80);

// ======= Build the HTML page =======
String buildPage() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP8266 LED Control</title>";
  html += "<style>";
  html += "body {text-align:center; font-family:Arial; margin:30px; background-color:#f7f7f7;}";
  html += "h2 {color:#333; margin-bottom:20px;}";
  html += "h3 {color:#444; margin-top:40px; margin-bottom:10px;}";
  html += ".button-row {display:flex; justify-content:center; gap:10px; margin-bottom:20px;}";
  html += "button {padding:12px 28px; cursor:pointer; border:none; border-radius:8px; background-color:#0078d7; color:white; font-size:14px; transition:0.3s;}";
  html += "button:hover {background-color:#005fa3;}";
  html += ".led-section {background:white; border-radius:12px; box-shadow:0 0 10px rgba(0,0,0,0.1); padding:20px; margin:20px auto; width:300px;}";
  html += "p.info {margin-top:30px; font-size:13px; color:#555;}";
  html += "</style></head>";

  html += "<body>";
  html += "<h2>ESP8266 LED Control (AP Mode)</h2>";

  // --- Red LED Section ---
  html += "<div class='led-section'>";
  html += "<h3>Red LED</h3>";
  html += "<p>Status: <b>";
  html += (redon ? "ON" : "OFF");
  html += "</b></p>";
  html += "<div class='button-row'>";
  html += "<a href='/red_on'><button>Turn ON</button></a>";
  html += "<a href='/red_off'><button>Turn OFF</button></a>";
  html += "</div></div>";

  // --- Green LED Section ---
  html += "<div class='led-section'>";
  html += "<h3>Green LED</h3>";
  html += "<p>Status: <b>";
  html += (greenon ? "ON" : "OFF");
  html += "</b></p>";
  html += "<div class='button-row'>";
  html += "<a href='/green_on'><button>Turn ON</button></a>";
  html += "<a href='/green_off'><button>Turn OFF</button></a>";
  html += "</div></div>";

  // --- WiFi Info ---
  html += "<p class='info'>Connect to WiFi: <b>";
  html += apSSID;
  html += "</b> (password: ";
  html += apPASS;
  html += ")</p>";

  html += "</body></html>";
  return html;
}

// ======= Route Handlers =======
void handleRoot(){server.send(200,"text/html",buildPage());}

void handleredon(){redon=true;digitalWrite(RED,HIGH);server.send(200,"text/html",buildPage());}
void handleredoff(){redon=false;digitalWrite(RED,LOW);server.send(200,"text/html",buildPage());}

void handlegreenon(){greenon=true;digitalWrite(GREEN,HIGH);server.send(200,"text/html",buildPage());}
void handlegreenoff(){greenon=false;digitalWrite(GREEN,LOW);server.send(200,"text/html",buildPage());}

void setup() {
Serial.begin(9600);
pinMode(RED,OUTPUT);
pinMode(GREEN,OUTPUT);
digitalWrite(RED,LOW);
digitalWrite(GREEN,LOW);

  // Connect to WiFi
Serial.println("Starting Access Point...");
WiFi.mode(WIFI_AP);
WiFi.softAP(apSSID,apPASS);

    // WiFi Info
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP started. Connect to WiFi '");
  Serial.print(apSSID);
  Serial.print("' with password '");
  Serial.print(apPASS);
  Serial.print("'. Then open: http://");
  Serial.println(myIP);

    // Define routes
server.on("/",handleRoot);
server.on("/red_on",handleredon);
server.on("/red_off",handleredoff);
server.on("/green_on",handlegreenon);
server.on("/green_off",handlegreenoff);
Serial.println("Web server started.");

  // Start web server
server.begin();

}

void loop() {
    server.handleClient();
}
