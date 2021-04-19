#include <iostream>
#include <unistd.h>
#include "ControllerFactory.h"
#include "RemoteRC.h"

int main()
{
    //g29 steering wheel
    Controller* con = ControllerFactory::create(ControllerFactory::XBOX_WIRELESS);

    RemoteRC rc(con);

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