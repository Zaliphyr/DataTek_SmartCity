#pragma once
#define common_h

// include needed libraries
#include <Arduino.h>
#include <Zumo32U4.h>
#include <wire.h>

// define needed parts of car
static Zumo32U4ButtonB buttonB;
static Zumo32U4Motors motors;
static Zumo32U4LCD lcd;
static Zumo32U4LineSensors lineSensor;
static Zumo32U4Buzzer buzzer;
static Zumo32U4Encoders encoders;

//* @brief Maximum speed for motors
static int speedValue = 200;
/**
 * @brief The layout of the map.
 * 
 * The map is a 2D array of characters, where each caracter is a number describing what is at that intersection.
 *  0: House
 *  1: Post office
 *  2: Charger
 */
static int map_layout[] = {0, 1, 0, 2};
//* @brief The number intersections on the map.
static int map_size = 4;
//* @brief The number of houses on the map.
static int houses = 2;