#include <DHT.h>

// Strata, New York 2012
// Sensor Mote Code

// Software Only Version of the code

//  Status LED
//  GND, 13

//  Motion LED
//  3 (GND), 2

//  Power LED
//  A4 (GND), A5

//  DHT    11    +5V
//         10    OUT and Resistor to +5V
//          9    NOT USED
//          8    GND

//  PIR     7    GND
//          6    OUT
//          5    +5V

//  MIC    A0    OUT
//         A1    GND
//         A2    +5V

#define CODE_VERSION 2.1

#define DHTTYPE DHT22     // DHT 22  (AM2302)
#define SILENT_VALUE 380  // Starting neutral microphone value (self-correcting)

// LEDs
int statusLED = 13; // The status LED uses the adjacent ground pin

int motionLED = 2;
int motionLED_gnd = 3;

const int powerLED = A5;
const int powerLED_gnd = A4;

// DHT
int dhtPin_5v = 11;
int dhtPin = 10;
int dhtPin_gnd = 8;
DHT dht(dhtPin, DHTTYPE);

// PIR
int pirPin_gnd = 7;
int pirPin = 6;
int pirPin_5v = 5;

int pirState = LOW; // Start off assuming no motion detected
int pirVal = 0;
int motionState = 0;

// MIC
const int micPin = A0;
const int micPin_gnd = A1;
const int micPin_5v = A2;

int micVal = 0;

// Setup
void setup() {

 // LEDs 
 pinMode( statusLED, OUTPUT );
 pinMode( motionLED, OUTPUT );
 pinMode( powerLED, OUTPUT );

 digitalWrite(powerLED, HIGH);   // Power ON
 
 // DHT
 pinMode( dhtPin, INPUT );
 
 // PIR
 pinMode( pirPin, INPUT );
 
 // 5V Pins
 pinMode( dhtPin_5v, OUTPUT);
 pinMode( pirPin_5v, OUTPUT );
 pinMode( micPin_5v, OUTPUT );

 digitalWrite( dhtPin_5v, HIGH );  
 digitalWrite( pirPin_5v, HIGH );  
 digitalWrite( micPin_5v, HIGH );  

 // Gound Pins
 pinMode( motionLED_gnd, OUTPUT );
 pinMode( powerLED_gnd, OUTPUT );
 
 pinMode( dhtPin_gnd, OUTPUT );
 pinMode( pirPin_gnd, OUTPUT );
 pinMode( micPin_gnd, OUTPUT);

 digitalWrite( motionLED_gnd, LOW );  
 digitalWrite( powerLED_gnd, LOW );  
 digitalWrite( dhtPin_gnd, LOW );  
 digitalWrite( pirPin_gnd, LOW );  
 digitalWrite( micPin_gnd, LOW );  
 
 // Begin 
 Serial.begin(9600);  
 Serial1.begin(9600);  
 dht.begin(); 

 Serial.print("SensorMote v");
 Serial.println(CODE_VERSION); 
 
}

void loop() {
  digitalWrite(statusLED, HIGH);
  
  // DHT-22
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  // PIR
  pirVal = digitalRead(pirPin);  // read input value
  if(pirVal == HIGH){            // check if the input is HIGH
    if(pirState == LOW){
      // we have just turned on
      motionState = 1;
      digitalWrite(motionLED, HIGH);  // turn LED ON
      
      // We only want to print on the output change, not state
      pirState = HIGH;
    }
  }else{
    if(pirState == HIGH){
      // we have just turned of
      motionState = -1;
      digitalWrite(motionLED, LOW);// turn LED OFF

      // We only want to print on the output change, not state
      pirState = LOW;
    }
  } 
  
  // Microphone
  micVal = getSound(); 

  // Send packet
  // Check that we have a reading from the DHT sensor
  if (isnan(t) || isnan(h)) {  
    Serial.println("Error: Failed to read from DHT");
    
    Serial.print("idigi_data:names=pir,motion,mic&values=");
    Serial.print(pirVal);
    Serial.print(",");
    Serial.print(motionState);
    Serial.print(",");
    Serial.print(micVal);
    Serial.println("&units=X,X,X");

    Serial1.print("idigi_data:names=pir,motion,mic&values=");
    Serial1.print(pirVal);
    Serial1.print(",");
    Serial1.print(motionState);
    Serial1.print(",");
    Serial1.print(micVal);
    Serial1.println("&units=X,X,X");
    
  } else {
    Serial.print("idigi_data:names=humidity,temperature,pir,motion,mic&values=");
    Serial.print(h);
    Serial.print(",");
    Serial.print(t);
    Serial.print(",");
    Serial.print(pirVal);
    Serial.print(",");
    Serial.print(motionState);
    Serial.print(",");
    Serial.print(micVal);
    Serial.println("&units=%,C,X,X,X");
    
    Serial1.print("idigi_data:names=humidity,temperature,pir,motion,mic&values=");
    Serial1.print(h);
    Serial1.print(",");
    Serial1.print(t);
    Serial1.print(",");
    Serial1.print(pirVal);
    Serial1.print(",");
    Serial1.print(motionState);
    Serial1.print(",");
    Serial1.print(micVal);
    Serial1.println("&units=%,C,X,X,X");
  }

  // reset motion state
  motionState = 0;
  
  // end packet
  digitalWrite(statusLED, LOW);   
  delay(250);
    
}

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
