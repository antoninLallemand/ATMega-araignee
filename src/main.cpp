/*
"Une araignée" project UTBM autumn 2023
This code is used in each ATmega328p (each leg)
Author : Antonin Lallemand
*/

#include <Arduino.h>
#include <Wire.h>
#include <MobaTools.h>        //github : https://github.com/MicroBahner/MobaTools
#include <CircularBufferAL.h> //github : https://github.com/antoninLallemand/CircularBuffer-library (simplified version is already included)

/*------------ BUFFER ---------------*/

CircularBufferAL i2cBuffer; //create a CircularBuffer instance

/*------------ SERVOS --------------*/
#define TOP_SERVO_PIN 5
#define MEDIUM_SERVO_PIN 6
#define BOTTOM_SERVO_PIN 9
#define INTERRUPT_PIN 8

MoToServo topServo;
MoToServo middleServo;
MoToServo bottomServo;

byte angleArray[12];                    //received angles from Rasp (3*4)
byte previousAngleArray[3] = {0,0,0};   //values of servo's angles when spider is turned off
byte angleToDo[3];                      //gap between actual angle and command angle for each servo
byte actualAngleArray[3];               //angles when a stop is received
const int maxServoDurationTime = 4000;  //reference speed : duration for a 0-270° displacement
bool startSequence = false;             //when angleArray is updated
bool sequenceFinished = false;          //in order to respond to raspberry
uint8_t nbOfSequence = 0;               //increment each sub-sequence
bool letNewSequence = false;            //when a sub-sequence is finished
bool stopSequence = false;              //when a stop is received
bool sendActualAngles = false;          //when actualAngleArray is filled after a stop
uint8_t sendCounter = 0;                //stop angles cursor 

//Servo infinite loop function
void servoMotion(){

  if(stopSequence){
    stopSequence = false;
    letNewSequence = false;                             //stop running sequence
    startSequence = false;
    topServo.write(topServo.read());
    middleServo.write(middleServo.read());              //write reading value = stop movement
    bottomServo.write(bottomServo.read());
    actualAngleArray[0] = topServo.read();
    actualAngleArray[1] = middleServo.read();           //read every servos position and fill stop angles array
    actualAngleArray[2] = bottomServo.read();
    for (int i=0; i<3; i++)
      previousAngleArray[i] = actualAngleArray[i] + 1;  //fill last angle position for future movement
    sendActualAngles = true;                            //allow answer to raspberry
  }

  //initialize servo speed and write angles
  else if(startSequence && nbOfSequence < 4){ //for each sub-sequence
    // delay(10);
    startSequence = false;
    uint8_t j = 3*nbOfSequence;     //reading the 3 corrects angles in the array
    for(int i=0; i<3; i++){
      angleToDo[i] = abs(angleArray[j+i] - previousAngleArray[i]);  //calcul angle to do during the sub-sequence for 3 servos
      previousAngleArray[i] = angleArray[j+i];  //for next sub-sequence
    }
    float timeOfSequence = maxServoDurationTime/180*max(max(angleToDo[0],angleToDo[1]),angleToDo[2]); //estimation of time using biggest movement
    float impulsionWith[3];
    float numberOfSegments = timeOfSequence/20; //number of command update during the movement
    for(int i=0; i<3; i++){
      impulsionWith[i] = 2*2000/180*angleToDo[i]/numberOfSegments; //every 20ms, impulsion will be extend of 0.5 * implusionWith (us)
    }
    topServo.setSpeed(impulsionWith[0]);
    middleServo.setSpeed(impulsionWith[1]);   //set speed of 3 servos in order to finish at the same time
    bottomServo.setSpeed(impulsionWith[2]);
    topServo.write(angleArray[j+0]);
    middleServo.write(angleArray[j+1]);       //command servos
    bottomServo.write(angleArray[j+2]);
    letNewSequence = true;
  }

  //when sub-sequence is finished
  else if(letNewSequence && topServo.moving()==0 && middleServo.moving()==0 && bottomServo.moving()==0){
    startSequence = true;
    nbOfSequence++;         //next sub-sequence
    letNewSequence = false;
  }
  
  //when sequence (4 sub-sequences) is finished
  else if(nbOfSequence == 4){
    sequenceFinished = true;
    digitalWrite(INTERRUPT_PIN, 1); //set HIGH when sequence is finished
    startSequence = false;
    nbOfSequence++; //permit to execute this function only once
  }

}

//----- I2C COMM------------------
#define ADRESS 0x09   //slave adress
int i = 0;

//when Raspberry send data to slave
void receiveEvent (int howMany){
  byte c;
  while(Wire.available()){
    c = Wire.read();          //read I2C data
    i2cBuffer.writeData(c);   //write reading data in the buffer
  }
}

//when Raspberry send a request to slave
void requestEvent (){
  if(sendActualAngles){ //send stop angles
    Wire.write(actualAngleArray[sendCounter]);
    sendCounter++;
    if(sendCounter == 3)
      sendActualAngles = false;
  }
}

//------EXECUTION--------------------
void setup() {
  //set I2C
  Wire.begin(ADRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

  //set servos pins
  topServo.attach(TOP_SERVO_PIN);
  middleServo.attach(MEDIUM_SERVO_PIN);
  bottomServo.attach(BOTTOM_SERVO_PIN);

  //set initial angles
  topServo.write(90);  //0
  middleServo.write(155); //165
  bottomServo.write(2); //
  previousAngleArray[0] = 90;
  previousAngleArray[1] = 155;
  previousAngleArray[2] = 2;

  //set interrupt pin
  pinMode(INTERRUPT_PIN, OUTPUT);
  digitalWrite(INTERRUPT_PIN, 0);

  Serial.begin(115200);
  Serial.println("hi");
}

void loop(){
  //-------- INCOMMING DATA INTERPRETATION --------------
  if(!i2cBuffer.isEmpty()) {
    byte data = i2cBuffer.readData(); //read buffer
    if(data > 0 && data<=181){  //if received data is an angle
      angleArray[i] = data-1;
      digitalWrite(INTERRUPT_PIN, 0); //set back to 0V when new angles are arriving
      i++;
    }
    else if(data == 0xC8) //stop code (unused value for angles)
      stopSequence = true;
    
    else if(data == 0xCA) //send actual angles after a stop
      sendActualAngles = true;

    //when every angles are received, start sequence
    if (i == 12){
      i = 0;    
      startSequence = true;
      nbOfSequence = 0;
      sequenceFinished = false;
    }
  }

  servoMotion();

}
