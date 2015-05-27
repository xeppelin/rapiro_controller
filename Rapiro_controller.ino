#include <Servo.h>

#define NO_OF_EYE_LEDS 3
#define R 0          // Red LED
#define G 1          // Green LED
#define B 2          // Blue LED
#define POWER 17
#define NO_OF_SERVOS 12

//Servos
Servo servos[NO_OF_SERVOS];
int servoOffset[NO_OF_SERVOS] ={
  -1    //Head
  ,-6   //Waist
  ,0    //Right Shoulder Roll
  ,-10  //Right Shoulder Pitch
  ,-15  //Right Hand
  ,0    //Left Shoulder Roll
  ,0    //Left Shoulder Pitch
  ,15   //Left Hand
  ,-5   //Right Foot Yaw
  ,-4   //Right Foot Pitch
  ,-5   //Left Foot Yaw
  ,-1   //Left Foot Pitch
};

int servoMaxDegree[NO_OF_SERVOS] = {
    90  //Head max turn degree
  , 90  //Waist max turn degree
  , 180  //Right Shoulder Roll max turn degree
  , 5  //Right Shoulder Pitch max turn degree
  , 25  //Right Hand max turn degree
  , 0  //Left Shoulder Roll max turn degree
  , 80  //Left Shoulder Pitch max turn degree
  , 30  //Left Hand turn degree
  , 30  //Right Foot Yaw turn degree
  , 25  //Right Foot Pitch turn degree
  , 30  //Left Foot Yaw turn degree
  , 25  //Left Foot Pitch max turn degree
};
  
  int servoMinDegree[NO_OF_SERVOS] = {
    -90  //Head max turn degree
  , -90  //Waist max turn degree
  , -180  //Right Shoulder Roll max turn degree
  , -80  //Right Shoulder Pitch max turn degree
  , -25  //Right Hand max turn degree
  , -175  //Left Shoulder Roll max turn degree
  , -5  //Left Shoulder Pitch max turn degree
  , -20  //Left Hand turn degree
  , -30  //Right Foot Yaw turn degree
  , -25  //Right Foot Pitch turn degree
  , -30  //Left Foot Yaw turn degree
  , -25  //Left Foot Pitch max turn degree
};

int defaultPOS[NO_OF_SERVOS] = { 90, 90,  0, 130, 90,180, 50, 90, 90, 90, 90, 90}; //Standing still
int currentPOS[NO_OF_SERVOS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int newPOS[NO_OF_SERVOS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int incPOS[NO_OF_SERVOS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long millisPOS[NO_OF_SERVOS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//Eyes
int EYES[NO_OF_EYE_LEDS];
int currentEYES[NO_OF_EYE_LEDS] = { 0, 0, 0};
int newEYES[NO_OF_EYE_LEDS] = { 0, 0, 0};
int incEYES[NO_OF_EYE_LEDS] = { 0, 0, 0};
unsigned long millisEYE[NO_OF_EYE_LEDS] = { 0, 0, 0};

//General global variables
unsigned long currentMillis=0;
unsigned long previousMillis=0;
int tmp = 0;
boolean waitFlag = false;

void initializeEyes(){
  EYES[R] = 6;           // Red LED of eyes
  EYES[G] = 5;           // Green LED of eyes
  EYES[B] = 3;           // Blue LED of eyes
  for(int i = 0; i < NO_OF_EYE_LEDS; i++){
    analogWrite(EYES[i],0);
  }
}

void initializeServos(){
  servos[0].attach(10);   // Head yaw
  servos[1].attach(11);   // Waist yaw
  servos[2].attach(9);    // R Sholder roll
  servos[3].attach(8);    // R Sholder pitch
  servos[4].attach(7);    // R Hand grip
  servos[5].attach(12);   // L Sholder roll
  servos[6].attach(13);   // L Sholder pitch
  servos[7].attach(14);   // L Hand grip
  servos[8].attach(4);    // R Foot yaw
  servos[9].attach(2);    // R Foot pitch
  servos[10].attach(15);  // L Foot yaw
  servos[11].attach(16);  // L Foot pitch
  for(int i = 0; i < NO_OF_SERVOS; i++){
    servos[i].write(servoOffset[i] + defaultPOS[i]);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(POWER, OUTPUT);
  digitalWrite(POWER, HIGH);
  initializeEyes();
  initializeServos();
  resetServos();
  resetEYES();
  Serial.println("Rapiro setup complete!");
}

void loop() {
  currentMillis = millis();
  if(updateAllEYES() && updateAllServos()){
    if(waitFlag){
      Serial.println("ok");
      waitFlag = false;
    }
  }
  
}

void setNewEYE(int inputEYE, int inputVal, int inputTime){
  inputVal = constrain(inputVal,0,255);
  if(inputEYE < 0 || inputEYE >= NO_OF_EYE_LEDS || inputVal == currentEYES[inputEYE]) return;
  int eyeChange = inputVal - currentEYES[inputEYE];
  eyeChange = abs(eyeChange);
  incEYES[inputEYE] = (inputTime / eyeChange); //Round down so it finishes early instead of late
  newEYES[inputEYE] = inputVal;
  millisEYE[inputEYE] = currentMillis + incEYES[inputEYE];
}

void setNewPOS(int inputServo, int inputVal, int inputTime){
  inputVal = constrain(inputVal,servoMinDegree[inputServo],servoMaxDegree[inputServo]);
  if(inputServo < 0 || inputServo >= NO_OF_SERVOS || inputVal == currentPOS[inputServo]) return;
  int posChange = inputVal - currentPOS[inputServo];
  posChange = abs(posChange);
  incPOS[inputServo] = (inputTime / posChange); //Round down so it finishes early instead of late
  newPOS[inputServo] = inputVal;
  millisPOS[inputServo] = currentMillis + incPOS[inputServo];
}

boolean updateEYE(int inputEYE){
  //Serial.println("updateEYE()");
  if(inputEYE >= NO_OF_EYE_LEDS || inputEYE < 0) return false;
  if(newEYES[inputEYE] == currentEYES[inputEYE]) return true;
  //Serial.println(millis());
  //Serial.println(millisEYE[inputEYE]);
  if(currentMillis >= millisEYE[inputEYE]){ //Time is past the increment interval
    if(newEYES[inputEYE] > currentEYES[inputEYE]){ //New value is larger
      currentEYES[inputEYE]++;
    } else { //new value is smaller
      currentEYES[inputEYE]--;
    }
    //Serial.println("setting eye");
    analogWrite(EYES[inputEYE],currentEYES[inputEYE]);
    millisEYE[inputEYE] = millisEYE[inputEYE] + incEYES[inputEYE];
    return false;
  }
  return false;
}

boolean updatePOS(int inputServo){
  //Serial.println("updateEYE()");
  if(inputServo >= NO_OF_SERVOS || inputServo < 0) return false;
  if(newPOS[inputServo] == currentPOS[inputServo]) return true;
  //Serial.println(millis());
  //Serial.println(millisEYE[inputEYE]);
  if(currentMillis >= millisPOS[inputServo]){ //Time is past the increment interval
    if(newPOS[inputServo] > currentPOS[inputServo]){ //New value is larger
      currentPOS[inputServo]++;
    } else { //new value is smaller
      currentPOS[inputServo]--;
    }
    //Serial.println("setting eye");
    servos[inputServo].write(currentPOS[inputServo] + defaultPOS[inputServo]);
    millisPOS[inputServo] = millisPOS[inputServo] + incPOS[inputServo];
    return false;
  }
  return false;
}

void setAllEYES(int red, int green, int blue, int inputTime){
  setNewEYE(0,red,inputTime);
  setNewEYE(1,green,inputTime);
  setNewEYE(2,blue,inputTime);
}

void setAllServos(int servo0, int servo1, int servo2, int servo3, int servo4, int servo5, int servo6, int servo7, int servo8, int servo9, int servo10, int servo11, int inputTime){
  setNewPOS(0,servo0,inputTime);
  setNewPOS(1,servo1,inputTime);
  setNewPOS(2,servo2,inputTime);
  setNewPOS(3,servo3,inputTime);
  setNewPOS(4,servo4,inputTime);
  setNewPOS(5,servo5,inputTime);
  setNewPOS(6,servo6,inputTime);
  setNewPOS(7,servo7,inputTime);
  setNewPOS(8,servo8,inputTime);
  setNewPOS(9,servo9,inputTime);
  setNewPOS(10,servo10,inputTime);
  setNewPOS(11,servo11,inputTime);
}

boolean updateAllEYES(){
  //Serial.println("updateAllEYES()");
  boolean result1 = updateEYE(0);
  boolean result2 = updateEYE(1);
  boolean result3 = updateEYE(2);
  return(result1 && result2 && result3);
}

boolean updateAllServos(){
  //Serial.println("updateAllEYES()");
  boolean result1 = updatePOS(0);
  boolean result2 = updatePOS(1);
  boolean result3 = updatePOS(2);
  boolean result4 = updatePOS(3);
  boolean result5 = updatePOS(4);
  boolean result6 = updatePOS(5);
  boolean result7 = updatePOS(6);
  boolean result8 = updatePOS(7);
  boolean result9 = updatePOS(8);
  boolean result10 = updatePOS(9);
  boolean result11 = updatePOS(10);
  boolean result12 = updatePOS(11);
  
  return(result1 && result2 && result3 && result4 && result5 && result6 && result7 && result8 && result9 && result10 && result11 && result12);
}

void resetEYES(){
  setAllEYES(0,0,0,0);
}

void resetServos(){
  setAllServos(0,0,0,0,0,0,0,0,0,0,0,0,0);
}

int getDistance(){
  return analogRead(6);
}

String getStatus(){
  String tmpString = "";
  for(int i=0;i<NO_OF_SERVOS;i++){
    tmpString += currentPOS[i];
    tmpString += ",";
  }
  for(int i=0;i<NO_OF_EYE_LEDS;i++){
    tmpString += currentEYES[i];
    tmpString += ",";
  }
  tmpString += getDistance();
  return tmpString;
}

int serialTmp[(NO_OF_SERVOS + NO_OF_EYE_LEDS + 1)];
void serialEvent(){
  while(Serial.available() > 0){
    char inChar = Serial.read();
    switch(inChar){
      default:
        Serial.read();
        break;
      case 'r':
        resetServos();
        resetEYES();
      case 'a':
        serialTmp[0] = Serial.parseInt();
        serialTmp[1] = Serial.parseInt();
        serialTmp[2] = Serial.parseInt();
        serialTmp[3] = Serial.parseInt();
        serialTmp[4] = Serial.parseInt();
        serialTmp[5] = Serial.parseInt();
        serialTmp[6] = Serial.parseInt();
        serialTmp[7] = Serial.parseInt();
        serialTmp[8] = Serial.parseInt();
        serialTmp[9] = Serial.parseInt();
        serialTmp[10] = Serial.parseInt();
        serialTmp[11] = Serial.parseInt();
        serialTmp[12] = Serial.parseInt();
        serialTmp[13] = Serial.parseInt();
        serialTmp[14] = Serial.parseInt();
        serialTmp[15] = Serial.parseInt();
        setAllServos(serialTmp[0],serialTmp[1],serialTmp[2],serialTmp[3],serialTmp[4],serialTmp[5],serialTmp[6],serialTmp[7],serialTmp[8],serialTmp[9],serialTmp[10],serialTmp[11],serialTmp[15]);
        setAllEYES(serialTmp[12],serialTmp[13],serialTmp[14],serialTmp[15]);
        break;
      case 'i':
        serialTmp[0] = Serial.parseInt();
        serialTmp[1] = Serial.parseInt();
        serialTmp[2] = Serial.parseInt();
        setNewPOS(serialTmp[0],serialTmp[1],serialTmp[2]);
        break;
      case 'e':
        serialTmp[0] = Serial.parseInt();
        serialTmp[1] = Serial.parseInt();
        serialTmp[2] = Serial.parseInt();
        serialTmp[3] = Serial.parseInt();
        setAllEYES(serialTmp[0],serialTmp[1],serialTmp[2],serialTmp[3]);
        break;
      case 'd':
        Serial.println(getDistance());
        break;
      case 's':
        Serial.println(getStatus());
    }
  }
  waitFlag = true;
}
