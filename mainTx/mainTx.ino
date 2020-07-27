#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include <OneWire.h>
#include <DallasTemperature.h>


#define RX_PIN 3
#define TX_PIN 4

#define ONE_WIRE_BUS 5
#define TURBIDITY_PIN A0

SoftwareSerial XSERIAL = SoftwareSerial(RX_PIN, TX_PIN);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  XSERIAL.begin(9600);
  Serial.begin(9600);
  startTemperature();
}

void loop() {
  int temperature = captureTemperature();
  float turbidity = captureTurbidity();
  char jsonChar[100];

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data.set("temperature", temperature);
  data.set("turbidity", turbidity);
  data.printTo(jsonChar);

  XSERIAL.println(jsonChar);

  Serial.println(jsonChar);

  delay(5000);
}

int startTemperature() {
  sensors.begin();
  return 0;
}

int captureTemperature() {
  sensors.requestTemperatures();
  int temperature = sensors.getTempCByIndex(0);
  
  return temperature;
}

int captureTurbidity() {
  int sensorValue = analogRead(TURBIDITY_PIN);
  float voltage = sensorValue * (5.0 / 1024.0);
  float ntu;

  if (voltage < 2.5){
    ntu = 3000;
  } else {
    ntu = -1120.4*square(voltage)+5742.3*voltage-4353.8; 
  }

  return voltage;
}
