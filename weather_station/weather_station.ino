//##################################################################################################################
//##                                      ELET2415 DATA ACQUISITION SYSTEM CODE                                   ##
//##                                                                                                              ##
//##################################################################################################################

// LIBRARY IMPORTS
#include <rom/rtc.h> 
#include <math.h>  // https://www.tutorialspoint.com/c_standard_library/math_h.htm 
#include <ctype.h>

// ADD YOUR IMPORTS HERE
#include <PubSubClient.h>
#include "DHT.h"
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Wire.h>
#include <Adafruit_BMP280.h>


#ifndef _WIFI_H 
#include <WiFi.h>
#endif

#ifndef STDLIB_H
#include <stdlib.h>
#endif

#ifndef STDIO_H
#include <stdio.h>
#endif

#ifndef ARDUINO_H
#include <Arduino.h>
#endif 
 
#ifndef ARDUINOJSON_H
#include <ArduinoJson.h>
#endif

 

// DEFINE VARIABLES
#define ARDUINOJSON_USE_DOUBLE      1
const int dry = 3200; // value for dry sensor
const int wet = 1500; // value for wet sensor


// DEFINE THE CONTROL PINS FOR THE DHT22
#define DHTPIN 33
#define DHTTYPE DHT22

// DEFINE BMP280 PINS
#define BMP280_ADDRESS 0x76


// DEFINE SOIL SENSOR PINS
#define SSPIN 35

// DEFINE TFT PINS
#define TFT_DC    17
#define TFT_CS    5
#define TFT_RST   16
#define TFT_CLK   18
#define TFT_MOSI  23
#define TFT_MISO  19



// WiFi
const char *ssid = "MonaConnect"; // Enter your WiFi name
const char *password = "";  // Enter WiFi password

// MQTT Broker
const char *mqtt_server = "broker.emqx.io";
static const char* pubtopic       = "620164419";                    // Add your ID number here
static const char* subtopic[]     = {"620164419_sub","/elet2415"};  // Array of Topics(Strings) to subscribe to
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;




// TASK HANDLES 
TaskHandle_t xMQTT_Connect          = NULL; 
TaskHandle_t xNTPHandle             = NULL;  
TaskHandle_t xLOOPHandle            = NULL;  
TaskHandle_t xUpdateHandle          = NULL;
TaskHandle_t xButtonCheckeHandle    = NULL;  

// FUNCTION DECLARATION   
void checkHEAP(const char* Name);   // RETURN REMAINING HEAP SIZE FOR A TASK
void initMQTT(void);                // CONFIG AND INITIALIZE MQTT PROTOCOL
unsigned long getTimeStamp(void);   // GET 10 DIGIT TIMESTAMP FOR CURRENT TIME
void callback(char* topic, byte* payload, unsigned int length);
void initialize(void);
bool publish(const char *topic, const char *payload); // PUBLISH MQTT MESSAGE(PAYLOAD) TO A TOPIC
void vButtonCheck( void * pvParameters );
void vUpdate( void * pvParameters );  
bool isNumber(double number);
 

/* Declare your functions below */ 



/* Init class Instances for the DHT22 etcc */
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp; // I2C
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
 
  

//############### IMPORT HEADER FILES ##################
#ifndef NTP_H
#include "NTP.h"
#endif

#ifndef MQTT_H
#include "mqtt.h"
#endif

// Temporary Variables 

void setup() {
  Serial.begin(115200);  // INIT SERIAL  

  // INITIALIZE ALL SENSORS AND DEVICES
  
  Serial.println(F("DHT22 test!"));
  dht.begin();

  while ( !Serial ) delay(100);   // wait for native usb
  Serial.println(F("BMP280 test"));
  unsigned status;
  status = bmp.begin(BMP280_ADDRESS);
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
  


  int16_t x1, y1;
  uint16_t w, h;
  const char *welcomeText = "Welcome Toff";
  tft.getTextBounds(welcomeText, 0, 0, &x1, &y1, &w, &h);
  int x_center = 160 - (w / 2);
  tft.setCursor(x_center, 80);
  tft.print(welcomeText);

    // Smaller font for "here is the weather data for today"
  tft.setTextSize(1);
  const char *subText = "Here is the weather data for today";
  tft.getTextBounds(subText, 0, 0, &x1, &y1, &w, &h);
  x_center = 160 - (w / 2);
  tft.setCursor(x_center, 110);
  tft.print(subText);
  delay(3000);
  tft.fillScreen(ILI9341_BLACK);

  int16_t x2, y2;
  uint16_t w1, h1;
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  const char *Loading = "Loading...";

  tft.getTextBounds(Loading, 0, 0, &x2, &y2, &w1, &h1);
  int x_center1 = 160 - (w1 / 2);
  tft.setCursor(x_center1, 80);
  tft.print(Loading);
    

  delay(3000); // Show welcome screen for 3 seconds

    // Clear the screen
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
 
  /* Add all other necessary sensor Initializations and Configurations here */
  //pinMode(SSPIN,INPUT_PULLUP);

  initialize();     // INIT WIFI, MQTT & NTP 
  // vButtonCheckFunction(); // UNCOMMENT IF USING BUTTONS INT THIS LAB, THEN ADD LOGIC FOR INTERFACING WITH BUTTONS IN THE vButtonCheck FUNCTION
}
  


void loop() {
  // put your main code here, to run repeatedly:       
  vTaskDelay(1000 / portTICK_PERIOD_MS); 
}





  
//####################################################################
//#                          UTIL FUNCTIONS                          #       
//####################################################################
void vButtonCheck( void * pvParameters )  {
  configASSERT( ( ( uint32_t ) pvParameters ) == 1 );     
    
  for( ;; ) {
    // Add code here to check if a button(S) is pressed
    // then execute appropriate function if a button is pressed  

    vTaskDelay(200 / portTICK_PERIOD_MS);  
  }
}

void vUpdate( void * pvParameters )  {
  configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
  for( ;; ) {
    // #######################################################
    // ## This function must PUBLISH to topic every second. ##
    // #######################################################

    // 1. Read Humidity and save in variable below
    double h = dht.readHumidity();
        
    // 2. Read temperature as Celsius   and save in variable below
    double t = dht.readTemperature();

    double p = bmp.readPressure();

    double a = bmp.readAltitude(1013.25);

    double m = analogRead(SSPIN);
  
    double moisturePercentage = map(m, wet, dry, 100, 0); // Calculate moisture percentage

    if (moisturePercentage > 100){
      moisturePercentage = 100;
    }
    else if(moisturePercentage < 0){
      moisturePercentage = 0;
    }

    // if (isnan(h) || isnan(t) || isnan(p) || isnan(a) || isnan(m)) {
    //   Serial.println(F("Failed to read from sensor!"));
    //   return;
    // }

    if (!isnan(t)){
      double hic = dht.computeHeatIndex(t, h, false);
      // ##Publish update according to ‘{"id": "student_id", "timestamp": 1702212234, "temperature": 30, "humidity":90, "heatindex": 30}’

      // 1. Create JSon object
      JsonDocument doc; // Create JSon object
      
      // 2. Create message buffer/array to store serialized JSON object
      char message[1100]  = {0};
      
      // 3. Add key:value pairs to JSon object based on above schema
      doc["id"]         = "620164419";
      doc["timestamp"]  = getTimeStamp();
      doc["temperature"]     = t;
      doc["humidity"]       = h;
      doc["heatindex"]       = hic;
      doc["pressure"]       = p;
      doc["altitude"]       = a;
      doc["moisture"]       = moisturePercentage;

      // 4. Seralize / Covert JSon object to JSon string and store in message array
      serializeJson(doc, message);
      publish("topic", message);
      // 5. Publish message to a topic sobscribed to by both backend and frontend
      if(mqtt.connected() ){
        //Serial.print("I am publishing to the back adn frontend");
        publish(pubtopic, message);
      }             
      //Serial.print("I am recieving data");
      Serial.print(F("Humidity: "));
      Serial.print(h);
      Serial.print(F("%  Temperature: "));
      Serial.print(t);
      Serial.print(F("°C  Heat index: "));
      Serial.print(hic);
      Serial.print(F("°C  Pressure: "));
      Serial.print(p);
      Serial.print(F("Pa  Approx altitude: "));
      Serial.print(a); /* Adjusted to local forecast! */
      Serial.print(F("m  Moisture: "));
      Serial.print(moisturePercentage);
      Serial.println("");

      tft.setCursor(40, 20); 
      tft.print("WEATHER STATION DATA");
      tft.println();
      tft.println();
      //Print Temperature
      tft.print("Temperature: ");
      tft.print(t);
      tft.write(0xF8);
      tft.print("C");
      tft.println();
      tft.println();

      //Print Humidity
      tft.print("Humidity: ");
      tft.print(h);
      tft.write(0x25);
      tft.println();
      tft.println();

      //Print Heat Index
      tft.print("Heat Index: ");
      tft.print(hic);
      tft.write(0xF8);
      tft.print("C");
      tft.println();
      tft.println();

      //Print Pressure
      tft.print("Pressure: ");
      tft.print(p);
      tft.print("Pa");
      tft.println();
      tft.println();

      //Print Altitude
      tft.print("Altitude: ");
      tft.print(a);
      tft.print("m");
      tft.println();
      tft.println();

      //Print Moisture
      tft.print("Moisture: ");
      tft.print(moisturePercentage);
      tft.print("");
      tft.println();
    }
     

    vTaskDelay(1000 / portTICK_PERIOD_MS);  
  }
}

 

unsigned long getTimeStamp(void) {
  // RETURNS 10 DIGIT TIMESTAMP REPRESENTING CURRENT TIME
  time_t now;         
  time(&now); // Retrieve time[Timestamp] from system and save to &now variable
  return now;
}


void callback(char* topic, byte* payload, unsigned int length) {
  // ############## MQTT CALLBACK  ######################################
  // RUNS WHENEVER A MESSAGE IS RECEIVED ON A TOPIC SUBSCRIBED TO
  
  Serial.printf("\nMessage received : ( topic: %s ) \n",topic ); 
  char *received = new char[length + 1] {0}; 
  
  for (int i = 0; i < length; i++) { 
    received[i] = (char)payload[i];    
  }

  // PRINT RECEIVED MESSAGE
  Serial.printf("Payload : %s \n",received);

 
  // CONVERT MESSAGE TO JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, received);  

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }


}

bool publish(const char *topic, const char *payload){   
  bool res = false;
  try{
    res = mqtt.publish(topic,payload);
    // Serial.printf("\nres : %d\n",res);
    if(!res){
      res = false;
      throw false;
    }
  }
  catch(...){
  Serial.printf("\nError (%d) >> Unable to publish message\n", res);
  }
  return res;
}



//***** Complete the util functions below ******
