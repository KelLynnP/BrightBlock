const int batteryPin = 34;  

void setup() {
  // put your setup code here, to run once:
  pinMode(batteryPin, INPUT);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Battery voltage: " + String(analogRead(batteryPin)));
  delay(1000);
}
