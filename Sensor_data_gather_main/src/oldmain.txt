#include <Arduino.h>
#include <OneWire.h>            // Libraries for the Temp sensor DS18B20
#include <DallasTemperature.h>  
#include <Ultrasonic.h>        //Library for the Distance sensor HC-SR04

////////////.....Initialise the Temperature sensor.....////////////
#define ONE_WIRE_BUS D5         //Data bus for DS18B20 is on pin 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float Celsius=0;              //variables for Temp sensor
float Fahrenheit=0;

////////////.....Initialise the Distance Sensor.....////////////
#define Echo D6
#define Trigger D7
Ultrasonic ultrasonic(Trigger, Echo);  //(Trigger, Echo)
int distance;                 //variable for distance sensor 

////////////.....My functions.....////////////
void ReadTemp() {
  sensors.requestTemperatures(); 
  Celsius=sensors.getTempCByIndex(0);
  Fahrenheit=sensors.toFahrenheit(Celsius);
  Serial.print("Temperature ");
  Serial.print(Celsius);
  Serial.print(" degrees C   ");
}
void ReadDist() {
   distance = ultrasonic.read();
  Serial.print("  Distance in CM: ");
  Serial.println(distance);
}
////////////.....Arduino functions.....////////////
void setup()
{
  Serial.begin(9600);
  sensors.begin();
}
void loop()
{ 
  ReadTemp();
  ReadDist();
  delay(2000);
}