// to purchase a sensor mote, go to: http://kippkitts.gostorego.com/sensor-mote-aq.html

/* by default, this sketch builds for the XBee; if you want to build for TSRP over ethernet, look for

     #undef TSRP

   and change it to

     #define TSRP

 */

#include <Wire.h>
#include <EEPROM.h>
#include "Adafruit_BMP085.h"
 
Adafruit_BMP085 bmp;
 
// Google I/O SMD Mote

// Authors: Alasdair Allan <alasdair@babilim.co.uk>, Rob Faludi <ron@faludi.com> and Kipp Bradford <kb@kippworks.com>
// Last Updated: 07-May-2013 15:35 EDT
// Last modification: Ready for production, modifications for altitude calculation, and fixes to serial data out 

#define BMP085_ADDRESS 0x77  // I2C address of BMP085
#define CODE_VERSION 2.3
#define SILENT_VALUE 380     // Starting neutral microphone value (self-correcting)
#define BMP085_DEBUG 1

#undef  TSRP

#ifdef  TSRP
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <util.h>
#include <SPI.h>

// The MAC address of your Ethernet board (or Ethernet Shield) is located on the back of the circuit board.
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0E, 0x9C, 0x1B };  // Arduino Ethernet

int requestID = 1;

PROGMEM prog_char *loopPacket1 = "{\"path\":\"/api/v1/thing/reporting\",\"requestID\":\"";
PROGMEM prog_char *loopPacket2 = "\",\"things\":{\"/device/climate/datasensinglab/air-quality\":{\"prototype\":{\"device\":{\"name\":\"Sensor Mote Air Quality\",\"maker\":\"Data Sensing Lab\"},\"name\":\"true\",\"status\":[\"present\",\"absent\",\"recent\"],\"properties\":{\"airQuality\":\"sigmas\",\"temperature\":\"celsius\",\"humidity\":\"percentage\",\"light\":\"lux\",\"altitude\":\"meters\",\"pressure\":\"millibars\"}},\"instances\":[{\"name\":\"Air Quality Sensor Mote\",\"status\":\"present\",\"unit\":{\"serial\":\"";
PROGMEM prog_char *loopPacket3 = "\",\"udn\":\"195a42b0-ef6b-11e2-99d0-";
PROGMEM prog_char *loopPacket4 = "-air-quality\"},\"info\":{\"airQuality\":";
PROGMEM prog_char *loopPacket5 = ",\"temperature\":";
PROGMEM prog_char *loopPacket6 = ",\"humidity\":";
PROGMEM prog_char *loopPacket7 = ",\"light\":";
PROGMEM prog_char *loopPacket8 = ",\"altitude\":";
PROGMEM prog_char *loopPacket9 = ",\"pressure\":";
PROGMEM prog_char *loopPacket10= "},\"uptime\":";
PROGMEM prog_char *loopPacket11= "}]}}}";

// All TSRP transmissions are via UDP to port 22601 on multicast address '224.192.32.19'.
EthernetUDP udp;
IPAddress ip(224,192,32,19);
unsigned int port = 22601;
#endif

// Pins
//
#ifndef TSRP
// 0         RX (from XBee)
// 1         TX (to XBee)
#endif

// 4         Shutdown for LT5334 RF Sensor (do not use)
const int rfShutdown = 4;

// 5         BMP085 XCLR
const int BMP085_XCLR = 5;

// 6         Button (Pressure Mat) 
//           (Also acts as pull-down pin for the Pressure Mat, if initially pulled LOW then pressure mat populated)
const int butPin = 6;

// 7         BMP085 EOC
const int BMP085_EOC = 7;

// 8         LED (Power)
// 9         LED (Loop Activity)
// 10        LED (Motion Detected - aka Button Pushed)
const int powr_led = 8;
const int loop_led = 9;
const int motn_led = 10;

// 11       RF Configuration Detector
const int rfPullDown = 11;

// 12        Pull-down pin for the Gas Sensor (If pulled LOW then Gas Sensor populated)
const int gasPullDown = 12;

// A0        Humidity Sensor (HTH-4030)
// A1        Light Sensor (TEMT6000)
// A2        Microphone (Adafruit Board)
// A3        Gas Sensor/RF Sensor (check Pin 5 and Pin 12 for which is populated if either)
//           NB: Cannot have both Gas Sensor and RF Sensor populated on same board as both share the output pin
const int hih4030 = A0;
const int temt6000 = A1;
const int micPin = A2;
const int analogPin = A3;

//Temperature & Humidity Sensor (BMP085) via I2C
// Uses I2C and the Wire Library
//
// NOTE: This is a 3.3V part
// NOTE: The SDA/SCL pins on the Uno and Leonardo are different.
// A4 & A5 (Arduino Uno)      
// 2 & 3 (Arduino Leonardo) 
//
// Route traces on your board to the dedicated SCL/SDA pins that the Uno R3 and Leonadro share.

// variables
short temperature;
long pressure;
const float p0 = 101325;             // Pressure at sea level (Pa)float altitude;
unsigned long samplingDelay = 20000; // default sampling delay in millis (SUPERCEDED BY ANYTHING STORED IN EEPROM)
unsigned long lastSampleTime = 0;
int micVal = 0;
int motionState = 0;
String inputString = "";   // a string to hold incoming data

// button variables
unsigned long total = 0;
unsigned long sinceLast = 0;

int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

int sentPacket = 0;

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers
unsigned long lastCounterTime = 0; // the last time the EEPROM counter pin written
unsigned long lastCallbackTime = 0;// the last time the data was written

// population flags
int hasRF = 0;
int hasGas = 0;
int hasMat = 0;


// SETUP ------------------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(9600);
#ifndef TSRP
  Serial1.begin(9600);  // to XBee
#endif
  
  // while the serial stream is not open, do nothing.
  //
  // WARNING: Needs to be removed for production release!!!
  //while (!Serial) ;
  Serial.println("Initializing...");

  Serial.print("SensorMote (Google I/O) v");
  Serial.println(CODE_VERSION);
  
  pinMode(rfShutdown, INPUT);    // TESTING ONLY. SHOULD BE OUTPUT
  pinMode(BMP085_XCLR, OUTPUT); 
  pinMode(BMP085_EOC, INPUT);    // End of conversion signal
  
  // Pull Down Resistors
  pinMode(butPin, INPUT);      
  pinMode(rfPullDown, INPUT);  
  pinMode(gasPullDown, INPUT); 

  digitalWrite(butPin, HIGH);  
  digitalWrite(rfPullDown, HIGH);  
  digitalWrite(gasPullDown, HIGH);  

  // LEDs
  pinMode( powr_led, OUTPUT);
  pinMode( loop_led, OUTPUT);
  pinMode( motn_led, OUTPUT);
  
  Serial.println("Checking LEDs...");
  
  digitalWrite(powr_led, HIGH);  // LED CHECK
  delay(1000);
  digitalWrite(powr_led, LOW);   // LED CHECK
  digitalWrite(loop_led, HIGH);  // LED CHECK
  delay(1000);
  digitalWrite(loop_led, LOW);   // LED CHECK
  digitalWrite(motn_led, HIGH);  // LED CHECK
  delay(1000);
  digitalWrite(motn_led, LOW);   // LED CHECK

  digitalWrite(powr_led, HIGH);  // LED OPERATIONAL
  digitalWrite(loop_led, HIGH);  // LED OPERATIONAL

  //get the saved sampling delay from memory
  unsigned long eepromNumber = getNumber();
  
  // ignore a zero value
  if (eepromNumber > 0) {
    samplingDelay = eepromNumber;
  }
  Serial.print( "Sampling delay = " );
  Serial.println( samplingDelay );

  Serial.println("Setting BMP085_XCLR high");
  digitalWrite(BMP085_XCLR, HIGH);  // Make sure BMP085 is on

  Serial.println("Calling bmp085.begin( )");  
  bmp.begin(); 
  
  if( digitalRead(rfPullDown) == LOW ) {
    Serial.println( "RF Sensor circuitry populated");
    hasRF = 1;
  } else {
    Serial.println( "No RF Sensor circuitry");
  }
  
  if( digitalRead(gasPullDown) == LOW ) {
    Serial.println( "Gas Sensor circuitry populated");
    hasGas = 1;
  } else {
    Serial.println( "No Gas Sensor circuitry");
  }
  
  if( digitalRead(butPin) == LOW ) {
    Serial.println( "Pressure Mat circuitry populated");
    hasMat = 1;
  } else {
    Serial.println( "No Pressure Mat circuitry");
  }
  digitalWrite(loop_led, LOW);
  
#ifdef  TSRP

  Serial.println("Waiting for DHCP address.");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Error: Failed to configure Ethernet using DHCP");
    while(1) {  }
  }

  Serial.print("MAC address: ");
  for (byte thisByte = 0; thisByte < 6; thisByte++) {
    if (mac[thisByte] < 0x0a) Serial.print("0");
    Serial.print(mac[thisByte], HEX);
    Serial.print(":");
  }
  Serial.println();

  Serial.print("IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();

  udp.beginMulti(ip,port);
#endif
}

// LOOP ------------------------------------------------------------------------------------------------------
//
// Main loop
// 1) Checks to see whether we should send a data sample
// 2) Looks for button pushes
// 3) Looks for config updates on the XBee network

void loop() {

  if ((millis() - lastSampleTime) > samplingDelay || lastSampleTime==0) {
    //Serial.println( "Getting Sample..." );
    getSample();
    lastSampleTime = millis();
  } 
  
  // Pressure Mat
  if ( hasMat ) {
  
    // read the state of the switch into a local variable:
    int reading = digitalRead(butPin);

    // check to see if you just pressed the button 
    // (i.e. the input went from LOW to HIGH),  and you've waited 
    // long enough since the last press to ignore any noise:  

    // If the switch changed, due to noise or pressing:
    if (reading != lastButtonState) {
      // reset the debouncing timer
      lastDebounceTime = millis();
    } 
    if ((millis() - lastDebounceTime) > debounceDelay) {
      // whatever the reading is at, it's been there for longer
      // than the debounce delay, so take it as the actual current state:
      buttonState = reading;
      if ( buttonState && !sentPacket ) {
        total = total + 1;
        sinceLast = sinceLast + 1;

        Serial.print("state = ");
        Serial.print( buttonState );
        Serial.print( ", total = " );
        Serial.println( total );
        sentPacket = 1;
  
      }  
    } 
    digitalWrite(motn_led, buttonState);
 
    // save the reading.  Next time through the loop,
    // it'll be the lastButtonState:
    if ( lastButtonState != reading ) {
       sentPacket = 0; 
    }
    lastButtonState = reading; 
  }
  
  // Check Mesh network for sample rate update
  checkForInput();  
}  

// GET SAMPLE ------------------------------------------------------------------------------------------------------
//
// Grabs sample data and outputs it to the serial port and XBee network

void getSample() {
  digitalWrite(loop_led,HIGH);

  // Temperature, Pressure, Light, Humidity, Altitude

  temperature = bmp.readTemperature();
  pressure = bmp.readPressure(); 
  int light = analogRead(temt6000);
  int humid = analogRead(hih4030);
  float relative_humid = ((0.0004*temperature + 0.149)*humid)-(0.0617*temperature + 24.436);

  //float altitude = (float)44330 * (1 - pow(((float) pressure/p0), 0.190295));
  float altitude = bmp.readAltitude(102210);

  // Microphone
  micVal = getSound(); 
 
  // Gas sensor
  int gasValue;
  int rfValue;
  if ( hasGas ) {
     gasValue =  analogRead(analogPin);
  } else if ( hasRF) {
     rfValue = analogRead(analogPin);
  }
    
  // Output
  Serial.print( "T = " );
  Serial.print( temperature );
  Serial.print( "C, P = ");
  Serial.print( pressure );
  Serial.print( "Pa, H = " );
  Serial.print( relative_humid );
  Serial.print( "%, Light = " );
  Serial.print( light );
  Serial.print( ", A = " );
  Serial.print( altitude );
  Serial.print( ", Mic = " );
  Serial.print( micVal );
  if ( hasGas ) {
     Serial.print( ", Gas = " );
     Serial.println( gasValue );
  } else if ( hasRF ) {
     Serial.print( ", RF = " );
     Serial.println( rfValue );
  } else if ( hasMat ) {
     Serial.print( ", total motion = " );
     Serial.print( total );
     Serial.print( ", since last = " );
     Serial.println( sinceLast );
  } else {
     Serial.println( "" );
    
  }
  
#ifndef TSRP
  if ( hasGas ) {
     Serial1.print("idigi_data:names=temperature,pressure,humidity,light,altitude,mic,gas&values=");
  } else if ( hasRF ) {
      Serial1.print("idigi_data:names=temperature,pressure,humidity,light,altitude,mic,rf&values=");
  } else if ( hasMat ) {
      Serial1.print("idigi_data:names=temperature,pressure,humidity,light,altitude,mic,motion,totalmotion&values=");
  } else {
     Serial1.print("idigi_data:names=temperature,pressure,humidity,light,altitude,mic&values=");
  }
  Serial1.print( temperature );
  Serial1.print(",");
  Serial1.print( pressure );
  Serial1.print(",");
  Serial1.print( relative_humid );
  Serial1.print(",");
  Serial1.print( light );
  Serial1.print(",");
  Serial1.print( altitude );
  Serial1.print(",");
  Serial1.print( micVal );
  if ( hasGas ) {
     Serial1.print(",");
     Serial1.println( gasValue );
     Serial1.println("&units=C,Pa,%,Lux,m,X,X");
  } else if ( hasRF ) {
     Serial1.print(",");
     Serial1.println( rfValue );
     Serial1.println("&units=C,Pa,%,Lux,m,X,X");
  } else if ( hasMat ) {
     Serial1.print(",");
     Serial1.print( sinceLast );
     Serial1.print(",");
     Serial1.print( total );
     Serial1.println("&units=C,Pa,%,Lux,m,X,X,X");
  } else {
     Serial1.println("&units=C,Pa,%,Lux,m,X");
  }
#else
  char  buffer[24];
  char packetBuffer[768];
  unsigned long now = millis();

  strcpy(packetBuffer,(char*)pgm_read_word(&loopPacket1) );
  strcat(packetBuffer, ultoa( requestID, buffer, 10) );

  strcat(packetBuffer,(char*)pgm_read_word(&loopPacket2) );
  for (byte thisByte = 0; thisByte < 6; thisByte++) {
      sprintf(buffer, "%02x", mac[thisByte] );
      strcat(packetBuffer, buffer);
  }

  strcat(packetBuffer,(char*)pgm_read_word(&loopPacket3) );
  for (byte thisByte = 0; thisByte < 6; thisByte++) {
      sprintf(buffer, "%02x", mac[thisByte] );
      strcat(packetBuffer, buffer);
  }

  strcat(packetBuffer,(char*)pgm_read_word(&loopPacket4) );
  strcat(packetBuffer, dtostrf(gasValue, 12, 4, buffer));

  strcat(packetBuffer,(char*)pgm_read_word(&loopPacket5) );
  strcat(packetBuffer, dtostrf(temperature, 12, 4, buffer));

  strcat(packetBuffer,(char*)pgm_read_word(&loopPacket6) );
  strcat(packetBuffer, dtostrf(relative_humid, 12, 4, buffer));

  strcat(packetBuffer,(char*)pgm_read_word(&loopPacket7) );
  strcat(packetBuffer, dtostrf(light, 12, 4, buffer));

  strcat(packetBuffer,(char*)pgm_read_word(&loopPacket8) );
  strcat(packetBuffer, dtostrf(altitude, 12, 4, buffer));

  strcat(packetBuffer,(char*)pgm_read_word(&loopPacket9) );
  strcat(packetBuffer, dtostrf(pressure * 0.01, 12, 4, buffer));

  strcat(packetBuffer,(char*)pgm_read_word(&loopPacket10) );
  strcat(packetBuffer, ultoa( now, buffer, 10) );

  strcat(packetBuffer,(char*)pgm_read_word(&loopPacket11) );

  Serial.println(packetBuffer);

  udp.beginPacket(udp.remoteIP(), udp.remotePort());
  udp.write(packetBuffer);
  udp.endPacket();
  requestID = requestID + 1;
#endif

  digitalWrite(loop_led, LOW);
  sinceLast = 0;
  
}

// CHECK FOR INPUT ------------------------------------------------------------------------------------------------------
//
// Checks for configuration updates over the XBee network

void checkForInput() {
#ifndef TSRP
  //Serial.println( "Checking for Input from Mesh Network" );
  /*
  Send the following type of command as a POST from iDigi to set the sampling time:
  <sci_request version="1.0">
   <data_service>
    <targets>
      <device id="00000000-00000000-00409DFF-FF521DBA"/>
    </targets>
    <requests>
      <device_request target_name="xig">
        &lt;send_data hw_address="00:13:A2:00:40:A2:0C:62!"&gt;set_sampling_time=20000>&lt;/send_data&gt;
      </device_request>
    </requests>
   </data_service>
  </sci_request>
  */
  if (Serial1.available()) {
    while (Serial1.available()) {
      // get the new byte:
      char inChar = (char)Serial1.read(); 
      // add it to the inputString:
      inputString += inChar;
      // if the incoming character is a close bracket we're done
      if (inChar == '>') {
        Serial.print("String received: \"");
        Serial.print(inputString);
        Serial.println("\"");
        if (inputString.indexOf("set_sampling_time=">=0)) {
          samplingDelay = inputString.substring(inputString.indexOf("set_sampling_time=")+18,inputString.length()-1).toInt();
          setNumber(samplingDelay);
          Serial.print("Set Sampling Delay to: ");
          Serial.println(samplingDelay);
        }
        inputString="";
      } 
    }
  }
#endif
}

// GET SOUND ------------------------------------------------------------------------------------------------------
//
// Does something sensible(ish) with the microphone input

int getSound() {
  static int average = SILENT_VALUE; // stores the neutral position for the mic
  static int avgEnvelope = 0; // stores the average sound pressure level
  int avgSmoothing = 10; // larger values give more smoothing for the average
  int envSmoothing = 2; // larger values give more smoothing for the envelope
  int numSamples=1000; //how many samples to take
  int envelope=0; //stores the mean sound from many samples
  for (int i=0; i<numSamples; i++) {
    int sound=analogRead(micPin); // look at the voltage coming from the mic
    int sampleEnvelope = abs(sound - average); // the distance from this reading to the average
    envelope = (sampleEnvelope+envelope)/2;
    avgEnvelope = (envSmoothing * avgEnvelope + sampleEnvelope) / (envSmoothing + 1);
    //Serial.println(avgEnvelope);
    average = (avgSmoothing * average + sound) / (avgSmoothing + 1); //create a new average
  }
  return envelope;
}
