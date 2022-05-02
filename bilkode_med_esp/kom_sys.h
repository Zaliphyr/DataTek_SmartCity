#pragma once
#define com_sys_h

class kom_sys
{
    public:
        //* @brief variable to hold last message
        String last_message = "none";
        //* @brief function that initializes kom_sys
        void init();
        //* @brief function that runs kom_sys
        void loop();
        /**
         * @brief Function that sends message to Node-RED
         * @param topic What is the name of the variable¨
         * @param value What is the value of the variable
         */
        static void send_value(String topic, String value);
    private:

};