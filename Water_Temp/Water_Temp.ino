//#define MY_DEBUG
#define MY_RADIO_NRF24
#define MY_RF24_CHANNEL 124
#define MY_REPEATER_FEATURE
#define MY_RF24_PA_LEVEL RF24_PA_HIGH
//#define MY_NODE_ID 109

#include <SPI.h>
#include <MySensors.h>
#include <OneWire.h> //#include<Wire.h>
#include <DallasTemperature.h>



//************  temp 1 Probe Related  *********************************************
#define ONE_WIRE_BUS1 3
OneWire oneWire1(ONE_WIRE_BUS1);
DallasTemperature sensors1(&oneWire1);
#define ID_S_TEMP1 1
MyMessage msg_S_TEMP1(ID_S_TEMP1, V_TEMP);
unsigned long intervalUpdate_1 = 20; // in second
unsigned long intervalCommit_1 = 60; //
float windowSensitive_1 = 0.5; // +- this number

//************  temp 1 Probe Related  *********************************************
#define ONE_WIRE_BUS2 4
OneWire oneWire2(ONE_WIRE_BUS2);
DallasTemperature sensors2(&oneWire2);
#define ID_S_TEMP2 2
MyMessage msg_S_TEMP2(ID_S_TEMP2, V_TEMP);
unsigned long intervalUpdate_2 = 20; // in second
unsigned long intervalCommit_2 = 60; //
float windowSensitive_2 = 0.5;

//************  etc  *********************************************
int LED_Status = 8;

float last_t1 = 0;
float now_t1 = 0;
float last_t2 = 0;
float now_t2 = 0;
unsigned long counter = 0;


void presentation()  {

  sendSketchInfo("WaterTeamperature", "1.09");
  wait(200);

  present(ID_S_TEMP1, S_TEMP, "Temperarue Sensor");
  wait(200);
  present(ID_S_TEMP2, S_TEMP, "Temperarue Sensor");
  wait(200);
}

void setup()
{
  pinMode(LED_Status, OUTPUT);

  sensors1.begin();
  wait(200);
  sensors2.begin();
  wait(200);
}

void loop()
{
  wait(1000);
  // ************************  temp sensor 1  section*******************
  if (counter % intervalUpdate_1 == 0) {
    sensors1.requestTemperatures();
    wait(1000);
    now_t1 = sensors1.getTempCByIndex(0);
    Serial.print(" temp1 ");
    Serial.println(now_t1);
    if (  abs(now_t1 - last_t1) >  windowSensitive_1 ) {
      last_t1 = now_t1;
      send(msg_S_TEMP1.set(last_t1, 1));
      wait(100);
    }
  }
  if (counter % intervalCommit_1 == 0 ) //750
  {
    last_t1 = now_t1;
    send(msg_S_TEMP1.set(last_t1, 1));
    wait(100);
  }


  // ****************************  temp sensor 2  section *********************
  if (counter % intervalUpdate_2 == 0) {
    sensors2.requestTemperatures();
    wait(1000);
    now_t2 = sensors2.getTempCByIndex(0);
    Serial.print(" temp2 ");
    Serial.println(now_t2);
    if (  abs(now_t2 - last_t2) >  windowSensitive_2 ) {
      last_t2 = now_t2;
      send(msg_S_TEMP2.set(last_t2, 1));
      wait(100);
    }
  }
  if (counter % intervalCommit_2 == 0 ) //750
  {
    last_t2 = now_t2;
    send(msg_S_TEMP2.set(last_t2, 1));
    wait(100);
  }

  //************************************  blink LED ************************************
  if ((counter % 1) == 0)
  {
    digitalWrite(LED_Status, digitalRead(LED_Status) ^ 1);
  }
  if (counter == 100000 ) {
    counter = 0;
  }
  counter++;
}
