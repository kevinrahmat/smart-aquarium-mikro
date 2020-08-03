#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define RX_PIN D2
#define TX_PIN D3

#define SSID "KemeK"
#define PASSWORD "bambangganteng"

#define FIREBASE_HOST "smart-aquarium-telkom.firebaseio.com"
#define FIREBASE_AUTH "d3Z3iIG3dTRJPStlScgTEoYSSPSpjAADIXGkWB3q"

SoftwareSerial XSERIAL = SoftwareSerial(RX_PIN, TX_PIN, false);

String parsedData = "";
bool isDataTransferCompleted = false;

FirebaseData firebaseData;

void connectWifi()
{
  Serial.print("Configuring access point...");
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");

  
  parsedData.reserve(200);
  delay(500);
}

void registerFirebase()
{
  Serial.println("Initializing Firebase");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}

void captureData () {
  if(XSERIAL.available()){
    char singleData = (char)XSERIAL.read();
        
    if (singleData == '\n') {
      isDataTransferCompleted = true;
    } else {
      parsedData += singleData;
    }
  }
}

void setup()
{
  delay(1000);
  
  XSERIAL.begin(9600);
  Serial.begin(9600);
  
  connectWifi();
  registerFirebase();
  
  Serial.println("Listening to ...");
}

void loop()
{  
  captureData();
  if (isDataTransferCompleted) {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& object = jsonBuffer.parseObject(parsedData);

    Serial.println(parsedData);

    if (object.success()) {
      
      const char* temperature = object["temperature"];
      const char* turbidity = object["turbidity"];
      
      Serial.print(temperature);
      Serial.print("|");
      Serial.println(turbidity);
      
      Firebase.set(firebaseData, "temperature/value", temperature);
      Firebase.set(firebaseData, "turbidity/value", turbidity);
      
    }
    
    isDataTransferCompleted = false;
    parsedData = "";
  }
}
