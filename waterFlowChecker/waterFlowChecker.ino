//#define MY_DEBUG
#define MY_RADIO_NRF24
#define MY_REPEATER_FEATURE
#define MY_RF24_PA_LEVEL RF24_PA_HIGH
#define MY_RF24_CHANNEL  123
//#define MY_NODE_ID 103

#include <SPI.h>
#include <MySensors.h>


//************  water volumn  Related  *********************************************
int flowmeterPIN = 3;
float pulsePerSec = 7.5; // calibrate  unit L/min error 3% ??
#define CHILD_ID_WATERFLOW 1
MyMessage waterVolMsg(CHILD_ID_WATERFLOW, V_VOLUME);
int updateWaterVolumnInterval = 10 ; //sec
int commitWaterVolumnInterval = 300 ;
//************ Solenoid Valve Probe Related *********************************************
int solenoidValvePin = 4;
#define CHILD_ID_relay 2
MyMessage relayMsg(CHILD_ID_relay, V_STATUS);
int pumpEachTimeSec = 6;
int intervalRefill = 60 ; //sec

//************ ultrasonic  Related *********************************************
int triggerPIN = 5;
int echoPIN = 6;
#define CHILD_ID_WATERLEVEL 3
MyMessage waterlevelMsg(CHILD_ID_WATERLEVEL, V_DISTANCE);
int windowSensitive_1 = 1; // 1cm
int setPointWaterLvl = 20; //cm




//**********************  etc  **************
int ledStatusPin = 7 ;
int counter = 0;

volatile int  flow_frequency;  // Measures flow meter pulses
unsigned long tmpNow;
unsigned long accumulateWaterInMillilites = 0;
float previousFlowRate = 0.0;

long remainMillisecRelay = 0;
long lastTimeStampSecRelay = 0;

bool flagFinishValve = false;
int nowWaterLevel;
int previousWaterLevel = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(flowmeterPIN, INPUT);

  pinMode(solenoidValvePin, OUTPUT);
  digitalWrite(solenoidValvePin, HIGH);

  pinMode(triggerPIN, OUTPUT);
  pinMode(echoPIN, INPUT);

  pinMode(ledStatusPin, OUTPUT);
}


void presentation()
{
  // Send the sketch version information to the gateway and Controller
  randomSeed(analogRead(7));
  int tmpppp = random(0, 2000);
  wait(tmpppp);
  //Send the sensor node sketch version information to the gateway
  sendSketchInfo("WaterSupply", "1.09");

  wait(2000 - tmpppp);
  tmpppp = random(500, 2000);
  wait(tmpppp);
  present(CHILD_ID_WATERFLOW, S_WATER, "Water Volumn Meter");
  wait(2000 - tmpppp);

  tmpppp = random(500, 2000);
  wait(tmpppp);
  present(CHILD_ID_relay,  S_BINARY, "Solenoid Valve");
  wait(2000 - tmpppp);

  tmpppp = random(500, 2000);
  wait(tmpppp);
  present(CHILD_ID_WATERLEVEL,  S_DISTANCE, "Water level ");
  wait(2000 - tmpppp);
}

int readDistance() {
  digitalWrite(triggerPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPIN, LOW);
  long duration = pulseIn(echoPIN, HIGH);
  //  Serial.print(duration / 29.1 / 2);
  //  Serial.print(" cm");
  //  Serial.println();
  if (duration == 0) {
    return -1;
  }
  return duration / 29.1 / 2 ; //distanceCm
}


void loop()
{
  // ******************************** waterlevel ********************************
  if (counter % updateWaterVolumnInterval == 0)
  {
    nowWaterLevel = readDistance();
    if (  abs(nowWaterLevel - previousWaterLevel) >  windowSensitive_1 ) {
      previousWaterLevel = nowWaterLevel;
      send(waterlevelMsg.set(nowWaterLevel));
      wait(100);
    }
  }
  if (counter % commitWaterVolumnInterval == 0 )
  {
    previousWaterLevel = nowWaterLevel;
    send(waterlevelMsg.set(nowWaterLevel));
    wait(100);
  }


  // ******************************** turn on.off valve ********************************
  //check refill
  if (counter % intervalRefill == 0) {
    if (nowWaterLevel > setPointWaterLvl) {
      remainMillisecRelay = pumpEachTimeSec * 1000;
    }
  }
  if (remainMillisecRelay > 0) {
    remainMillisecRelay -=  (  millis() - lastTimeStampSecRelay )   ;
    digitalWrite(solenoidValvePin, HIGH);  // active high, hold low for disconnect, Normal Open
    remainMillisecRelay--;
    if (remainMillisecRelay < 0  ) {
      flagFinishValve = true;
    }
  } else {
    digitalWrite(solenoidValvePin, LOW); // open contact
    remainMillisecRelay = 0;
  }
  lastTimeStampSecRelay = millis();


  // ******************************** measure water volumn if valve ON ********************************
  flow_frequency = 0;
  if ( remainMillisecRelay > 0) {
    tmpNow = millis();
    while (tmpNow + 1000 > millis() ) {  // loop about 1 sec
      if ( pulseIn(flowmeterPIN, HIGH, 100000) ) { // timeout in microsec
        flow_frequency++;
      }
    }
  } else {
    wait(1000);
  }
  accumulateWaterInMillilites +=  (  (  (flow_frequency / 7.5 ) + (previousFlowRate)  ) * 16.67 / 2     ) ;
  previousFlowRate = flow_frequency / 7.5;
  // report total vol.  after valve off.
  if (flagFinishValve) {
    send(waterVolMsg.set(accumulateWaterInMillilites));
    flagFinishValve = false;
  }

  //******************************** debug section ********************************
  Serial.print("jobs remain  |  Valve ");
  Serial.print(remainMillisecRelay);
  Serial.print(" ms  | acc volumn   ");
  Serial.print(accumulateWaterInMillilites);
  Serial.print(" ml  | nowWaterLevel ");            // Print litres/min
  Serial.print(nowWaterLevel);
  Serial.println(" cm");
  //  Serial.print(flow_frequency, DEC);            // Print litres/hour
  //  Serial.print(" freq Hz , ");
  //  Serial.print(" ml  | water flow rate . ");
  //  Serial.print(flow_frequency / 7.5);            // Print litres/min
  //  Serial.println(" L/min");

  //******************************** reset if counter over 100 ********************************
  if (counter == 10000)
  {
    counter = 0;
  }

  //**********************************   blink led every 2 sec ********************************
  if ((counter % 1) == 0)
  {
    digitalWrite(ledStatusPin, digitalRead(ledStatusPin) ^ 1);
  }
  counter++;
}

void receive(const MyMessage &message)
{
  // command  of first doser
  if ( message.sensor == CHILD_ID_WATERFLOW) {
    Serial.print("Incoming change for sensor:");
    Serial.print(message.sensor);
    Serial.print(", value : ");
    Serial.println(message.getString());
    send(waterVolMsg.set(accumulateWaterInMillilites));
  }

  // command  of first doser
  if ( message.sensor == CHILD_ID_WATERLEVEL) {
    Serial.print("Incoming change for sensor:");
    Serial.print(message.sensor);
    Serial.print(", value : ");
    Serial.println(message.getString());
    send(waterlevelMsg.set(nowWaterLevel));
  }

  if ( message.sensor == CHILD_ID_relay) {
    Serial.print("Incoming change for sensor:");
    Serial.print(message.sensor);
    Serial.print(", value : ");
    Serial.println(message.getString());
    int turnOnSec = message.getInt();
    remainMillisecRelay = turnOnSec;
    remainMillisecRelay *= 1000;
  }

}


