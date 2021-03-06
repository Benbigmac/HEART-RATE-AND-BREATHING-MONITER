
#include <Wire.h>
#include <SPI.h>

#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
#define Serial SerialUSB
#endif
#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
//#include "pitches.h"
#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
#include <SoftwareSerial.h>Ω
#endif

int heartRateReading=A0;
int Respiration=A1;
int buzzerPin = 9;
int lightUp = 8;
int heartRate = 0;
#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);



//Variables for LED respiration rate
int ledPin = 13;
int upperThreshold = 495; 
int lowerThreshold = 490;
int meditationThreshold = 18; //rate per min that is considered for meditation
int reading = 0;
float rate = 0.0;
bool ignoreReading = false;
bool firstPulseDetected = false;
unsigned long firstPulseTime = 0;
unsigned long secondPulseTime = 0;
unsigned long pulseInterval = 0;

void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

void setup() {
  // initialize the serial communication:
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(10, INPUT); // Setup for leads off detection LO +
  pinMode(11, INPUT); // Setup for leads off detection LO -
  pinMode(buzzerPin, INPUT); //Gives you a buzz when you a little too paniced
  pinMode(lightUp, INPUT);
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }
  /* Disable command echo from Bluefruit */
  ble.echo(false);
  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  // Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
  // Serial.println(F("Then Enter characters to send to Bluefruit"));
  // Serial.println();
  ble.verbose(false);  // debug info is a little annoying after this point!
  /* Wait for connection */
  while (! ble.isConnected()) {
    delay(500);
  }
  Serial.println(F("******************************"));
  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }
  // Set module to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);
  Serial.println(F("******************************"));
}

void loop() {
  reading = analogRead(A1);
  if(reading > upperThreshold && ignoreReading == false){
    if(firstPulseDetected == false){
      firstPulseTime = millis();
      firstPulseDetected = true;
    }
    else{
      secondPulseTime = millis();
      pulseInterval = secondPulseTime - firstPulseTime;
      firstPulseTime = secondPulseTime;
    }
    ignoreReading = true;
  }

  // Heart beat trailing edge detected.
  if(reading < lowerThreshold){
    ignoreReading = false;
  }

  rate = (1.0/pulseInterval) * 60.0 * 1000;
  Serial.print("Rate is ");
  Serial.println(rate);
  if(rate > meditationThreshold) {
    digitalWrite(ledPin, HIGH);
  }
  else {
    digitalWrite(ledPin, LOW);
  }
  


  if((digitalRead(10) == 1)||(digitalRead(11) == 1)){
    Serial.println('!');
  }
  else{
    // send the value of analog input 0:
    heartRate=analogRead(A2);
    
    //Respiration=analogRead(A1);
    //Serial.println(heartRate);
    ble.print(reading);
    ble.print(",");
    ble.print(heartRate);
    ble.print("\n");

    if(heartRate>180){ //need to change or calculate the heart rate
      digitalWrite(lightUp, HIGH);

    }else{
      digitalWrite(lightUp, LOW);
    }

  }
  //Wait for a bit to keep serial data from saturating
  delay(1);
}
