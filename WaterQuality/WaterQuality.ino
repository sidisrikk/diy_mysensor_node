#define MY_DEBUG
#define MY_RADIO_NRF24
#define MY_REPEATER_FEATURE
#define MY_RF24_PA_LEVEL RF24_PA_HIGH
#define MY_RF24_CHANNEL  124
//#define MY_NODE_ID 34

#include <SPI.h>
#include <MySensors.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#define EMA_ALPHA 10
#include <microsmooth.h>


//************ EC Probe Related *********************************************
int ECPower = A1;
int ECPin = A2;
int R1 = 470; //(in Ohm) R1 must greater than 300ohms
int Ra = 25; //(in Ohm) Resistance of powering Pins in arduino
float TemperatureCoef = 0.019; // generaly  the standard for plant nutrients solution
float K = 3.3; // Estimate constant. upon plug's shape
#define disconnectEC_pin1 5
#define disconnectEC_pin2 6
#define temp1Pin 4
#define CHILD_ID_EC 1
MyMessage ECMsg(CHILD_ID_EC, V_EC);
unsigned long intervalUpdate_1 = 600; // in second
unsigned long intervalCommit_1 = 3000; //
float windowSensitive_1 = 0.1; // +- this number

//**************** ph related
const int pHPin = A3;
float calibrateBuffferPH1 = 3.96;
float calibrateBuffferPH2 = 6.64;
float senseValue1 = 404;
float senseValue2 = 501; // from ph6.68
#define CHILD_ID_PH 2
MyMessage pHMsg(CHILD_ID_PH, V_PH);
unsigned long intervalUpdate_2 = 600; // in second
unsigned long intervalCommit_2 = 3000; //
float windowSensitive_2 = 0.2; // +- this number

//**************** diaphragm pump
const int ACpumpPin = 3;
#define CHILD_ID_DIAPHRAGM 3
MyMessage diaphragmPumpMsg(CHILD_ID_DIAPHRAGM, V_DISTANCE);
long preparePumpInSec = 20;


// | | 5.2, 4.9 |   5.8 , 6.0   |   7.0,7.2
// ****************   etc
float last_ec = 0.0;
float last_ph = 0.0;
float nowEC = 0.0;
float nowPh = 0.0;
int counter = 1;
OneWire oneWire1(temp1Pin);
DallasTemperature waterTemp1Sensors(&oneWire1);

long remainMillisecRelay = 0;
long lastTimeStampSecRelay = 0;

void setup() {
  //Serial.begin(115200);
  waterTemp1Sensors.begin();
  wait(200);
  pinMode(8, OUTPUT); //led pin

  pinMode(ECPin, INPUT);
  pinMode(ECPower, OUTPUT);
  pinMode(disconnectEC_pin1, OUTPUT);
  pinMode(disconnectEC_pin2, OUTPUT);
  disconnectECPin();

  pinMode(pHPin, INPUT);

  pinMode(ACpumpPin, OUTPUT);
  digitalWrite(ACpumpPin, LOW); // active low

  R1 = (R1 + Ra); // Taking into acount Powering Pin Resitance
}




void presentation()
{
  // Send the sketch version information to the gateway and Controller
  randomSeed(analogRead(7));
  int tmpppp = random(0, 2000);
  wait(tmpppp);
  //Send the sensor node sketch version information to the gateway
  sendSketchInfo("WaterQuality", "1.09");

  tmpppp = random(500, 2000);
  wait(tmpppp);
  present(CHILD_ID_EC,  S_WATER_QUALITY, "EC Sensor");
  wait(2000 - tmpppp);

  tmpppp = random(500, 2000);
  wait(tmpppp);
  present(CHILD_ID_PH, S_WATER_QUALITY, "PH Sensor");
  wait(2000 - tmpppp);

  tmpppp = random(500, 2000);
  wait(tmpppp);
  present(CHILD_ID_DIAPHRAGM, S_WATER_QUALITY, "Diaphragm Pump");
  wait(2000 - tmpppp);
}


void loop()
{
  wait(1000);




  // ********************************  EC section ********************************

  // pre load pump
  if (  ((counter + preparePumpInSec) % intervalUpdate_1) == 0) {
    if (remainMillisecRelay < preparePumpInSec * 1000) {
      remainMillisecRelay = preparePumpInSec * 1000;
    }
  }

  if (counter % intervalUpdate_1 == 0 )
  {
    waterTemp1Sensors.requestTemperatures();
    wait(1000);
    nowEC  = getEC(waterTemp1Sensors.getTempCByIndex(0));
    if (  abs(nowEC - last_ec) > windowSensitive_1 )
    {
      last_ec = nowEC;
      send(ECMsg.set(nowEC, 1));
      wait(100);
    }
  }
  if (counter % intervalCommit_1 == 0) //600
  {
    last_ec = nowEC;
    send(ECMsg.set(nowEC, 1));
    wait(100);
  }


  //*************************************** ph section ********************************
  if (counter % intervalUpdate_2 == 0 )
  {
    nowPh = getpH();
    //check before update
    if ( abs(nowPh - last_ph) > windowSensitive_2)  {
      last_ph = nowPh;
      send(pHMsg.set(last_ph, 1));
      wait(100);
    }
  }
  if (counter % intervalCommit_2 == 0) //750
  {
    last_ph = nowPh;
    send(pHMsg.set(last_ph, 1));
    wait(100);
  }

  // ****************************  pump section *******************************
  if (remainMillisecRelay > 0) {
    remainMillisecRelay -=  (  millis() - lastTimeStampSecRelay );
    digitalWrite(ACpumpPin, HIGH);  // active low
  } else {
    digitalWrite(ACpumpPin, LOW); // open contact
    remainMillisecRelay = 0;
  }
  lastTimeStampSecRelay = millis();

  //debug sec
  Serial.print("jobs remain  | AcPump ");
  Serial.print(remainMillisecRelay);
  Serial.println(" ms");

  //******************************** reset if counter over 100 ********************************
  if (counter > 10000) {
    counter = 1;
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
  if ( message.sensor == CHILD_ID_EC) {
    Serial.print("Incoming change for sensor:");
    Serial.print(message.sensor);
    Serial.print(", value : ");
    Serial.println(message.getString());
    waterTemp1Sensors.requestTemperatures();
    wait(1000);
    float tempp =  getEC(waterTemp1Sensors.getTempCByIndex(0));
    send(ECMsg.set(tempp, 1));
  }

  if ( message.sensor == CHILD_ID_PH) {
    Serial.print("Incoming change for sensor:");
    Serial.print(message.sensor);
    Serial.print(", value : ");
    Serial.println(message.getString());
    float tempp = getpH();
    send(pHMsg.set(tempp, 1));
  }

  if ( message.sensor == CHILD_ID_DIAPHRAGM) {
    Serial.print("Incoming change for sensor:");
    Serial.print(message.sensor);
    Serial.print(", value : ");
    Serial.println(message.getString());
    int turnOnSec = message.getInt();
    remainMillisecRelay = turnOnSec;
    remainMillisecRelay *= 1000;
  }
}


float getEC(float Temperature) {
  uint16_t *ptrEC = ms_init(SMA); // use software filter  for better reading EC
  float EC = 0;
  float EC25 = 0;
  float EC25FN = 0;
  float raw = 0;

  float Vin = 5;
  float Vdrop = 0;
  float Rc = 0;


  //************Estimates Resistance of Liquid ****************//
  connectECPin();
  for (int x = 0; x < 20; x++) {
    digitalWrite(ECPower, HIGH);
    raw = analogRead(ECPin);
    raw = analogRead(ECPin);
    digitalWrite(ECPower, LOW);
    raw = (float)sma_filter((int)raw, ptrEC);
    wait(44);
  }
  ms_deinit(ptrEC);
  disconnectECPin();
  //***************** Converts to EC **************************//
  Vdrop = (Vin * raw) / 1024.0;
  Rc = (Vdrop * R1) / (Vin - Vdrop); //***** why - vin-vdrop
  Rc = Rc - Ra; //acounting for Digital Pin Resitance
  EC = 1000 / (Rc * K); // milli siemens


  //*************expo graph math calibrate********************//
  EC25 = EC - Math_calib(EC);


  //*************Compensating For Temperaure********************//

  EC25FN  =  EC25 / (1 + TemperatureCoef * (Temperature - 25.0));

  Serial.print(" E.C. : ");
  Serial.print(EC25FN);
  Serial.print(" u/S  Temp : ");
  Serial.print(Temperature);
  Serial.println(" *C ");


  //********** Usued for Debugging ************
  // ec in micro/semens   u/S
  /*
    Serial.println();
    Serial.print("| Vdrop: ");
    Serial.print(Vdrop);
    Serial.print(" | Rc: ");
    Serial.print(Rc);
    Serial.print(" | RAW EC: ");
    Serial.print(EC);
    Serial.print(" | math calib EC: ");
    Serial.print(EC25);
    Serial.print(" | ATC+calib EC: ");
    Serial.print(EC25FN);
    Serial.println("|");

    //********** end of Debugging Prints *********
  */
  return EC25FN;

}
void disconnectECPin() {
  digitalWrite(disconnectEC_pin1, HIGH);
  digitalWrite(disconnectEC_pin2, HIGH);
  wait(1000);
}
void connectECPin() {
  digitalWrite(disconnectEC_pin1, LOW);
  digitalWrite(disconnectEC_pin2, LOW);
  wait(1000);
}
float  Math_calib(float input) {
  return (0.004116 * pow(input, 3) + 0.011 * pow(input, 2) + 0.036 * pow(input, 1) - 0.044);
}

// the loop routine runs over and over again forever:
float getpH() {
  uint16_t *ptrHistoryPH = ms_init(EMA);
  analogRead(pHPin);
  // read the input on analog pin 0:
  int sensorValue = 0;

  //************Estimates pH of Liquid ****************//
  for (int x = 0; x < 20; x++) {
    int tmp = 1023 - analogRead(pHPin);
    //Serial.println(tmp);
    sensorValue = (float)ema_filter((int)tmp, ptrHistoryPH);
    wait(44);
  }
  //sensorValue /= 5;
  Serial.print(" Avg  raw ADC ");
  Serial.print(sensorValue);

  //find estimatePH no ATC


  // slope adc per pH
  float slope =    (senseValue2 - senseValue1) / (calibrateBuffferPH2 - calibrateBuffferPH1);
  // find c
  float constantinEqua = senseValue2 - (calibrateBuffferPH2 * slope);

  //final pH estimation /w no ATC
  float estimatePH = (sensorValue - constantinEqua) / slope;

  Serial.print(" pH  : ");
  Serial.println(estimatePH);
  ms_deinit(ptrHistoryPH);
  return estimatePH;
}

