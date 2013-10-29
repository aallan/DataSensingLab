
#include "DHT.h"
#define DHTTYPE DHT22   // DHT 22  (AM2302)

int dhtPin = 2;
DHT dht(dhtPin, DHTTYPE);

int powerPin = 13;
int loopPin = 12;
int lightPin = 0;

// Setup
void setup() {
    pinMode(dhtPin, INPUT);     // declare DHT sensor pin as input
    pinMode(powerPin, OUTPUT);
    pinMode(loopPin, OUTPUT);
    
    Serial.begin(9600);  
    Serial1.begin(9600); 
    
    dht.begin();
    digitalWrite(powerPin, HIGH);
}

// Loop
void loop() {
    digitalWrite(loopPin, HIGH);
    
    // DHT-22
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    float t = dht.readTemperature();
  
    if (isnan(t) || isnan(h)) {  
      Serial.println("Error: Failed to read from DHT");
      
    } else {
    
      int light = analogRead(lightPin);
      
      // Send to USB serial port
      Serial.print( "T = " ),
      Serial.print( t );
      Serial.print( "C, H = " );
      Serial.print( h );
      Serial.print( "%, light = " ); 
      Serial.println( light );
    
      // Send to XBee on hardware serial port
      Serial1.print( t );
      Serial1.print( "," );
      Serial1.print( h );  
      Serial1.print( "," );
      Serial1.println( light );    
    
    }    
  
    digitalWrite(loopPin, LOW);
    delay(250);
  
}    
