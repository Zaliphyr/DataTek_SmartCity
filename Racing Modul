// import libraries
#include <Zumo32U4.h>
#include <Wire.h>


// give descriptive names to the libraries
Zumo32U4ButtonA buttonA;
Zumo32U4Motors motors;     
Zumo32U4Encoders encoders;
Zumo32U4LCD lcd;
Zumo32U4LineSensors lineSensor;
Zumo32U4Buzzer buzzer;


// create necessary variables
int pos;
unsigned int linesensorValues[5];
int error;
const int wantedValue = 2000;
float Kp = 0.6; 
float Td = 2.4; 
int lastError = 0;
float speedValue = 400;
int leftCorner = 0;
int rightCorner = 0;
int cornerTime = 0;
int turnTime = 0;
int noRoadLeft = 0;
int noRoadRight = 0;
int deadEnd = 0;
int deadTime = 0;
int rightNext = 0;
int negativeTurnSpeed;
int turnRefrence ;
int afterNext = 0;
int afterTime = 0;


void setup() {
  // activates the five frontlinesensors
  lineSensor.initFiveSensors();
  
  Serial.begin(9600);
  
  //start protocol
  lcd.init();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Press A Button");
  buttonA.waitForButton();
  lcd.clear();
  cal();
  delay(1000);
  lcd.setCursor(0,0);
  lcd.print("Calb");
  lcd.setCursor(0,1);
  lcd.print("Done");
  delay(1000);
  buzzer.playFrequency(440, 500, 8);
  delay(1000);
  lcd.clear();
  
}

void loop() {
  //reads all the front sensors
  lineSensor.readCalibrated(linesensorValues);

  // Checks if the car drives over a 90degree leftturn
  // initiates if the middle and two left sensors are very high, and first right sensor i fairly low
  if (linesensorValues[0] >= 800 && linesensorValues[2] >= 800 && linesensorValues[1] >= 800 && leftCorner == 0 && linesensorValues[3] > 80){
    leftCorner = 1;
    cornerTime = millis();
    deadEnd = 1;
    deadTime = millis();
    turnTime = millis();
    noRoadLeft = 1;
  }

  // Checks if the car drives over a 90degree rightturn
  // initiates if the middle and two right sensors are very high, and first left sensor i fairly low
  else if (linesensorValues[3] >= 800 && linesensorValues[2] >= 800 && linesensorValues[4] >= 800 && rightCorner == 0 && linesensorValues[1] > 80){
    cornerTime = millis();
    rightCorner = 1;
    deadEnd = 1;
    deadTime = millis();
    turnTime = millis();
    noRoadRight = 1;
  }

  // initiates if no recent 90 degreeturn
  if( leftCorner == 0 && rightCorner == 0){
    // reads the five sensorvalues in the list, and gives a value between 0 and 4000
    pos = lineSensor.readLine(linesensorValues);
    // repositions the pos value to between 2000 and -2000, which makes 0 straight forward
    error = pos - wantedValue;
    // how much off straight + the last value of that times two different konstants
    int speedDifference = (error * Kp) + Td * (error - lastError);
    // if it off to the right, leftside reduces speed and right turns up
    int leftSpeed = 400 + speedDifference;
    //constrains the speed to under 400 and over 0
    leftSpeed = constrain(leftSpeed, 0, speedValue);
    // if off to the left, right loses speed and left side turns up
    int rightSpeed = 400 - speedDifference;
    //constrains the speed to under 400 and over 0
    rightSpeed = constrain(rightSpeed, 0, speedValue);
    // applies the calculated speeds to the car
    motors.setSpeeds(leftSpeed, rightSpeed);
    lastError = error;
  }
  
  // initiates if the car recently hit a deadend shortly after a 90degree turn and turned 180 degrees.
  else if (rightNext == 1){
    // initiates if there is a right turn shortly after the car turned 180.
    if (rightCorner == 1){
      // calls function to turn, and tell it to turn 90 degrees, with 400 speed, with both wheels.
      turnAngle(90, 400, true);
      rightNext =0; 
      rightCorner = 0;
      deadEnd = 0;
      // this will block the if-statement when all sensors are fairly low, for 100ms.
      afterNext = 1;
      afterTime = millis();
      // reads the sensors again.
      lineSensor.readCalibrated(linesensorValues);
    }
  }

  // the car will go straight if it reads a 90degree turn
  else if (rightCorner == 1 || leftCorner ==1){
    motors.setSpeeds (400, 400);
  }

  // initiates if all sensors read fairly low values (none are on the black tape), and afternext are zero.
  if (linesensorValues[0] < 200 && linesensorValues[1] < 200 && linesensorValues[2] < 200 && linesensorValues[3] < 200 && linesensorValues[4] < 200 && afterNext == 0){

    // initiates if no recent 90 degree turns
    if (deadEnd == 0 && noRoadLeft == 0 && noRoadRight == 0 && straightRegret == 0){
      motors.setSpeeds(400, 400);
      // if (straightRegretTime == 0){
      //   straightRegretTime = millis();
      // }
    }
    
    //initiates if there is a 90degree turn with tape after it, and a little later a deadend.
    else if (deadEnd == 1 && noRoadLeft == 0 && noRoadRight == 0){
      turnAngle(180, 400, true); // turns the car 180 degrees
      // initiates the the car turn 90 degrees right before straight ahead i next turn if-statement.
      rightNext = 1;
    }

    // initiates right after a 90degrre leftturn, if there are no tape (all sensors are fairly low).
    else if (noRoadLeft == 1 && deadEnd == 1){
      //turns the car 90 degrees to the left.
      turnAngle(-90, 400, true);
      //resets the parameter values.
      noRoadLeft = 0;
      deadEnd = 0;
    }
    // same as over, but towards the right
    else if (noRoadRight == 1 && deadEnd == 1){
      turnAngle(90, 400, true);
      noRoadRight = 0;
      deadEnd = 0;
    }

  }

  // resets the corner values fast after a 90degree turn, the variables lasts for 10ms
  if ((millis() - cornerTime) > 10 && (millis() - cornerTime) < 20){
    leftCorner = 0;
    rightCorner = 0;
  }

  // resets the "no tape after a 90degree values" 150ms after a 90degree turn.
  if ((millis() - turnTime) >= 150){
    noRoadLeft = 0;
    noRoadRight = 0;
  }

  // resets the "deadend shortly after e 90degree turn" after 550ms
  if ((millis() - deadTime)>= 550){
    deadEnd = 0;
  }

  // resets the " block low sensorvalues after a 90degree right turn after a 180 degree rotation" after 100ms.
  if ((millis() - afterTime)>= 100){
    afterNext = 0;
  }

  
}


// Calibration function
// Turns car left and right while calibrating the linesensor
void cal(){
  delay(1000);
  for (int i = 0; i <= 120; i++){
    if (i > 30 && i <= 90){
      motors.setSpeeds(-150, 150);
    } else {
      motors.setSpeeds(150, -150);
    }
    lineSensor.calibrate();
  }
  motors.setSpeeds(0, 0);
}

void turnAngle(int angle, int turnSpeed, bool turnWithBothWheels){
  // turn the robot by angle degrees
  int leftStartAngle = encoders.getCountsLeft();
  int rightStartAngle = encoders.getCountsRight();

  motors.setSpeeds(0, 0);
  delay(100);

  if (turnWithBothWheels == true)
  {
      negativeTurnSpeed = -turnSpeed;
      turnRefrence = 7.5;
  }
  else
  {
      negativeTurnSpeed = 0;
      turnRefrence = 14.5;
  }

  if (angle > 0)
  {
      while (angle > (encoders.getCountsLeft() - leftStartAngle) / turnRefrence)
      {
          motors.setSpeeds(turnSpeed, negativeTurnSpeed);
      }
  }
  else if (angle < 0)
  {
      while (abs(angle) > (encoders.getCountsRight() - rightStartAngle) / turnRefrence)
      {
          motors.setSpeeds(negativeTurnSpeed, turnSpeed);
      }
  }
  motors.setSpeeds(0, 0);
}
