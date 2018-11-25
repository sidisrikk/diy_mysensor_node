#define MY_DEBUG
#define MY_RADIO_NRF24
#define MY_RF24_CHANNEL 123
#define MY_REPEATER_FEATURE
#define MY_RF24_PA_LEVEL RF24_PA_HIGH
//#define MY_NODE_ID 101

#include <SPI.h>
#include <MySensors.h>


//************ Pressure Pump Probe Related *********************************************
int PressurePumpPin = 3;
#define CHILD_ID_PressurePump 1
MyMessage relayMsg(CHILD_ID_PressurePump, V_DISTANCE);




//****************************  etc ****************************
int LED_Status_Pin = 6;
int counter = 0;
long remainMillisecRelay = 0;
long lastTimeStampSecRelay = 0;

void setup()
{
  pinMode(LED_Status_Pin, OUTPUT);

  pinMode(PressurePumpPin, OUTPUT);
  digitalWrite(CHILD_ID_PressurePump, HIGH);
}

void presentation()  {

  // Send the sketch version information to the gateway and Controller
  randomSeed(analogRead(7));
  int tmpppp = random(0, 2000);
  wait(tmpppp);
  //Send the sensor node sketch version information to the gateway
  sendSketchInfo("12VDC", "1.09");

  wait(2000 - tmpppp);
  tmpppp = random(500, 2000);
  wait(tmpppp);
  present(CHILD_ID_PressurePump, S_BINARY, "Pressure Pump");
  wait(2000 - tmpppp);
}

void loop()
{
  wait(1000);

  if (remainMillisecRelay > 0) {
    remainMillisecRelay -=  (  millis() - lastTimeStampSecRelay )   ;
    digitalWrite(PressurePumpPin, LOW);  // active high, hold low for disconnect, Normal Open
  } else {
    digitalWrite(PressurePumpPin, HIGH ); // open contact
    remainMillisecRelay = 0;
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
    digitalWrite(LED_Status_Pin, digitalRead(LED_Status_Pin) ^ 1);
  }
  counter++;
}

void receive(const MyMessage &message) {
  if ( message.sensor == CHILD_ID_PressurePump) {
    Serial.print("Incoming change for sensor:");
    Serial.print(message.sensor);
    Serial.print(", value : ");
    Serial.println(message.getString());
    int turnOnSec = message.getInt();
    remainMillisecRelay = turnOnSec;
    remainMillisecRelay *= 1000;
  }
}

