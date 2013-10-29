
#include <DHT.h>

#define DHTTYPE DHT22     // DHT 22  (AM2302)
#define SILENT_VALUE 380  // Starting neutral microphone value (self-correcting)

// LEDs
int statusLED = 13;       // The status LED uses the adjacent ground pin
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
int pirPin_gnd = 5;
int pirPin = 6;
int pirPin_5v = 7;
int pirState = LOW;
int pirVal = 0;
int motionState = 0;

// Start off assuming no motion detected
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
   digitalWrite(powerLED, HIGH);

   // DHT
   pinMode( dhtPin, INPUT );

   // PIR
   pinMode( pirPin, INPUT );

   // Power ON
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
}

void loop() {
  digitalWrite(statusLED, HIGH);

  // DHT-22
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
  } else {
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
  // Check that we have a reading from the DHT sensor
  if (isnan(t) || isnan(h)) {
    Serial.println("Error: Failed to read from DHT");
    Serial.print(pirVal);
    Serial.print(",");
    Serial.print(motionState);
    Serial.print(",");
    Serial.println(micVal);
    
    Serial1.print(pirVal);
    Serial1.print(",");
    Serial1.print(motionState);
    Serial1.print(",");
    Serial1.println(micVal);    
    
  } else {
    Serial.print(h);
    Serial.print(",");
    Serial.print(t);
    Serial.print(",");
    Serial.print(pirVal);
    Serial.print(",");
    Serial.print(motionState);
    Serial.print(",");
    Serial.println(micVal);
    
    Serial1.print(h);
    Serial1.print(",");
    Serial1.print(t);
    Serial1.print(",");
    Serial1.print(pirVal);
    Serial1.print(",");
    Serial1.print(motionState);
    Serial1.print(",");
    Serial1.println(micVal);
    
  }
  // reset motion state
  motionState = 0;
  
  // end packet
  digitalWrite(statusLED, LOW);
  delay(2000);
}

int getSound() {
  static int average = SILENT_VALUE; 
  static int avgEnvelope = 0; 
  int avgSmoothing = 10; 
  int envSmoothing = 2; 
  int numSamples=1000; 
  int envelope=0; 
  for (int i=0; i<numSamples; i++) {
    int sound=analogRead(micPin); 
    int sampleEnvelope = abs(sound - average); 
    envelope = (sampleEnvelope+envelope)/2;
    avgEnvelope = (envSmoothing * avgEnvelope + sampleEnvelope) / (envSmoothing + 1);
    average = (avgSmoothing * average + sound) / (avgSmoothing + 1);
  }
  return envelope;
}
