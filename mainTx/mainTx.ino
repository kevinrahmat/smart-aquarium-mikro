// including software serial library for rx vlc
#include <SoftwareSerial.h>

// including onewire and dallas temperature library for temperature
#include <OneWire.h>
#include <DallasTemperature.h>


// defining rx and tx pin for vlc
#define RX_PIN 3
#define TX_PIN 4

// defining one wire pin for temperature
#define ONE_WIRE_BUS 5

#define TURBIDITY_PIN A0


// setup software serial pin to communicate with vlc
SoftwareSerial XSERIAL = SoftwareSerial(RX_PIN, TX_PIN);

// setup a oneWire instance to communicate with any OneWire devices
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
  
  Serial.println(temperature);
  Serial.println(turbidity);
}

int startTemperature() {
  sensors.begin();
  return 0;
}

int captureTemperature() {
  sensors.requestTemperatures(); // request temperature from sensors

  int temperature = sensors.getTempCByIndex(0);
  
  XSERIAL.print("Celsius temperature (C): ");
  XSERIAL.println(temperature); // index 0 means first registered ic on the bus 
  delay(1000);
 
  return temperature;
}

int captureTurbidity() {
  int sensorValue = analogRead(TURBIDITY_PIN);
  float voltage = sensorValue * (5.0 / 1024.0); // multiply voltage by bit
  float ntu;

  if(voltage < 2.5){
      ntu = 3000;
    }else{
      ntu = -1120.4*square(voltage)+5742.3*voltage-4353.8; 
    }
  
  XSERIAL.print ("Turbidity Output (V): ");
  XSERIAL.println (ntu);
  delay(1000);

  return voltage;
}
