#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define RX_PIN D2
#define TX_PIN D3

#define PUMP_PIN D3 
#define HEATER_PIN D4

#define SSID "KemeK"
#define PASSWORD "bambangganteng"

#define FIREBASE_HOST "smart-aquarium-telkom.firebaseio.com"
#define FIREBASE_AUTH "d3Z3iIG3dTRJPStlScgTEoYSSPSpjAADIXGkWB3q"

SoftwareSerial XSERIAL = SoftwareSerial(RX_PIN, TX_PIN, false);

String parsedData = "";
bool isDataTransferCompleted = false;

bool autoModeStatus = true;
bool pumpModeStatus = false;
bool heaterModeStatus = false;

FirebaseData firebaseData;
FirebaseData autoMode;
FirebaseData pumpStatus;
FirebaseData heaterStatus;

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

  parsedData.reserve(0200);
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
    int comparison = XSERIAL.read();

    if (singleData == '\n') {
      isDataTransferCompleted = true;
    } else {
       parsedData += singleData;
    }

  }
}

void beginStreaming()
{
  if (!Firebase.beginStream(autoMode, "/status/auto_mode"))
  {
    Serial.println(firebaseData.errorReason());
  }
  if (!Firebase.beginStream(pumpStatus, "/status/pump"))
  {
    Serial.println(firebaseData.errorReason());
  }
  if (!Firebase.beginStream(heaterStatus, "/status/heater"))
  {
    Serial.println(firebaseData.errorReason());
  }
}

void readAutoModeData()
{
  if (!Firebase.readStream(autoMode))
  {
    Serial.println(autoMode.errorReason());
  }
  
  if (firebaseData.streamTimeout())
  {
    Serial.println("Auto Mode timeout, resume streaming...");
    Serial.println();
  }
  
  if (autoMode.streamAvailable())
  {
   if (autoMode.dataType() == "boolean") {
      Serial.println(autoMode.boolData() == 1 ? "true" : "false");
      if (autoMode.boolData() == 1)  {
        autoModeStatus = true;
      } else {
        autoModeStatus = false;
      }
   }
  }
}

void readHeaterStatus()
{
  if (!Firebase.readStream(heaterStatus))
  {
    Serial.println(heaterStatus.errorReason());
  }
  
  if (heaterStatus.streamTimeout())
  {
    Serial.println("Heater timeout, resume streaming...");
    Serial.println();
  }
  
  if (heaterStatus.streamAvailable())
  {
    if (heaterStatus.dataType() == "boolean") {
      Serial.println(heaterStatus.boolData() == 1 ? "true" : "false");
      if (heaterStatus.boolData() == 1)  {
        heaterModeStatus = true;
      } else {
        heaterModeStatus = false;
      }
    }
  }
}

void readPumpStatus()
{
  if (!Firebase.readStream(pumpStatus))
  {
    Serial.println(pumpStatus.errorReason());
  }
  
  if (pumpStatus.streamTimeout())
  {
    Serial.println("Pump timeout, resume streaming...");
    Serial.println();
  }
  
  if (pumpStatus.streamAvailable())
  {
    if (pumpStatus.dataType() == "boolean") {
      Serial.println(pumpStatus.boolData() == 1 ? "true" : "false");
      if (pumpStatus.boolData() == 1)  {
        pumpModeStatus = true;
      } else {
        pumpModeStatus = false;
      }
    }
  }
}


void setup()
{
  delay(1000);

  pinMode(PUMP_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  
  XSERIAL.begin(9600);
  Serial.begin(74880);
  
  connectWifi();
  registerFirebase();
  beginStreaming();
  
  Serial.println("Listening to ...");
}

void loop()
{  
  captureData();
  if (isDataTransferCompleted) {
    readAutoModeData();
    readHeaterStatus();
    readPumpStatus();

    char c;

    for (int i=0; i<parsedData.length()-1;++i){
        c = parsedData.charAt(i);
        if(!isAscii(c)){
            parsedData.remove(i, 1);
        }
    }
    
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& object = jsonBuffer.parseObject(parsedData);

    if (object.success()) {
      
      const int temperature = object["t"];
      const int turbidity = object["u"];
      
      Serial.print(temperature);
      Serial.print("|");
      Serial.println(turbidity);

      if (autoModeStatus) {
        if (temperature < 22) {
          digitalWrite(HEATER_PIN,HIGH);
        } else {
          digitalWrite(HEATER_PIN, LOW);
        }
  
        if (turbidity < 2) {
           digitalWrite(PUMP_PIN,HIGH);
        } else {
          digitalWrite(PUMP_PIN, LOW);
        }
      } else {
        if (heaterModeStatus) {
           digitalWrite(HEATER_PIN,HIGH);
         } else {
           digitalWrite(HEATER_PIN, LOW);
         }
         if (pumpModeStatus) {
           digitalWrite(PUMP_PIN,HIGH);
         } else {
           digitalWrite(PUMP_PIN, LOW);
         }
      }

      
      Firebase.set(firebaseData, "temperature/value", temperature);
      Firebase.set(firebaseData, "turbidity/value", turbidity);
      
    }
    
    isDataTransferCompleted = false;
    parsedData = "";
  }
}
