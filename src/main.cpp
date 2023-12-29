#include <Arduino.h>
#include <Wire.h>
#include <MobaTools.h>
#include <CircularBuffer.h>

//------------ BUFFER ------------------------------
CircularBuffer i2cBuffer;

//---- SERVOS-------------------------
#define TOPSERVOPIN 5
#define MEDIUMSERVOPIN 6
#define BOTTOMSERVOPIN 9
#define INTERRUPTPIN 8

MoToServo topServo;
MoToServo middleServo;
MoToServo bottomServo;

byte angleArray[12];
byte previousAngleArray[3] = {0,0,0}; //values of servo's angles when spider is turned off
byte angleToDo[3];
byte actualAngleArray[3];
const int maxServoDurationTime = 4000; //defined in advance
bool startSequence = false;
bool sequenceFinished = false;  //in order to respond to raspberry
uint8_t nbOfSequence = 0;
bool letNewSequence = false;
bool stopSequence = false;
bool sendActualAngles = false;
uint8_t sendCounter = 0;
unsigned long lastSemiSequence = 0;

void servoMotion(){

  if(stopSequence){
    stopSequence = false;
    letNewSequence = false;
    startSequence = false;
    topServo.write(topServo.read());
    middleServo.write(middleServo.read());
    bottomServo.write(bottomServo.read());
    actualAngleArray[0] = topServo.read();
    actualAngleArray[1] = middleServo.read();
    actualAngleArray[2] = bottomServo.read();
    for (int i=0; i<3; i++){
      previousAngleArray[i] = actualAngleArray[i] + 1;
    }
  }
  else if(startSequence && nbOfSequence < 4){ //initialize servo speed and write angles
    delay(10);
    startSequence = false;
    uint8_t j = 3*nbOfSequence;
    for(int i=0; i<3; i++){
      angleToDo[i] = abs(angleArray[j+i] - previousAngleArray[i]);
      previousAngleArray[i] = angleArray[j+i];
    }
    float timeOfSequence = maxServoDurationTime/180*max(max(angleToDo[0],angleToDo[1]),angleToDo[2]);//estimation of time taking into account max time duration
    float impulsionWith[3];
    float numberOfSegments = timeOfSequence/20; 
    for(int i=0; i<3; i++){
      impulsionWith[i] = 2*2000/180*angleToDo[i]/numberOfSegments; //every 20ms, impulsion will be extend of 0.5 * implusionWith us
    }
    // Serial.println(impulsionWith[0]);
    topServo.setSpeed(impulsionWith[0]);
    middleServo.setSpeed(impulsionWith[1]);
    bottomServo.setSpeed(impulsionWith[2]);
    topServo.write(angleArray[j+0]);
    middleServo.write(angleArray[j+1]);
    bottomServo.write(angleArray[j+2]);
    letNewSequence = true;
  }
  // if(millis()-lastSemiSequence > 10){
  else if(letNewSequence && topServo.moving()==0 && middleServo.moving()==0 && bottomServo.moving()==0){
    // delay(10);  //wait for hardware end
    startSequence = true;
    nbOfSequence++;
    letNewSequence = false;
    lastSemiSequence = millis();
    // Serial.println("semi");
  }
  else if(nbOfSequence == 4){
    sequenceFinished = true;
    digitalWrite(INTERRUPTPIN, 1); //set HIGH when sequence is finished
    startSequence = false;
    nbOfSequence++; //executed once
    // Serial.println();
    // Serial.println("complete");
  }
  // }
}

//----- I2C COMM------------------
#define ADRESS 0x09
int i = 0;
// bool readAngles = false;

void receiveEvent (int howMany){
  byte c;
  while(Wire.available()){
    c = Wire.read();
    i2cBuffer.writeData(c);
  }
}

void requestEvent (){
  if(sendActualAngles){
    Wire.write(actualAngleArray[sendCounter]);
    sendCounter++;
    if(sendCounter == 3)
      sendActualAngles = false;
  }
}

//------EXECUTION--------------------
void setup() {
  i2cBuffer.begin(100);
  Wire.begin(ADRESS);
  Serial.begin(115200);
  Serial.println("hi");
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

  topServo.attach(TOPSERVOPIN);
  middleServo.attach(MEDIUMSERVOPIN);
  bottomServo.attach(BOTTOMSERVOPIN);
  //set initial angles
  topServo.write(90);  //0
  middleServo.write(155); //
  bottomServo.write(2); //
  previousAngleArray[0] = 90;
  previousAngleArray[1] = 165;
  previousAngleArray[2] = 2;

  pinMode(INTERRUPTPIN, OUTPUT);
  digitalWrite(INTERRUPTPIN, 0);

}

void loop(){
  //-------- FILL ANGLE ARRAY --------------
  if(!i2cBuffer.isEmpty()) {
    byte data = i2cBuffer.readData();
    // delay(1);
    if(data > 0 && data<=181){ //&& readAngles){ //ajust received angles
      angleArray[i] = data-1;
      digitalWrite(INTERRUPTPIN, 0); //set back to 0V when new angles are arriving
      // Serial.println(data-1);
      i++;
    }
    else if(data == 200){ //stop code (unused value for angles)
      stopSequence = true;
    }
    else if(data == 0xCA) //send actual angles after a stop
      sendActualAngles = true;
    if (i == 12){
      i = 0;
      // Serial.println("all data");
      // Serial.println();      
      startSequence = true;
      nbOfSequence = 0;
      sequenceFinished = false;
    }
  }

  servoMotion();

}
