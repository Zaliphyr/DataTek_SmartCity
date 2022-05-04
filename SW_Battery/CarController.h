#pragma once
#define CarController_h
#include "common.h"

class CarController
{
public:
    unsigned int low_buffer = 600;
    unsigned int high_buffer = 800;

    String mode = "auto";
    String cc_state = "following_line";
    int want = 10;
    /* job items:
        getting_post: Going to post office and picking up post
        delivering_post: Going to houses to deliver post
    */
    String job = "getting_post";
    // If true then stops at chrger to charge
    bool need_charge = false;

    // linefollower variables
    int pos;
    unsigned int linesensorValues[5];
    int error;
    int lastError = 0;
    float Kp = 0.6;
    float Td = 2;

    // intersection variables
    /*  Intersection types:
        0: No intersection
        1: Left intersection
        2: Right intersection
        3: Left and right intersection
    */
    int intersection_type = 0;
    /*  Intersection straight:
        0: No straight
        1: Straight
    */
    int intersection_straight = 0;
    int intersection_number = 0;

    int repeat = 0;
    int step = 0;

    int encoder_turn_count = 0;

    long unsigned int lost_line_time = 0;
    long unsigned int wait_at_place = 0;

    //* Public functions:
    /**
     * @brief The init function calibrates the cars sensors, starts serial communication and initializes the display.
     */
    void init();
    /**
     * @brief The main loop. Controls everything about the car by calling methods from the class.
     */
    void loop();

private:
    //* Private functions:

    /**
     * @brief Checks for if there is no road infront of the car.
     */
    void lost_line();

    /**
     * @brief The car's linefollower algorithm keeps the car on the tape.
     */
    void followLine();

    /**
     * @brief Tells the car to turn 90 degrees left or right.
     * @param left If true then turns left, else turns right.
     */
    void turn_sharp(bool left);
    /**
     * @brief Controls what the car should do baised on it's state.
     */
    void turn_off();

    /**
     * @brief Enables the car's linefollower if there is no intersection.
     */
    void state_follow_line();

    /**
     * @brief Controls how the car should deal with different types of intersections.
     */
    void state_intersection();

    /**
     * @brief Returns the car to the line if a dead end is found.
     */
    void state_lost_line();

    /**
     * @brief Tells the car what to do at the post office.
     */
    void state_getting_post();

    /**
     * @brief Tells the car what to do at the houses.
     */
    void state_delivering_post();

    /**
     * @brief Tells the car to go to the charger.
     */
    void state_going_to_charger();

    /**
     * @brief Checks relative to the map where the car is.
     */
    void update_intersection_number();

    /**
     * @brief Checks if the car is at an intersection.
     */
    void intersection_checker();

    /**
     * @brief Calibrates the car's sensors.
     */
    void calibration();
};