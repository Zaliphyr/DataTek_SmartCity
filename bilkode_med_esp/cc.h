#pragma once
#define cc_h
#include "common.h"

class cc
{
    // Needed for callback function
    using m_cb = void (*)(String topic, String value);

    // everything public
    public:
        //* @brief Low buffer for line detection
        unsigned int low_buffer = 600;
        //* @brief High buffer for line detection
        unsigned int high_buffer = 700;
        //* @brief Delay in ms before turning after end of intersection detection
        int delay_after_intersection = 100;
        //* @brief Motor speed trough intersection
        int speed_trough_intersection = 200;

        //* @brief Mode of car, autonomous or manual
        String mode = "auto";
        //* @brief Current status, what is the car doing
        String cc_state = "following_line";
        //* @brief Current job
        String job = "getting_post";
        //* @brief Makes the car stop at charger when passing it
        bool need_charge = true;
        //* @brief Counts how many houses car has delievered packages to
        int house_count = 0;

        // linefollower variables
        //* @brief Current line sensor value
        int pos;
        //* @brief Raw linesensor values
        unsigned int linesensorValues[5];
        //* @brief diffrence in wanted lineseensor value and current lineseensor value
        int error;
        //* @brief The last error
        int lastError = 0;
        //* @brief PD regulator P constant
        const float Kp = 0.6;
        //* @brief PD regulator D constant
        const float Td = 2;

        // intersection variables
        /**
        * @brief Intersection type
        *   0: No intersection
        *   1: Left intersection
        *   2: Right intersection
        *   3: Left and right intersection
        */
        int intersection_type = 0;
        /**
         * @brief Intersection straight
         * 0: No straight
         * 1: Straight
         */
        int intersection_straight = 0;
        //* @brief Counts how many intersections car has passed
        int intersection_number = 0;

        //* @brief Variable for events that only happen once
        int repeat = 0;
        //* @brief Variable for stepped events
        int step = 0;
        
        //* @brief Counter for turns using the encoder
        int encoder_turn_count = 0;

        //* @brief Variable for timed events
        unsigned long lost_line_time = 0;
        unsigned long wait_at_place = 0;

        /**
         * @brief Function that initializes the car controller
         */
        void init();
        /**
         * @brief Function that runs the car controller
         */
        void loop();
        /**
         * @brief Function that adds function for sending variables to kom_sys
         * @param cb The kom_sys function to add
         */
        void add_callback(m_cb cb)
        {
            send_value = cb;
        }

    private:
        //* @brief Variable for callback function
        m_cb send_value;
        //* @brief Function that checks if we are at an intersection
        void intersection_checker();
        //* @brief Function that checks if we have lost the line
        void lost_line();
        //* @brief Function that runs the linefollower
        void followLine();
        /**
         * @brief Function that turns the car 90 degrees
         * @param left True if left turn, false if right turn
         */
        void turn_sharp(bool left);
        //* @brief Checks if we should turn off at that intersection
        void turn_off();
        //* @brief Function that updates the intersection counter
        void update_intersection_number();
        
        // State functions
        //* @brief Function that runs while following line
        void state_follow_line();
        //* @brief Function that runs when encountering an intersection
        void state_intersection();
        //* @brief Function that runs when we have lost the line
        void state_lost_line();
        //* @brief Function that runs when getting post
        void state_getting_post();
        //* @brief Function that runs when delivering post
        void state_delivering_post();
        //* @brief Function that runs when going to charger
        void state_going_to_charger();
};
