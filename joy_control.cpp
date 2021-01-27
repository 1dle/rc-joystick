/*
	@author: Bence Deli
	@date: 2020-01-26
	@description:
          rc irányítása közvetlen joystickkel (xbox one controller)
*/
#include "lib/joystick/joystick.hh"
#include <pigpio.h>
#include <unistd.h>
#include <iostream>

const int AXIS_THRESHOLD = 0.10 * JoystickEvent::MAX_AXES_VALUE; //ten percent of max value

enum buttons_axes {
    STEER = 0,
    ACCELERATE = 4,//wired: 5, bluetooth: 4
    BRAKE_REVERSE = 5,//wired: 2, bluetooth: 5
    QUIT = 11 //menu button on xbox controller
};

#define IDLE_MOTOR_SPEED 1500
#define MAX_DIFF_SPEED 200 //400 is dangerous for testing i guess
#define ESC_PIN 27

#define STEER_PIN 17
#define STEER_CENTER 700
#define STEER_MAX_ANGLE 180

int main(int argc, char** argv)
{
    gpioInitialise();
    gpioSetMode(ESC_PIN, PI_OUTPUT);
    gpioSetMode(STEER_PIN, PI_OUTPUT);

    gpioServo(ESC_PIN, IDLE_MOTOR_SPEED);
    gpioServo(STEER_PIN, STEER_CENTER);

    // Create an instance of Joystick
    Joystick joystick("/dev/input/js0");

    // Ensure that it was found and that we can use it
    if (!joystick.isFound())
    {
        std::cout << "joystick not found." << std::endl;
        return 1;
    }

    bool si, ai, bi; //steer, accelerate, brake is initialized
    bool quit;

    while (!quit)
    {
        // Restrict rate
        usleep(1000);
        bool isacc, isbrake; //in current iteration are we accelerating? or breaking?

        // Attempt to sample an event from the joystick
        JoystickEvent event;
        if (joystick.sample(&event))
        {
            if (event.isButton())
            {
                if(event.number == QUIT && event.value){
                    quit = true;
                }
                /*
                printf("Button %u is %s\n",
                event.number,
                event.value == 0 ? "up" : "down");*/
            }
            if (event.isAxis())
            {
                //melyik tengelyek?
                switch(event.number){
                    case STEER:
                        if (!si){ si = true; break; } //for ignoring first, intialize values
                        if(event.value < -AXIS_THRESHOLD){
                            //balra
                            //hány százalékra balra a joy?
                            double l_percent = (event.value + 0.0 + AXIS_THRESHOLD) / (JoystickEvent::MIN_AXES_VALUE + AXIS_THRESHOLD);
                            std::cout << "balra: " <<  l_percent * 100<< "%" << std::endl;
                            gpioServo(STEER_PIN, STEER_CENTER + l_percent * STEER_MAX_ANGLE);
                        }else if(event.value > AXIS_THRESHOLD){
                            //jobbra mennyire
                            double r_percent = (event.value + 0.0 - AXIS_THRESHOLD) / (JoystickEvent::MAX_AXES_VALUE - AXIS_THRESHOLD);
                            std::cout << "jobbra: " <<  r_percent * 100 << "%" << std::endl;
                            gpioServo(STEER_PIN, STEER_CENTER - r_percent * STEER_MAX_ANGLE);
                        }else{
                            std::cout << "kozep" << std::endl;
                            gpioServo(STEER_PIN, STEER_CENTER);
                        }
                        break;
                    case ACCELERATE:
                        // (-32767; 32767)
                        // (  0%  ; 100% )
                        if (!ai){ ai = true; break; } //for ignoring first, intialize values
                        if(event.value > JoystickEvent::MIN_AXES_VALUE + AXIS_THRESHOLD){
                            double a_percent = (event.value+0.0 + JoystickEvent::MAX_AXES_VALUE - AXIS_THRESHOLD) / (2*JoystickEvent::MAX_AXES_VALUE - AXIS_THRESHOLD );
                            std::cout << "elore " << a_percent * 100 << "%" << std::endl;
                            gpioServo(ESC_PIN, IDLE_MOTOR_SPEED + MAX_DIFF_SPEED * a_percent);

                            isacc = true;
                        }else
                            isacc = false;
                        break;
                    case BRAKE_REVERSE:
                        if (!bi){ bi = true; break; } //for ignoring first, intialize values
                        if(event.value > JoystickEvent::MIN_AXES_VALUE + AXIS_THRESHOLD){

                            double b_percent = (event.value+0.0 + JoystickEvent::MAX_AXES_VALUE - AXIS_THRESHOLD) / (2*JoystickEvent::MAX_AXES_VALUE - AXIS_THRESHOLD );
                            
                            std::cout << "hatra " << b_percent * 100 << "%" << std::endl;
                            
                            gpioServo(ESC_PIN, IDLE_MOTOR_SPEED - MAX_DIFF_SPEED * b_percent);

                            isbrake = true;
                        }else{
                            isbrake = false;
                        }
                }
            }
        }
        if(!isbrake && !isacc){
            gpioServo(ESC_PIN, IDLE_MOTOR_SPEED);
            std::cout << "all" << std::endl;
        }
    }
    gpioTerminate();
}
