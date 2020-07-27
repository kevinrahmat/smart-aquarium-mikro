#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <FirebaseArduino.h>
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
  //Firebase.stream("led_status");

  delay(10);

  if (Firebase.failed())
  {
    Serial.print("Connection failed");
    Serial.println(Firebase.error());
    return;
  }
  else
  {
    Serial.println("Connection success");
  }

  Firebase.setInt("led_status", 1);
  if (Firebase.failed())
  {
    Serial.print("Set failed");
    Serial.println(Firebase.error());
    return;
  }
  else
  {
    Serial.println("Set success");
  }
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
    Serial.println(parsedData);

    Firebase.setString("sensor", parsedData);
    if (Firebase.failed()) {
     Serial.println("Set Failed");
    }
    
    isDataTransferCompleted = false;
    parsedData = "";
  }
}
