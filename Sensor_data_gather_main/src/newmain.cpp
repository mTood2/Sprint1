#include <Arduino.h>
#include <ArduinoJson.h>         // Library to Create Json objects
#include <OneWire.h>            // Libraries for the Temp sensor DS18B20
#include <DallasTemperature.h>  
#include <Ultrasonic.h>        //Library for the Distance sensor HC-SR04
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
////////////.....My Constants.....////////////
String DS18B20topic = "t";
String DeviceID = "M";
String SensID = "2";
String SensType ="t";
String timestamp ="0000";

////////////.....Initialise my Wifi.....////////////
const char* ssid = "Matts Hotspot";
const char* password = "hotspot1";
const char* mqtt_server = "149.28.177.218";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char T_SensReading[3];
char D_SensReading[3];
int value = 0;

////////////.....Initialise the Temperature sensor.....////////////
#define ONE_WIRE_BUS D5         //Data bus for DS18B20 is on pin 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float Celsius=0.0;              //variables for Temp sensor

////////////.....Initialise the Distance Sensor.....////////////
#define Echo D6
#define Trigger D7
Ultrasonic ultrasonic(Trigger, Echo);  //(Trigger, Echo)
int distance;                 //variable for distance sensor 

////////////.....My functions.....////////////
void ReadTemp() {
  sensors.requestTemperatures(); 
  Celsius=sensors.getTempCByIndex(0);
  Celsius = round(Celsius*10.0)/10.0;
  // Serial.print("Temperature");
  // Serial.println(Celsius, 1);
}
void ReadDist() {
   distance = ultrasonic.read();
  // Serial.print("    Distance in CM: ");
  // Serial.println(distance);
}
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
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

////////////.....Arduino functions.....//////////// 

void setup()
{
  Serial.begin(9600);
  sensors.begin();
   setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}
void loop()
{ 
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  ReadTemp();  //Get values
  ReadDist();  //

  long now = millis();
if (now - lastMsg > 2000) {
  lastMsg = now;

  dtostrf(Celsius, 3,1 , T_SensReading);
  char T_prefix[] = "Mc00001A";  // xyaaaabcddd //DevID/Topic/Time/SensID/SensType
  char T_PAYLOAD[11];
  strcpy(T_PAYLOAD, T_prefix);
  concatenate(T_PAYLOAD, T_SensReading);
  Serial.println(T_PAYLOAD);
  client.publish("c", T_PAYLOAD);

  dtostrf(distance, 2,0, D_SensReading);
  char D_prefix[] = "Md00002c0";
  char D_PAYLOAD[11];
  strcpy(D_PAYLOAD, D_prefix);
  concatenate(D_PAYLOAD, D_SensReading);
  Serial.println(D_PAYLOAD);
  client.publish("d", D_PAYLOAD);
}
  delay(5000);
}
