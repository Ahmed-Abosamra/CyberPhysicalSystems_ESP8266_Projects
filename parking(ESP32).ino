#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>  // Use ESP32-compatible servo

// WiFi AP credentials
const char* WIFI_SSID = "SmartParkingAP";
const char* WIFI_PASS = "kali";

// Parking rate settings
const float BASE_RATE = 5.0;
const float HOURLY_RATE = 3.0;

const int SLOT_COUNT = 4;
const long DIST_THRESHOLD_CM = 7;

// Ultrasonic sensor pins (entrance)
const uint8_t TRIG_ENT = 18;
const uint8_t ECHO_ENT = 19;

// Ultrasonic sensor pins (slots)
const uint8_t trigPins[SLOT_COUNT] = { 5, 35, 12, 4 };
const uint8_t echoPins[SLOT_COUNT] = { 2, 15, 13, 14 };

// LED pins for each slot (green and red)
const uint8_t ledGreenPins[SLOT_COUNT] = { 21, 22, 23, 25 };
const uint8_t ledRedPins[SLOT_COUNT]   = { 26, 27, 32, 33 };

// Servo pin
const uint8_t SERVO_PIN = 12;

WebServer server(80);
Servo gate;

bool occupied[SLOT_COUNT] = { false, false, false, false };
unsigned long timeIn[SLOT_COUNT] = { 0, 0, 0, 0 };
float lastBill[SLOT_COUNT] = { 0, 0, 0, 0 };

long measureCM(uint8_t trigPin, uint8_t echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return 9999;
  return (duration * 0.0343) / 2;
}

float computeBill(unsigned long start_ms, unsigned long end_ms) {
  float hours = (end_ms - start_ms) / 3600000.0;
  return BASE_RATE + HOURLY_RATE * hours;
}

String makeDashboard() {
  String html = "<!doctype html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>Smart Parking</title>";
  html += "<meta http-equiv='refresh' content='3'>";
  html += "<style>body{font-family:Arial;padding:8px;}table{border-collapse:collapse;width:100%;}td,th{border:1px solid #ccc;padding:6px;text-align:left;}</style>";
  html += "</head><body>";
  html += "<h3>Smart Parking System</h3>";
  html += "<table><tr><th>Slot</th><th>Status</th><th>Time In</th><th>Duration</th><th>Bill (EGP)</th></tr>";

  int avail = 0;
  unsigned long now = millis();
  for (int i = 0; i < SLOT_COUNT; i++) {
    String status = occupied[i] ? "<span style='color:red'>Occupied</span>" : "<span style='color:green'>Available</span>";
    String timeInStr = "-";
    String durStr = "-";
    String billStr = "0.00";
    if (occupied[i]) {
      unsigned long durMs = now - timeIn[i];
      unsigned long s = durMs / 1000;
      unsigned long hh = s / 3600;
      unsigned long mm = (s % 3600) / 60;
      unsigned long ss = s % 60;
      char buf[32];
      sprintf(buf, "%02lu:%02lu:%02lu", hh, mm, ss);
      durStr = String(buf);
      timeInStr = String(timeIn[i] / 1000) + "s";
      float bill = computeBill(timeIn[i], now);
      char bbuf[16];
      dtostrf(bill, 0, 2, bbuf);
      billStr = String(bbuf);
    } else {
      avail++;
    }
    html += "<tr><td>" + String(i + 1) + "</td><td>" + status + "</td><td>" + timeInStr + "</td><td>" + durStr + "</td><td>" + billStr + "</td></tr>";
  }

  html += "</table>";
  html += "<p><b>Total Slots:</b> " + String(SLOT_COUNT) +
          " &nbsp; <b>Available:</b> " + String(avail) +
          " &nbsp; <b>Occupied:</b> " + String(SLOT_COUNT - avail) + "</p>";
  html += "<p>Rate: Base " + String(BASE_RATE) + " + " + String(HOURLY_RATE) + "/hour</p>";
  html += "</body></html>";
  return html;
}

void handleRoot() {
  server.send(200, "text/html", makeDashboard());
}

void setupPins() {
  pinMode(TRIG_ENT, OUTPUT);
  pinMode(ECHO_ENT, INPUT);
  for (int i = 0; i < SLOT_COUNT; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
    pinMode(ledGreenPins[i], OUTPUT);
    pinMode(ledRedPins[i], OUTPUT);
    digitalWrite(ledGreenPins[i], HIGH);
    digitalWrite(ledRedPins[i], LOW);
  }
}

void setup() {
  Serial.begin(9600);
  setupPins();

  // Servo setup
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  gate.setPeriodHertz(50);
  gate.attach(SERVO_PIN, 500, 2400);
  gate.write(0);

  // **Set ESP32 to AP mode only**
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASS);
  Serial.print("AP Mode IP: ");
  Serial.println(WiFi.softAPIP());  // default is 192.168.4.1 normally :contentReference[oaicite:0]{index=0}

  // Start web server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started (AP mode)");
}

unsigned long lastMeasure = 0;
const unsigned long SERVO_OPEN_MS = 7000;

void loop() {
  server.handleClient();

  unsigned long now = millis();
  if (now - lastMeasure < 300) return;
  lastMeasure = now;

  long dent = measureCM(TRIG_ENT, ECHO_ENT);
  bool entrance = (dent < DIST_THRESHOLD_CM);

  bool detected[SLOT_COUNT];
  for (int i = 0; i < SLOT_COUNT; i++) {
    long d = measureCM(trigPins[i], echoPins[i]);
    detected[i] = (d < DIST_THRESHOLD_CM);
  }

  for (int i = 0; i < SLOT_COUNT; i++) {
    if (!occupied[i] && detected[i]) {
      occupied[i] = true;
      timeIn[i] = now;
      digitalWrite(ledGreenPins[i], LOW);
      digitalWrite(ledRedPins[i], HIGH);
      Serial.printf("Slot %d occupied\n", i + 1);
      
    } else if (occupied[i] && !detected[i]) {
      unsigned long leftAt = now;
      lastBill[i] = computeBill(timeIn[i], leftAt);
      occupied[i] = false;
      timeIn[i] = 0;
      digitalWrite(ledGreenPins[i], HIGH);
      digitalWrite(ledRedPins[i], LOW);
      Serial.printf("Slot %d freed. Bill: %.2f\n", i + 1, lastBill[i]);
    }
  }

  int avail = 0;
  for (int i = 0; i < SLOT_COUNT; i++) {
    if (!occupied[i]) avail++;
  }

  if (entrance && avail > 0) {
    Serial.println("Entrance detected & space available -> opening gate");
    gate.write(90);
    unsigned long openedAt = millis();
    while (millis() - openedAt < SERVO_OPEN_MS) {
      delay(100);
    }
    gate.write(0);
  }
}
