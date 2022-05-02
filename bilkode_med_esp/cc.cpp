#include "common.h"
#include "cc.h"
#include <Arduino.h>

void cc::init()
{
    // Start linesensors
    lineSensor.initFiveSensors();
    // Start Serial
    Serial.begin(115200);
    // Start encoders
    encoders.init();
    // Start LCD
    lcd.init();

    // Display message on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Press B");
    lcd.setCursor(0, 1);
    lcd.print("to calibrate");
    // Display message in Node-RED
    send_value("status", "waiting for button");
    // Wait for button to be pressed
    buttonB.waitForButton();

    // Display calibration message
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Line cal");
    // Delay so user can remove hand
    delay(1000);

    // Calibrate line sensors
    for (int i = 0; i <= 120; i++)
    {
        if (i > 30 && i <= 90)
        {
            motors.setSpeeds(-150, 150);
        }
        else
        {
            motors.setSpeeds(150, -150);
        }
        lineSensor.calibrate();
    }
    motors.setSpeeds(0, 0);

    // Delay so user can straighten up car
    delay(1000);

    // Display message on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Done");
    delay(1000);
    lcd.clear();

    // Send values to Node-RED
    send_value("status", cc_state);
    delay(100);
    send_value("job", job);
    delay(100);
    send_value("mode", mode);
}

void cc::loop()
{
    // Only runs car if it is in auto mode
    if (mode == "auto")
    {
        // Checks what state car is in, then runs connected functions
        if (cc_state == "following_line")
        {
            state_follow_line();
        }
        else if (cc_state == "intersection")
        {
            state_intersection();
        }
        else if (cc_state == "lost_line")
        {
            state_lost_line();
        }
        else if (cc_state == "getting_post")
        {
            state_getting_post();
        }
        else if (cc_state == "delivering_post")
        {
            state_delivering_post();
        }
        else if (cc_state == "going_to_charger")
        {
            state_going_to_charger();
        }
    }
}

void cc::state_follow_line()
{
    // Runs linefollower if it hasnt encountered an intersection
    if (intersection_type == 0)
    {
        followLine();
        lost_line();
    }
    intersection_checker();
}

void cc::state_intersection()
{
    // Small delay before checking if there is a line across the intersection
    delay(delay_after_intersection);

    // Checks if there is a line across the intersection
    lineSensor.readCalibrated(linesensorValues);
    if (linesensorValues[1] > high_buffer || linesensorValues[2] > high_buffer || linesensorValues[3] > high_buffer)
    {
        intersection_straight = 1;
    }

    // If there isnt a line across the intersection, turn in direction of connecting line
    if (intersection_type == 1 && intersection_straight == 0)
    {
        turn_sharp(true);
        intersection_type = 0;
        cc_state = "following_line";
        send_value("status", cc_state);
    }
    else if (intersection_type == 2 && intersection_straight == 0)
    {
        turn_sharp(false);
        intersection_type = 0;
        cc_state = "following_line";
        send_value("status", cc_state);
    }

    // if lines both ways, stop
    else if (intersection_type == 3 && intersection_straight == 0)
    {
        motors.setSpeeds(0, 0);
    }

    // If we have a 3way intersection where one is straight, check if we are supposed to turn off
    else if (intersection_type == 1 && intersection_straight == 1)
    {
        turn_off();
    }
    else if (intersection_type == 2 && intersection_straight == 1)
    {
        turn_off();
    }

    // If 4 way intersection, stop
    else if (intersection_type == 3 && intersection_straight == 1)
    {
        motors.setSpeeds(0, 0);
    }
}

void cc::state_lost_line()
{
    // Reads linesensor
    lineSensor.readCalibrated(linesensorValues);

    // if we stil havent found line after 300ms turn around and go back to line
    if (millis() - lost_line_time > 300 && repeat == 0)
    {
        motors.setSpeeds(0, 0);
        if (repeat == 0)
        {
            turn_sharp(false);
            turn_sharp(false);
            motors.setSpeeds(speedValue, speedValue);
            repeat = 1;
        }
    }

    // If we find line, continue to follow that line
    else if (linesensorValues[1] > high_buffer || linesensorValues[2] > high_buffer || linesensorValues[3] > high_buffer)
    {
        repeat = 0;
        cc_state = "following_line";
        send_value("status", cc_state);
    }
}

void cc::state_getting_post()
{
    // Stepped sequence, step says where in the following program we are
    // Step 0, follows the line to the end, then turns 180 degrees
    if (step == 0)
    {
        followLine();
        if (linesensorValues[0] < low_buffer && linesensorValues[1] < low_buffer &&
            linesensorValues[2] < low_buffer && linesensorValues[3] < low_buffer && linesensorValues[4] < low_buffer)
        {
            motors.setSpeeds(0, 0);
            turn_sharp(true);
            turn_sharp(true);
            wait_at_place = millis();
            step = 1;
        }
    }
    // step 1 wait at the end of the line for 5 seconds
    else if (step == 1)
    {
        if (millis() - wait_at_place > 5000)
        {
            step = 2;
        }
    }
    // step 2 go back to intersection
    else if (step == 2)
    {
        followLine();
        if (linesensorValues[0] > high_buffer || linesensorValues[4] > high_buffer)
        {
            motors.setSpeeds(speed_trough_intersection, speed_trough_intersection);
            delay(delay_after_intersection);
            step = 3;
        }
    }
    // step 3 turn and continue around the map loop
    else if (step == 3)
    {
        lineSensor.readCalibrated(linesensorValues);
        if (linesensorValues[1] < low_buffer && linesensorValues[2] < low_buffer && linesensorValues[3] < low_buffer)
        {
            if (intersection_type == 1)
            {
                turn_sharp(true);
            }
            else if (intersection_type == 2)
            {
                turn_sharp(false);
            }
            step = 0;
            intersection_type = 0;
            intersection_straight = 0;
            job = "delivering_post";
            send_value("job", job);
            delay(100);
            cc_state = "following_line";
            send_value("status", cc_state);
        }
    }
}

void cc::state_delivering_post()
{
    // Stepped sequence, step says where in the following program we are
    // Step 0, follows the line to the end, then turns 180 degrees
    if (step == 0)
    {
        followLine();
        if (linesensorValues[0] < low_buffer && linesensorValues[1] < low_buffer &&
            linesensorValues[2] < low_buffer && linesensorValues[3] < low_buffer && linesensorValues[4] < low_buffer)
        {
            motors.setSpeeds(0, 0);
            turn_sharp(true);
            turn_sharp(true);
            wait_at_place = millis();
            step = 1;
        }
    }
    // step 1 wait at the end of the line for 1 second
    else if (step == 1)
    {
        if (millis() - wait_at_place > 1000)
        {
            house_count++;
            step = 2;
        }
    }
    // step 2 go back to intersection
    else if (step == 2)
    {
        followLine();
        if (linesensorValues[0] > high_buffer || linesensorValues[4] > high_buffer)
        {
            motors.setSpeeds(speed_trough_intersection, speed_trough_intersection);
            delay(delay_after_intersection);
            step = 3;
        }
    }
    // step 3, turns to continue around the map loop, resets to getting post if all houses has been selievered to
    else if (step == 3)
    {
        lineSensor.readCalibrated(linesensorValues);
        if (linesensorValues[1] < low_buffer && linesensorValues[2] < low_buffer && linesensorValues[3] < low_buffer)
        {
            if (house_count == houses)
            {
                house_count = 0;
                job = "getting_post";
                send_value("job", job);
            }
            if (intersection_type == 1)
            {
                turn_sharp(true);
            }
            else if (intersection_type == 2)
            {
                turn_sharp(false);
            }
            step = 0;
            intersection_type = 0;
            intersection_straight = 0;
            cc_state = "following_line";
            send_value("status", cc_state);
        }
    }
}

void cc::state_going_to_charger()
{
    // Stepped sequence, step says where in the following program we are
    // Step 0, follows the line to the end, then turns 180 degrees, sets the car in stop mode
    if (step == 0)
    {
        followLine();
        if (linesensorValues[0] < low_buffer && linesensorValues[1] < low_buffer &&
            linesensorValues[2] < low_buffer && linesensorValues[3] < low_buffer && linesensorValues[4] < low_buffer)
        {
            motors.setSpeeds(0, 0);
            turn_sharp(true);
            turn_sharp(true);
            mode = "stop";
            send_value("mode", mode);
            step = 1;
        }
    }
    // If car gets out of stop mode, go to step 2
    else if (step == 1)
    {
        step == 2;
    }
    // step 2 return to intersection
    else if (step == 2)
    {
        followLine();
        if (linesensorValues[0] > high_buffer || linesensorValues[4] > high_buffer)
        {
            motors.setSpeeds(speed_trough_intersection, speed_trough_intersection);
            delay(delay_after_intersection);
            step = 3;
        }
    }
    // step 3 turn and continue around the map loop
    else if (step == 3)
    {
        lineSensor.readCalibrated(linesensorValues);
        if (linesensorValues[1] < low_buffer && linesensorValues[2] < low_buffer && linesensorValues[3] < low_buffer)
        {
            if (intersection_type == 1)
            {
                turn_sharp(true);
            }
            else if (intersection_type == 2)
            {
                turn_sharp(false);
            }
            step = 0;
            intersection_type = 0;
            intersection_straight = 0;
            cc_state = "following_line";
            send_value("status", cc_state);
        }
    }
}

void cc::followLine()
{
    // reads linesensor
    pos = lineSensor.readLine(linesensorValues);
    // check what the error is
    error = pos - 2000;
    // sets the wished speed with PD regulator
    int speedDifference = (error * Kp) + (error - lastError) * Td;

    // update left motor speed
    int leftSpeed = 400 + speedDifference;
    leftSpeed = constrain(leftSpeed, 0, speedValue);
    // update right motor speed
    int rightSpeed = 400 - speedDifference;
    rightSpeed = constrain(rightSpeed, 0, speedValue);
    // Set speeds
    motors.setSpeeds(leftSpeed, rightSpeed);
    // Save last error
    lastError = error;
}

void cc::intersection_checker()
{
    // read linesensor
    lineSensor.readCalibrated(linesensorValues);
    // Check if outer sensors are above high_buffer
    if ((linesensorValues[0] > high_buffer || linesensorValues[4] > high_buffer) && linesensorValues[2] > high_buffer)
    {
        // Sets type of intersection
        if (linesensorValues[0] > high_buffer && intersection_type == 0)
        {
            intersection_type = 1;
        }
        if (linesensorValues[4] > high_buffer && intersection_type == 0)
        {
            intersection_type = 2;
        }
        if (linesensorValues[0] > high_buffer && linesensorValues[4] > high_buffer)
        {
            intersection_type = 3;
        }
        // If intersection is encountered, stop linefollower and set motors to set speed
        if (intersection_type != 0)
        {
            motors.setSpeeds(speed_trough_intersection, speed_trough_intersection);
        }
    }

    // checks if we have passed the intersection
    if (intersection_type == 1 && linesensorValues[0] < low_buffer)
    {
        cc_state = "intersection";
        send_value("status", cc_state);
    }
    if (intersection_type == 2 && linesensorValues[4] < low_buffer)
    {
        cc_state = "intersection";
        send_value("status", cc_state);
    }
    if (intersection_type == 3 && (linesensorValues[0] < low_buffer || linesensorValues[4] < low_buffer))
    {
        cc_state = "intersection";
        send_value("status", cc_state);
    }
}

void cc::lost_line()
{
    // Checks if we have lost the line, if so, change state to lost_line
    if (linesensorValues[0] < low_buffer && linesensorValues[1] < low_buffer &&
        linesensorValues[2] < low_buffer && linesensorValues[3] < low_buffer && linesensorValues[4] < low_buffer)
    {
        motors.setSpeeds(speedValue, speedValue);
        lost_line_time = millis();
        cc_state = "lost_line";
        send_value("status", cc_state);
    }
}

void cc::turn_sharp(bool left)
{
    // reset encoders
    encoder_turn_count = 0;
    encoders.getCountsAndResetLeft();
    encoders.getCountsAndResetRight();
    // turn until correct encoder count is reached
    while (encoder_turn_count < 950)
    {
        // if left turn, turn left
        if (left)
        {
            motors.setSpeeds(-speedValue, speedValue);
            encoder_turn_count = abs(encoders.getCountsLeft()) + abs(encoders.getCountsRight()) / 2;
        }
        else
        {
            motors.setSpeeds(speedValue, -speedValue);
            encoder_turn_count = abs(encoders.getCountsLeft()) + abs(encoders.getCountsRight()) / 2;
        }
    }
    motors.setSpeeds(0, 0);
}

void cc::turn_off()
{
    // checks if the current job is relevant to the intersection number, if so then turns off
    if (job == "getting_post" && map_layout[intersection_number] == 1)
    {
        if (intersection_type == 1)
        {
            turn_sharp(true);
        }
        else if (intersection_type == 2)
        {
            turn_sharp(false);
        }
        delay(300);
        cc_state = "getting_post";
        send_value("status", cc_state);
        update_intersection_number();
    }
    else if (job == "delivering_post" && map_layout[intersection_number] == 0)
    {
        if (intersection_type == 1)
        {
            turn_sharp(true);
        }
        else if (intersection_type == 2)
        {
            turn_sharp(false);
        }
        cc_state = "delivering_post";
        send_value("status", cc_state);
        update_intersection_number();
    }
    // if we need charge,and intersection is charger, turn off
    else if (need_charge && map_layout[intersection_number] == 2)
    {
        if (intersection_type == 1)
        {
            turn_sharp(true);
        }
        else if (intersection_type == 2)
        {
            turn_sharp(false);
        }
        cc_state = "going_to_charger";
        send_value("status", cc_state);
        update_intersection_number();
    }
    else
    // if none apply, then up the intersection number and continue around the map
    {
        update_intersection_number();
        intersection_type = 0;
        intersection_straight = 0;
        cc_state = "following_line";
        send_value("status", cc_state);
    }
}

void cc::update_intersection_number()
{
    // increase intersection number
    intersection_number++;
    // Check if we have reached the end of the map
    if (intersection_number > map_size - 1)
    {
        intersection_number = 0;
    }
}