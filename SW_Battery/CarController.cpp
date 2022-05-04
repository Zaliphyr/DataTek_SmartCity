// Includes libraries
#include "common.h"

void CarController::init()
{
    lineSensor.initFiveSensors();
    Serial.begin(115200);

    Wire.begin();
    encoders.init();
    lcd.init();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Press B");
    lcd.setCursor(0, 1);
    lcd.print("to calibrate");

    buttonB.waitForButton();
    calibration();
    lcd.clear();
    lcd.setCursor(0, 0);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Line cal");
}

void CarController::calibration()
{

    delay(1000);
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
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Done");
    delay(1000);
    lcd.clear();
}

void CarController::loop()
{
    if (mode == "auto")
    {
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

void CarController::state_follow_line()
{
    if (intersection_type == 0)
    {
        followLine();
        lost_line();
    }
    intersection_checker();
}

void CarController::state_intersection()
{
    lineSensor.readCalibrated(linesensorValues);
    if (linesensorValues[1] > high_buffer || linesensorValues[2] > high_buffer || linesensorValues[3] > high_buffer)
    {
        intersection_straight = 1;
    }
    if (intersection_type == 1 && intersection_straight == 0)
    {
        turn_sharp(true);
        intersection_type = 0;
        cc_state = "following_line";
    }
    else if (intersection_type == 2 && intersection_straight == 0)
    {
        turn_sharp(false);
        intersection_type = 0;
        cc_state = "following_line";
    }
    else if (intersection_type == 3 && intersection_straight == 0)
    {
        motors.setSpeeds(0, 0);
    }
    else if (intersection_type == 1 && intersection_straight == 1)
    {
        turn_off();
    }
    else if (intersection_type == 2 && intersection_straight == 1)
    {
        turn_off();
    }
    else if (intersection_type == 3 && intersection_straight == 1)
    {
        motors.setSpeeds(0, 0);
    }
}

void CarController::state_lost_line()
{
    lineSensor.readCalibrated(linesensorValues);
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
    else if (linesensorValues[1] > high_buffer || linesensorValues[2] > high_buffer || linesensorValues[3] > high_buffer)
    {
        repeat = 0;
        cc_state = "following_line";
    }
}

void CarController::state_getting_post()
{
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
    else if (step == 1)
    {
        if (millis() - wait_at_place > 5000)
        {
            step = 2;
        }
    }
    else if (step == 2)
    {
        followLine();
        if (linesensorValues[0] > high_buffer || linesensorValues[4] > high_buffer)
        {
            motors.setSpeeds(50, 50);
            step = 3;
        }
    }
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
        }
    }
}
void CarController::state_delivering_post()
{
}
void CarController::state_going_to_charger()
{
}

void CarController::followLine()
{
    pos = lineSensor.readLine(linesensorValues);
    error = pos - 2000;
    int speedDifference = (error * Kp) + (error - lastError) * Td;

    int leftSpeed = 400 + speedDifference;
    leftSpeed = constrain(leftSpeed, 0, speedValue);

    int rightSpeed = 400 - speedDifference;
    rightSpeed = constrain(rightSpeed, 0, speedValue);

    motors.setSpeeds(leftSpeed, rightSpeed);
    lastError = error;
}

void CarController::intersection_checker()
{
    lineSensor.readCalibrated(linesensorValues);
    if ((linesensorValues[0] > high_buffer || linesensorValues[4] > high_buffer) && linesensorValues[2] > high_buffer)
    {
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
        if (intersection_type != 0)
        {
            motors.setSpeeds(200, 200);
        }
    }
    if (intersection_type == 1 && linesensorValues[0] < low_buffer)
    {
        cc_state = "intersection";
    }
    if (intersection_type == 2 && linesensorValues[4] < low_buffer)
    {
        cc_state = "intersection";
    }
    if (intersection_type == 3 && (linesensorValues[0] < low_buffer || linesensorValues[4] < low_buffer))
    {
        cc_state = "intersection";
    }
}

void CarController::lost_line()
{
    if (linesensorValues[0] < low_buffer && linesensorValues[1] < low_buffer &&
        linesensorValues[2] < low_buffer && linesensorValues[3] < low_buffer && linesensorValues[4] < low_buffer)
    {
        motors.setSpeeds(speedValue, speedValue);
        lost_line_time = millis();
        cc_state = "lost_line";
    }
}

void CarController::turn_sharp(bool left)
{
    encoder_turn_count = 0;
    encoders.getCountsAndResetLeft();
    encoders.getCountsAndResetRight();
    while (encoder_turn_count < 1050)
    {
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

void CarController::turn_off()
{
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
        delay(2000);
        cc_state = "getting_post";
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
        update_intersection_number();
    }
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
        update_intersection_number();
    }
    else
    {
        update_intersection_number();
        intersection_type = 0;
        intersection_straight = 0;
        cc_state = "following_line";
    }
}

void CarController::update_intersection_number()
{
    intersection_number++;
    if (intersection_number > map_size - 1)
    {
        intersection_number = 0;
    }
}