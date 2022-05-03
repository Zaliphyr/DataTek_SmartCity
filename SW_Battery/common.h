#pragma once
#define common_h
#include <Arduino.h>
#include <Zumo32U4.h>
#include <Wire.h>
#include "CarController.h"
#include "SW_Battery.h"

// Converts method names from class to easily readable names.
static Zumo32U4ButtonA buttonA;
static Zumo32U4ButtonB buttonB;
static Zumo32U4Motors motors;
static Zumo32U4Encoders encoders;
static Zumo32U4LCD lcd;
static Zumo32U4LineSensors lineSensor;
static Zumo32U4Buzzer buzzer;

//*@breif Driving distance in meters
static int carDrivingDistance = 10;
//*@brief constant value for when the user should be notified about the battery level.
static const int dangerZone = 10;

//*@brief How fast the car should drive between 0 and 400.
static int speedValue = 400;

//*@breif Battery charge left in %.
static int charge = 100;
