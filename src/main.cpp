#include <iostream>
#include <unistd.h>
#include "ControllerFactory.h"
#include "RemoteRC.h"
#include <atomic>
#include <csignal>
#include <cstring>

std::atomic<bool> quit(false);    // signal flag

void got_signal(int)
{
    quit.store(true);
}
int main()
{
    //these are for catching the SIGTERM signal, because if the program stopped with ctrl+c or other incorrect way,
    //the destructors are not called.
    //source: https://stackoverflow.com/a/4250601
    struct sigaction sa;
    memset( &sa, 0, sizeof(sa) );
    sa.sa_handler = got_signal;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT,&sa,NULL); //sends Ctrl+C
    sigaction(SIGTERM, &sa, NULL); //sends Clion stop button

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
        JoystickEvent event;
        while (true)
        {
            // Restrict sampling rate
            usleep(800);
            // Attempt to sample an event from the joystick
            if (joystick.sample(&event))
            {
                if(!rc.processRawInput(event)){
                    break;
                }
            }
            if( quit.load() ) return 0;
        }   
    }
    
    return 0;
}