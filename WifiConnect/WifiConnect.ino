#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "GparksB";
const char* password = "4rustypaintcan";
const char* serverName = "https://bb-vercel-kellynnp.vercel.app/api/hello";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  for (int i = 0; i < 10; i++) {
    int randomNumber = random(0, 100); // generate a random number between 0 and 100
    postData(randomNumber);
    delay(2000); // wait for 1 second before generating the next random number
  }
  delay(2000); // wait for 1 second before exiting the loop
}

void postData(int randomNumber) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    http.begin(serverName);
    
    // Trying to run json instead
    http.addHeader("Content-Type", "application/json");
    DynamicJsonDocument jsonDoc(1024);
    jsonDoc["value"] = randomNumber;
    String postData;
    serializeJson(jsonDoc, postData);
    Serial.print(postData);
    Serial.println();
    int httpResponseCode = http.POST(postData);
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String response = http.getString();
      Serial.println(response);
    } else {
      Serial.print("Error in posting data. HTTP Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}
