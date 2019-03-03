///////////////////////////////////////////////////////////////
/// Title:  Stagnum                                         ///
/// Author: Matthew Todd                                    ///
/// Date:   22/02/2019                                      ///
/// Description: This code will gather data from my ph,     ///
///               distance and temperature sensors and send ///
///               it to the MQTT Broker.                    ///
///////////////////////////////////////////////////////////////


#include <Arduino.h>
#include <OneWire.h>            // Libraries for the Temp sensor DS18B20
#include <DallasTemperature.h>  
#include <Ultrasonic.h>        //Library for the Distance sensor HC-SR04
#include "DFRobot_PH.h"        //Libaries for the pH sensor
#include <EEPROM.h>
#include <ESP8266WiFi.h>       //Libraries for Communication
#include <PubSubClient.h>

#include "Gsender.h"

////////////.....Initialise my Wifi.....////////////
const char* ssid = "Matts Hotspot";
const char* password = "hotspot1";
const char* mqtt_server = "149.28.177.218";          // Leonidas  149.28.180.60  
//                                             or // turtle 
//                                            or // gravity 45.76.117.90 

WiFiClient espClient;
PubSubClient client(espClient);

////////////.....Payload variables.....////////////

char T_prefix[] = "1cM00001A";char T_suffix[3]; char* temp_topic = "c";
char D_prefix[] = "1Md00002c0";char D_suffix[2]; char* dist_topic = "d";
char ph_prefix[] ="1Mp00003p0";char ph_suffix[2]; char* ph_topic = "p";

long lastMsg = 0;
long lastsend = 0;
////////////.....Email Service variables.....////////////

Gsender *gsender = Gsender::Instance(); 
String subject = "Pool Monitor Alert!";

////////////.....Initialise the Temperature sensor.....////////////
#define ONE_WIRE_BUS D5         //Data bus for DS18B20 is on pin 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float Celsius=0;              //variables for Temp sensor
float Fahrenheit=0;

////////////.....Initialise the Distance Sensor.....////////////
#define Echo D7
#define Trigger D6
Ultrasonic ultrasonic(Trigger, Echo);  //(Trigger, Echo)
int distance;                 //variable for distance sensor 

////////////.....Initialise the pH Sensor.....////////////
#define PH_PIN A0
float voltage,phValue,temperature = 25;
DFRobot_PH ph;

////////////.....Sensor functions.....////////////
float readTemp() {
  sensors.requestTemperatures(); 
  Celsius=sensors.getTempCByIndex(0);
  Fahrenheit=sensors.toFahrenheit(Celsius);
  return Celsius;
}
float readDist() {
   distance = ultrasonic.read();
  return distance;
}
float readpH() {
    int rawAnalog;
    rawAnalog = analogRead(PH_PIN);
    voltage = rawAnalog/1024.0*3100; // read the voltage
    phValue = ph.readPH(voltage,temperature); // convert voltage to pH with temperature compensation
    return phValue;
}
////////////.....Wifi functions.....//////////// 
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  long connect_attempt_time  = 0;
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - connect_attempt_time > 30000) {
    Serial.println("Wifi connection timeout. Going to sleep for 5 minutes");
    ESP.deepSleep(6e7); // 3e8 (us) = 5 mins  
    }
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
}
void callback(char* topic, byte* payload, int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
////////////.....Building the Payload.....////////////

void concatenate(char p[], char q[]) {
   int c, d;
   c = 0;
   while (p[c] != '\0') {
      c++;      
   }
   d = 0;
   while (q[d] != '\0') {
      p[c] = q[d];
      d++;
      c++;    
   }
   p[c] = '\0';
}
  void Assemble(char* pre,char* suff,int dig,int dec,float my_data, char* topic) {
      dtostrf (my_data, dig, dec, suff);
      char Payload[12];
      strcpy(Payload, pre);
      concatenate(Payload, suff);
      Serial.println(Payload);
      client.publish(topic, Payload);
  }
void sendAlert(int errType, float reading) {

    if (errType == 1) {
      if(gsender->Subject(subject)->Send("mt.todd565@gmail.com", "Your pool's water level is dangerously low! top up now.")) {
      Serial.println("Message sent.");
      }
    }
    else if (errType ==2) {
      if(gsender->Subject(subject)->Send("mt.todd565@gmail.com", "Your pool's pH level is dangerously low! ")) {
      Serial.println("Message sent.");
      }
    }
    else if (errType ==3) {
      if(gsender->Subject(subject)->Send("mt.todd565@gmail.com", "Your pool's pH level is dangerously high! ")) {
      Serial.println("Message sent.");
      }
    }
}



////////////.....Arduino functions.....////////////
void setup() {
  Serial.begin(9600);
  sensors.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop(){
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  float pH_Read = readpH();
  float temp_Read = readTemp();
  float dist_Read = readDist();

   
  if (millis()- lastMsg > 2000) {
  lastMsg = millis(); 

    Assemble (T_prefix, T_suffix, 2, 1, temp_Read, temp_topic);
    Assemble (D_prefix, D_suffix, 2, 0, dist_Read, dist_topic);
    Assemble (ph_prefix, ph_suffix, 1, 1, pH_Read, ph_topic);
  } 
  
  if (millis() -lastsend > 10000) {            //need to move sending emails into this end case otherwise I will send like 20 emails while my guys awake.
  long lastsend = millis();
      //check for urgent problems with the pool data
  if (dist_Read > 30) {
    sendAlert(1, dist_Read);
    } 
  if (pH_Read < 7) {
    sendAlert(2, pH_Read);
  }
  if (pH_Read > 8) {
    sendAlert(3, pH_Read);
  }
  Serial.println("Going to sleep for 5 minutes");
  ESP.deepSleep(300e6); // 3e8 (us) = 5 mins
  }
}