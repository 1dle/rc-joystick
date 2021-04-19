#include <iostream>
#include <unistd.h>
#include "Controller.h"
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
        JoystickEvent event;
        //consume on sampling because those contains the dafault states of different inputs
        joystick.sample(nullptr);
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
            //sigterm signal gotcha
            if( quit.load() ) return 0;
        }   
    }
    
    return 0;
}