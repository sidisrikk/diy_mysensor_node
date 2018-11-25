#define MY_DEBUG
#define MY_RADIO_NRF24
#define MY_REPEATER_FEATURE
#define MY_RF24_PA_LEVEL RF24_PA_HIGH
#define MY_RF24_CHANNEL  124
//#define MY_NODE_ID 57

#include <SPI.h>
#include <MySensors.h>

// 1   24 , 0.85   | 24 1.0
// 2  36 , 1.0 | 30 0.85
//************  dose 1 Probe Related  *********************************************
int pwmPIN_Motor1 = 5;
int secToReach20ml_Motor1 = 36; // calibrate pump1
float dutyCycle_Motor1 = 1.0; // calib too
#define CHILD_ID_dose1 1
MyMessage doseMsg1(CHILD_ID_dose1, V_VOLUME);
//************ dose 2 Probe Related *********************************************
int pwmPIN_Motor2 = 6;
int secToReach20ml_Motor2 = 30; // calibrate pump1
float dutyCycle_Motor2 = 0.85; // calib too
#define CHILD_ID_dose2 2
MyMessage doseMsg2(CHILD_ID_dose2, V_VOLUME);
//************ ACpump Probe Related *********************************************
int ACpumpPin = 3;
#define CHILD_ID_relay 3
MyMessage relayMsg(CHILD_ID_relay, V_LEVEL  );



//**********************  etc  **************
int ledStatusPin = 8;

int counter = 0;

long remainMillisec_Pump_1 = 0;
long lastTimeStampSec_Pump_1 = 0;

long remainMillisec_Pump_2 = 0;
long lastTimeStampSec_Pump_2 = 0;

long remainMillisecRelay = 0;
long lastTimeStampSecRelay = 0;

void setup() {
  pinMode(ledStatusPin, OUTPUT);

  pinMode(pwmPIN_Motor1, OUTPUT);
  pinMode(pwmPIN_Motor2, OUTPUT);
  digitalWrite(pwmPIN_Motor1, LOW);
  digitalWrite(pwmPIN_Motor2, LOW);

  pinMode(ACpumpPin, OUTPUT);
}


void presentation()
{
  // Send the sketch version information to the gateway and Controller
  randomSeed(analogRead(7));
  int tmpppp = random(0, 2000);
  wait(tmpppp);
  //Send the sensor node sketch version information to the gateway
  sendSketchInfo("SolutionDoser", "1.09");

  wait(2000 - tmpppp);
  tmpppp = random(500, 2000);
  wait(tmpppp);
  present(CHILD_ID_dose1, S_WATER, "Dosing Pump");
  wait(2000 - tmpppp);

  tmpppp = random(500, 2000);
  wait(tmpppp);
  present(CHILD_ID_dose2, S_WATER, "Dosing Pump");
  wait(2000 - tmpppp);

  tmpppp = random(500, 2000);
  wait(tmpppp);
  present(CHILD_ID_relay,  S_WATER , "Ac Pump");
  wait(2000 - tmpppp);
}



void loop()
{
  wait(1000);

  //do jobs
  if (remainMillisec_Pump_1 > 0) {
    remainMillisec_Pump_1 -=  (  millis() - lastTimeStampSec_Pump_1 )   ;
    analogWrite(pwmPIN_Motor1, dutyCycle_Motor1 * 255);
  } else {
    analogWrite(pwmPIN_Motor1, 0);
    remainMillisec_Pump_1 = 0;
  }
  lastTimeStampSec_Pump_1 = millis();


  if (remainMillisec_Pump_2 > 0) {
    remainMillisec_Pump_2 -=  (  millis() - lastTimeStampSec_Pump_2 )   ;
    analogWrite(pwmPIN_Motor2, dutyCycle_Motor2 * 255);
  } else {
    analogWrite(pwmPIN_Motor2, 0);
    remainMillisec_Pump_2 = 0;
  }
  lastTimeStampSec_Pump_2 = millis();

  if (remainMillisecRelay > 0) {
    remainMillisecRelay -=  (  millis() - lastTimeStampSecRelay )   ;
    digitalWrite(ACpumpPin, HIGH);  // active high, hold low for disconnect, Normal Open
  } else {
    digitalWrite(ACpumpPin, LOW); // open contact
    remainMillisecRelay = 0;
  }
  lastTimeStampSecRelay = millis();


  //debug sec
  Serial.print("jobs remain  | dosing Pump-1 ");
  Serial.print(remainMillisec_Pump_1);
  Serial.print(" ms  |  dosing pump-2 ");
  Serial.print(remainMillisec_Pump_2);
  Serial.print(" ms  |  AcPump ");
  Serial.print(remainMillisecRelay);
  Serial.println(" ms ");
  //******************************** reset if counter over 100 ********************************
  if (counter == 10000)
  {
    counter = 0;
  }

  //**********************************   blink led every 2 sec ********************************
  if ((counter % 1) == 0)
  {
    digitalWrite(8, digitalRead(8) ^ 1);
  }

  counter++;
}

void receive(const MyMessage &message)
{
  // command  of first doser
  if ( message.sensor == CHILD_ID_dose1) {
    Serial.print("Incoming change for sensor:");
    Serial.print(message.sensor);
    Serial.print(", value : ");
    Serial.println(message.getString());
    int doesVolumn = message.getInt();
    remainMillisec_Pump_1 = 1000 * (double)doesVolumn  * (secToReach20ml_Motor1 / 20.0);
    //remainMillisec_Pump_1 *= 1000;
  }

  if ( message.sensor == CHILD_ID_dose2) {
    Serial.print("Incoming change for sensor:");
    Serial.print(message.sensor);
    Serial.print(", value : ");
    Serial.println(message.getString());
    int doesVolumn = message.getInt();
    remainMillisec_Pump_2 = 1000 * (double)doesVolumn  * (secToReach20ml_Motor2 / 20.0);
    //remainMillisec_Pump_2 *= 1000;
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



