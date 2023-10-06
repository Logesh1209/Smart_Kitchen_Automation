// Connections
// OLED  D1 - SCL, D2 - SDA
// DHT D3
// MQ135 A0
// PIR D4
// EXHAUST FAN, BUZZER D0
// RELAYS FAN D5, LIGHT D6, OVEN D7, FRIDGE D8

#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <MQ135.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>


// Blynk
#define BLYNK_AUTH_TOKEN "p3Fr3mOaKXGoozm_eaKgujPN4uFyAgq4"

// WIFI
#define SSID "Logesh S"
#define PW "87654321"

// Blynk Virtual Pins
// V0 Temperature, V1 Humidity, V2 AQ, V3 Fan, V4 Light
// V5 Oven, V6 Fridge, V7 Alarm, V8 Person Detected

// Relay
#define FAN D5
#define LIGHT D6
#define OVEN D7
#define FRIDGE D8

// Time delay in ms
const int DELAY = 2000;

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// DHT
#define DHT_PIN D3
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// MQ135
#define MQ_PIN A0
MQ135 gasSensor = MQ135(MQ_PIN);
int ALARM_STATE;
const int AQ_LIMIT =  5;

// Exhaust Fan and Buzzer
#define ALARM D0

// PIR
#define PIR_PIN D4
int PIR_VALUE = 0;


// First time setup
void setup()  
{  
  Serial.begin(9600);

  Blynk.begin(BLYNK_AUTH_TOKEN, SSID, PW);

  // Configure IO pins
  pinMode(PIR_PIN, INPUT);
  pinMode(ALARM, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(LIGHT, OUTPUT);
  pinMode(OVEN, OUTPUT);
  pinMode(FRIDGE, OUTPUT);

  digitalWrite(ALARM, LOW);
  digitalWrite(FAN, HIGH);
  digitalWrite(LIGHT, HIGH);
  digitalWrite(OVEN, HIGH);
  digitalWrite(FRIDGE, HIGH);

  dht.begin();
  // Loop infinitely until connection is established with OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
  {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  delay(DELAY);

  display.clearDisplay();
  display.setTextColor(WHITE);    
}  


// Blynk State Syncing
BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V3);  // will cause BLYNK_WRITE to be executed
  Blynk.syncVirtual(V4);
  Blynk.syncVirtual(V5);
  Blynk.syncVirtual(V6);
}


// Repeatedly call
void loop()  
{ 
  Blynk.run();

  dht_run();
  delay(DELAY);

  display.clearDisplay();

  mq_run();
  delay(DELAY);

  pir_run();

  display.clearDisplay();
}  


void dht_run()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  } 

  Serial.print("Temperature in C: ");  
  Serial.println((dht.readTemperature()));  
  Serial.print("Humidity in %: ");  
  Serial.println((dht.readHumidity()));

  // Blynk display
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);

  // OLED Display temperature
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temperature: ");
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print(t);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");
 
  // OLED Display humidity
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Humidity: ");
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print(h);
  display.print(" %");

  display.display();
}


void mq_run()
{
  float AQI = gasSensor.getPPM();
  Serial.print("Air Quality in PPM: ");
  Serial.println(AQI);

  // Blynk display air quality
  Blynk.virtualWrite(V2, AQI);

  // OLED display air quality
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Air Quality: ");
  display.setTextSize(2);
  display.setCursor(0, 15);
  display.print(AQI);
  display.print(" PPM");

  if(AQI > AQ_LIMIT)
  {
    Serial.println("Smoke Detected!!!");
    display.setTextSize(1);
    display.setCursor(0, 45);
    display.print("Smoke Warning!");
    digitalWrite(ALARM, HIGH);
    Blynk.virtualWrite(V7, 1);
  }
  else
  {
    digitalWrite(ALARM, LOW);
    Blynk.virtualWrite(V7, 0);
  }

  display.display();
}


void pir_run()
{
  PIR_VALUE = digitalRead(PIR_PIN);

  if (PIR_VALUE == 1) 
  {
    Serial.println("Motion Detected");
    Blynk.virtualWrite(V8, 1);
  } 
  else if (PIR_VALUE == 0)
  {
    Serial.println("No Motion"); 
    Blynk.virtualWrite(V8, 0);
  }
}

// Fan
BLYNK_WRITE(V3) // Executes when the value of virtual pin changes
{
  if(param.asInt() == 1)
  {
    // execute this code if the switch widget is now ON
    digitalWrite(FAN, LOW);  // Set digital pin 2 HIGH
  }
  else
  {
    // execute this code if the switch widget is now OFF
    digitalWrite(FAN, HIGH);    
  }
}

// Light
BLYNK_WRITE(V4) // Executes when the value of virtual pin changes
{
  if(param.asInt() == 1)
  {
    // execute this code if the switch widget is now ON
    digitalWrite(LIGHT, LOW);  // Set digital pin 2 HIGH
  }
  else
  {
    // execute this code if the switch widget is now OFF
    digitalWrite(LIGHT, HIGH);    
  }
}

// Oven
BLYNK_WRITE(V5) // Executes when the value of virtual pin changes
{
  if(param.asInt() == 1)
  {
    // execute this code if the switch widget is now ON
    digitalWrite(OVEN, LOW);  // Set digital pin 2 HIGH
  }
  else
  {
    // execute this code if the switch widget is now OFF
    digitalWrite(OVEN, HIGH);    
  }
}

// Fridge
BLYNK_WRITE(V6) // Executes when the value of virtual pin changes
{
  if(param.asInt() == 1)
  {
    // execute this code if the switch widget is now ON
    digitalWrite(FRIDGE, LOW);  // Set digital pin 2 HIGH
  }
  else
  {
    // execute this code if the switch widget is now OFF
    digitalWrite(FRIDGE, HIGH);    
  }
}
