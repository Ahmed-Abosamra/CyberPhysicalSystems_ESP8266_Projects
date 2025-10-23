#define TRIG_PIN D1
#define ECHO_PIN D2

int leds[] = {D3, D4, D5, D6, D7}; // LED pins
int ledCount = 5;

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);  // Set Trig pin as OUTPUT
  pinMode(ECHO_PIN, INPUT);   // Set Echo pin as INPUT

  for (int i = 0; i < ledCount; i++) {
    pinMode(leds[i], OUTPUT); 
    digitalWrite(leds[i], HIGH); // Turn on all LEDs initially
  }
}

void loop() {
  long duration;
  float distance;

  // Send ultrasonic pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read the returning pulse duration
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate distance in centimeters
  distance = duration * 0.034 / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // --- Control the number of LEDs ---
  // Distance > 50 cm → all LEDs ON
  // Distance < 10 cm → all LEDs OFF
  int numOn = map(distance, 10, 50, 0, ledCount);
  numOn = constrain(numOn, 0, ledCount);

  for (int i = 0; i < ledCount; i++) {
    if (i < numOn)
      digitalWrite(leds[i], HIGH); // LED ON
    else
      digitalWrite(leds[i], LOW);  // LED OFF
  }

  delay(200); // Wait 200 ms before next measurement
}
