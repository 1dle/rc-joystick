#include <iostream>
#include <unistd.h>
#include "Controller.h"
#include "RemoteRC.h"

int main()
{
    //g29
    /*constexpr controller con = {
        { 0, -3276, 3276, 0 }, //steer //10% jobbra és balra
        { 2, 32767, -32767, 32767 }, //acc pedal
        { 3, 32767, -32767, 32767 }, //brake pedal
        23 //quit btn
    };*/
    //xbox
    /*constexpr controller con = {
            { 0, -32767, 32767, 0 }, //steer //10% jobbra és balra
            { 4, -32767, 32767, -32767 }, //acc pedal
            { 5, -32767, 32767, -32767 }, //brake pedal
            11 //quit btn
    };*/
    //xbox one controller
    const Controller con(Controller::Axis(0,-32767,32767, 0),
                   Controller::Axis(4,-32767, 32767, -32767),
                   Controller::Axis(5, -32767, 32767, -32767),
                   11);

    RemoteRC rc(con, "192.168.1.123", "8888");

    if(rc.connected()){
        std::cout << "rc connected." << std::endl;
        Joystick joystick("/dev/input/js0");

        if (!joystick.isFound())
        {
            std::cout << "joystick not found." << std::endl;
            return 1;
        }
        while (true)
        {
            // Restrict sampling rate
            usleep(800);
            // Attempt to sample an event from the joystick
            JoystickEvent event;
            if (joystick.sample(&event))
            {
                if(!rc.processRawInput(event)){
                    break;
                }
            }
        }   
    }
    
    return 0;
}