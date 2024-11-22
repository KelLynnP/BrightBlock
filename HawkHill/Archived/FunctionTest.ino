// Basic ESP32 diagnostic test

void setup() {
  Serial.begin(115200);
  delay(1000);  // Give serial time to start
  
  Serial.println("\n=== ESP32 Diagnostic Test ===");
  
  // Test basic GPIO outputs - your indicator LEDs
  Serial.println("\nTesting Indicator LEDs...");
  pinMode(10, OUTPUT);  // Green
  pinMode(11, OUTPUT);  // Red
  pinMode(12, OUTPUT);  // Blue
  
  // Flash each LED in sequence
  Serial.println("Green LED Test");
  // digitalWrite(10, HIGH);
  // delay(1000);
  // digitalWrite(10, LOW);
  
  Serial.println("Red LED Test");
  // digitalWrite(11, HIGH);
  // delay(1000);
  // digitalWrite(11, LOW);
  
  Serial.println("Blue LED Test");
  // digitalWrite(12, HIGH);
  // delay(1000);
  // digitalWrite(12, LOW);
  
  // Test input pins
  Serial.println("\nTesting Input Pins...");
  pinMode(8, INPUT);   // Charge Status
  pinMode(13, INPUT);  // Log Button
  pinMode(33, INPUT);  // Mode Button
  
  // Print chip info
  Serial.printf("\nChip Info:\n");
  Serial.printf("Chip ID: %6X\n", ESP.getChipId());
  Serial.printf("Flash Chip Size: %d bytes\n", ESP.getFlashChipSize());
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  
  Serial.println("\nDiagnostic setup complete. Starting continuous monitoring...");
}

void loop() {
  Serial.println("\n=== Status Update ===");
  
  // Read and report input pins
  Serial.printf("Charge Status Pin (8): %d\n", digitalRead(8));
  Serial.printf("Log Button Pin (13): %d\n", digitalRead(13));
  Serial.printf("Mode Button Pin (33): %d\n", digitalRead(33));
  
  // Report current heap status
  Serial.printf("Current Free Heap: %d bytes\n", ESP.getFreeHeap());
  
  delay(2000);
}