#include <Arduino.h>
#include <ArduinoBLE.h>
#define LEDPIN 13
#define SENSORPIN 4 // beam sensor

// variables will change:
int  sensor_state = 0;
int  last_state   = 0;
bool scored       = false;
const char * score_uuid   = "0e86742d-9448-4816-9663-b4b7b97d8146";
const char * service_uuid = "489b94d1-b4f9-437a-953d-afc67a9eb363";

// ble characteristic & service
BLEByteCharacteristic score_char(score_uuid, BLERead | BLENotify | BLEWrite);
BLEService ble_service(service_uuid);

void SendScore() {
  score_char.writeValue(1);
}

void ResetScore() {
  score_char.writeValue(0);
}

void connected(BLEDevice central) {
  Serial.println(F("connected"));
}

void disconnected(BLEDevice central) {
  Serial.println(F("disconnected"));
}

void setup() {
  // delay start in case of error (to reflash)
  delay(3000);

  // start serial
  Serial.begin(9600);

  // setup pins
  pinMode(LEDPIN, OUTPUT);      
  pinMode(SENSORPIN, INPUT);     
  digitalWrite(SENSORPIN, HIGH); // turn on the pullup
  
  // start ble
  if (!BLE.begin()) {
    Serial.println(F("starting BLE failed!"));
    while (1);
  }

  // set ble params
  BLE.setLocalName("pop-a-shot");
  ble_service.addCharacteristic(score_char);
  BLE.addService(ble_service);
  BLE.setEventHandler(BLEConnected, connected);
  BLE.setEventHandler(BLEDisconnected, disconnected);
  score_char.setValue(0);
  BLE.setAdvertisedService(ble_service);
  BLE.advertise();

  Serial.println(F("setup done"));
}

void loop(){
  // read the state of the pushbutton value:
  BLE.poll();
  sensor_state = digitalRead(SENSORPIN);

  // set leds for visual feedback
  // beam is broken = sensor_state == LOW
  if (sensor_state == LOW) {     
    // turn LED on:
    digitalWrite(LEDPIN, HIGH);  
  } 
  else {
    // turn LED off:
    digitalWrite(LEDPIN, LOW); 
  }
  
  // we don't need to update each loop, just when the state changes
  if (sensor_state && !last_state) {
    Serial.println(F("Unbroken"));
    ResetScore();
  } 
  if (!sensor_state && last_state) {
    Serial.println(F("Broken"));
    SendScore();
  }

  last_state = sensor_state;
}