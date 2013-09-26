/*------------------------------------------------------\
|	Copyleft kippkitts LLC, 2013    		|
|       http://creativecommons.org/licenses/by-sa/3.0/  |
|       Attribution-ShareAlike 3.0 Unported             |
|							|
|	kippkitts, LLC.			          	|
|	999 Main Street STE 705                         |
|       Pawtucket, RI USA                		|
|	Tel: 401-400-0KIT, Fax: 401-475-3574		|
|							|
| Arduino code: DSL Workshop Shield      		|
|							|
| kippkitts, LLC HAS MADE EVERY EFFORT TO INSURE THAT	|
| THE CODE CONTAINED IN THIS RELEASE IS ERROR-FREE.	|
| HOWEVER, kippkitts DOES NOT WARRANT THE		|
| OPERATION OF THE CODE, ASSOCIATED FILES,      	|
| SCHEMATICS &/OR OTHER ENGINEERING MATERIAL.		|
|							|
| THESE MATERIALS WERE PRODUCED FOR PROTOTYPE		|
| PURPOSES ONLY, AND ARE NOT INTENDED FOR		|
| PRODUCTION. THEY ARE RELEASED SOLELY TO GIVE A	|
| DEVELOPER PRELIMINARY ASSISTANCE IN            	|
| THEIR PROJECT.                			|
|							|
| THIS CODE IS PROVIDED "AS IS".                        |
| kippkitts ACCEPTS NO RESPONSIBILITY OR LIABILITY	|
| FOR THE USE OF THIS MATERIAL, OR ANY PRODUCTS	        |
| THAT MAY BE BUILT WHICH UTILIZE IT.			|
|							|
|							|
|	File name : 	DSL_Workshop_Shield_ver2.ino	|
|	Written by: 	Kipp Bradford    	        |
|	Final edit date : 	24 Sept 2013      	|
|							|
|							|
|							|
|Notes:							|
|							|
|							|
|		NOT FOR PRODUCTION			|
|							|
\_______________________________________________________*/

// YOU MUST INSTALL THE ADAFRUIT DHT 22 LIBRARY
// You can find the library here:
// https://github.com/adafruit/DHT-sensor-library

// I've included the library like this:
#include "DHT.h"

// I define what pin I've connected the Temp/Humidity sensor to
#define DHTPIN 2

// Uncomment the proper DHTxx device to get the correct settings:
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT11   // DHT 11 
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Here, define the pins that we'll be using with this program
// Pins
//
// 0         RX (from XBee)
// 1         TX (to XBee)

// 2         DHT22 data pin
// 3         Button (or Motion Detector) 
// 8         LED (Power)
// 9         LED (Motion Detected - aka Button Pushed)
// 13        LED (System Activity)

// A0        Microphone (Adafruit Board)

// Now assign names to the pins for clear coding:
const int PIR_Pin = 3;
const int power_led = 8;
const int motion_led = 9;
const int status_led = 13;
const int micPin = A0;

// Declare the variables for this program:
int buttonState;   // the current reading from the input pin
int micValue;      // the current reading from the microphone

float h = 0;  // current humidity reading
float t = 0;  // current temperature reading

// We need to create a DHT device so we can get our data
DHT dht(DHTPIN, DHTTYPE);


// SETUP ------------------------------------------------------------------------------------------------------
void setup() 
{
  // "Serial" sends to your computer via USB
  Serial.begin(9600);
  // "Serial1" sends to your computer via the XBee
  Serial1.begin(9600);  // to XBee
  
  Serial.print("Sensor Workshop V2.0");
  
  // Let's set the direction of the I/O lines
  pinMode( power_led, OUTPUT);
  pinMode( status_led, OUTPUT);
  pinMode( motion_led, OUTPUT);
  pinMode( micPin, INPUT);
  pinMode( PIR_Pin, INPUT);
  
  digitalWrite(PIR_Pin, HIGH);  // Turn on the pull-up if you are just using a pushbutton
  
  // Lets test some LEDs!
  Serial.println("Checking LEDs...");
  
  digitalWrite(power_led, HIGH);  // LED CHECK
  delay(1000);
  digitalWrite(power_led, LOW);   // LED CHECK
  digitalWrite(status_led, HIGH);  // LED CHECK
  delay(1000);
  digitalWrite(status_led, LOW);   // LED CHECK
  digitalWrite(motion_led, HIGH);  // LED CHECK
  delay(1000);
  digitalWrite(motion_led, LOW);   // LED CHECK

  digitalWrite(power_led, HIGH);  // Let's leave this on for operation
  digitalWrite(status_led, HIGH);  // Let's leave this on for operation

  delay(2000);  // This little delay let's you see the light(s)

}

// LOOP ------------------------------------------------------------------------------------------------------
void loop()
{

  // read the state of the switch into a variable:
  buttonState = digitalRead(PIR_Pin);
  // now write that output to the LED
  digitalWrite(motion_led, buttonState);

  // Send the status of the motion detector to the USB
  Serial.print("PIR State: ");
  Serial.println(buttonState);
  // Send the status of the motion detector to the XBee
  Serial1.print("PIR State: ");
  Serial1.println(buttonState);


  // Read the mic value
  micValue = analogRead(micPin);    

  // Send the status of the mic detector to the USB
  Serial.print("Mic Value: ");
  Serial.println(micValue);
  // Send the status of the mic detector to the XBee
  Serial1.print("Mic Value: ");
  Serial1.println(micValue);


  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(t) || isnan(h))
  {
    Serial.println("Failed to read from DHT");
    Serial1.println("Failed to read from DHT");
  } 
  else 
  {
    Serial.print("Humidity: "); 
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: "); 
    Serial.print(t);
    Serial.println(" *C");

    Serial1.print("Humidity: "); 
    Serial1.print(h);
    Serial1.print(" %\t");
    Serial1.print("Temperature: "); 
    Serial1.print(t);
    Serial1.println(" *C");
  }
  
  delay(500);
  digitalWrite(status_led, HIGH);  // LEDs let us know that we are alive!
  delay(500);
  digitalWrite(status_led, LOW);   // LEDs let us know that we are alive!
}  

