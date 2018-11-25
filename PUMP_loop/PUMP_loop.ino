//////////////////////////////////////////////////////////////////////////////////////////
#define MY_DEBUG
#define MY_RADIO_NRF24
#define MY_RF24_CHANNEL 123
#define MY_REPEATER_FEATURE
#define MY_RF24_PA_LEVEL RF24_PA_HIGH
//#define MY_NODE_ID 100

#include <SPI.h>
#include <MySensors.h>


//************ Feedback Loop Pump Probe Related *********************************************
int relayPumpACPin = 4;
#define CHILD_ID_relay 1
MyMessage relayMsg(CHILD_ID_relay, V_STATUS);
unsigned long uptime  = 10000; // ms, dont * here
unsigned long downtime = 10000; // ms,

//****************************** etc ************************************
int LED_Status = 6;

int counter = 0;
unsigned long remainMillisecRelay = 0;
unsigned long lastTimeStampSecRelay = 0;
unsigned long stampDownTimeStart = 0;
bool toggleStampDownTime = false;

void presentation()  {

  // Send the sketch version information to the gateway and Controller
  randomSeed(analogRead(7));
  int tmpppp = random(0, 2000);
  wait(tmpppp);
  //Send the sensor node sketch version information to the gateway
  sendSketchInfo("AcLoopToggle", "1.09");

  wait(2000 - tmpppp);
  tmpppp = random(500, 2000);
  wait(tmpppp);
  present(CHILD_ID_relay, S_BINARY, "Ac Pump");
  wait(2000 - tmpppp);
}

void setup()
{
  pinMode(LED_Status, OUTPUT);

  pinMode(relayPumpACPin, OUTPUT);
  digitalWrite(relayPumpACPin, HIGH);

  stampDownTimeStart = millis();
}


void loop()
{
  wait(1000);

  if (remainMillisecRelay > 0) {
    if (remainMillisecRelay < 1000) {
      remainMillisecRelay = 0;
    } else {
      remainMillisecRelay -=  (  millis() - lastTimeStampSecRelay )   ;
    }

    digitalWrite(relayPumpACPin, LOW);  // active high, hold low for disconnect, Normal Open
    if (remainMillisecRelay == 0) {
      stampDownTimeStart = millis();
      send(relayMsg.set(1)); // off msg
      wait(100);
      send(relayMsg.set(0)); // off msg
    }
    toggleStampDownTime = true;
  } else {
    digitalWrite(relayPumpACPin, HIGH); // open contact
    remainMillisecRelay = 0;

    // if off for 10minutes then turn on
    if (  (millis() - stampDownTimeStart) > downtime ) {
      remainMillisecRelay = uptime;
      send(relayMsg.set(0)); // off msg
      wait(100);
      send(relayMsg.set(1)); // on msg
    }
    toggleStampDownTime = false;
  }
  lastTimeStampSecRelay = millis();



  //debug sec
  Serial.print("jobs remain  | pump ");
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
    digitalWrite(LED_Status, digitalRead(LED_Status) ^ 1);
  }
  counter++;
}

void receive(const MyMessage &message) {

}

