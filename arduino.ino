#include <AFMotor.h>

//configuration
const unsigned long timeToEject = 700;
const uint8_t ejectSpeed = 100;
uint8_t currentSpeed =  0;
const uint8_t laneSpeed = 240;
const unsigned long timeToRun = 100;


//pins
#define s1 A5 //ejected = LOW
#define s2 A4 //retracted = HIGH
#define RESET 2


AF_DCMotor ejectMotor(1);
AF_DCMotor laneMotor(3);


//globals
bool ejecting = false;
bool retracting = false;
bool s = false;
bool stopped = false;

unsigned long started = 0;
unsigned long now = 0;

unsigned long lastSd = 0;
const unsigned long sdTime = 100;
String in;
unsigned long runTime = 0;
unsigned long nowTime= 0;


void stopSeq(){
    ejectMotor.setSpeed(60);
    delay(100);
    ejectMotor.run(RELEASE);
}

void eject(){
  if(!s){
    currentSpeed = ejectSpeed;
    ejectMotor.run(FORWARD);
    ejectMotor.setSpeed(ejectSpeed);
    started = millis();
    s = true;
  }
  now = millis();

  if(((now - started) > timeToEject) || digitalRead(s1) == LOW){
    stopSeq();
    ejectMotor.setSpeed(0);
    ejectMotor.run(RELEASE);

    //start retracting
    s = false;
    ejecting = false;
  }
}


void retract(){
  if(!s){
    currentSpeed = ejectSpeed;
    ejectMotor.run(BACKWARD);
    ejectMotor.setSpeed(ejectSpeed);
    started = millis();
    s = true;
  }
  now = millis();

  if(((now - started) > 2*timeToEject) || digitalRead(s2) == HIGH){
    stopSeq();
    ejectMotor.setSpeed(0);
    ejectMotor.run(RELEASE);

    //end sequence
    s = false;
    retracting = false;
  }
}

void setup() {
  //set input/output pins
  digitalWrite(RESET, HIGH);
  pinMode(RESET, OUTPUT);
  pinMode(s1, INPUT_PULLUP);
  pinMode(s2, INPUT_PULLUP);

  //initialize serial
  Serial.begin(9600);

  //wait for connection to serial
  while(!Serial){
    delay(100);
  }

  Serial.println("Configuration completed");
}

void loop() {
  //restart arduino if serial is disconnected
  if(!Serial){
    digitalWrite(RESET, LOW);
  }

  //check if serial has data waiting
  if(Serial.available() != 0) {
    in = Serial.readString();
    in.trim();

    if(in == "eject" && !ejecting && !retracting){
      if(digitalRead(s2) != HIGH){
        retract();   
      }
      delay(50);
      s=false;
      ejecting = true;
      retracting = true;
    }
    else if(in == "resume"){
      runTime = millis();
      stopped = false;
      laneMotor.setSpeed(laneSpeed);
      laneMotor.run(FORWARD);
    }
    else if(in == "reverse"){
      runTime = millis();
      stopped = false;
      laneMotor.setSpeed(laneSpeed);
      laneMotor.run(BACKWARD);
    }
    else if(in == "pause"){
      laneMotor.setSpeed(0);      
      laneMotor.run(RELEASE);
    }    
    else{
      Serial.println(in);
    }
  }

  //stopper
  nowTime = millis();
  if((nowTime - runTime) > timeToRun && !stopped){
    stopped = true;
    laneMotor.setSpeed(0);      
    laneMotor.run(RELEASE);
    Serial.println("stop");
  }

  //eject now
  if(ejecting){
    eject();
  }
  else if(retracting){
    retract();
  }

}
