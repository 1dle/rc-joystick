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

enum axes {
    STEER = 0,
    ACCELERATE = 5,
    BRAKE_REVERSE = 2
};

#define IDLE_MOTOR_SPEED 1500
#define FORWARD_MOTOR_SPEED 1560
#define BACKWARDS_MOTOR_SPEED 1410
#define ESC_PIN 27

#define STEER_PIN 17
#define STEER_CENTER 730
#define STEER_MAX_ANGLE 175

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

    while (true)
    {
        // Restrict rate
        usleep(1000);
        bool isacc, isbrake; //in current iteration are we accelerating? or breaking?

        // Attempt to sample an event from the joystick
        JoystickEvent event;
        if (joystick.sample(&event))
        {
	  	/*
	  	if (event.isButton())
	  	{
			printf("Button %u is %s\n",
			event.number,
			event.value == 0 ? "up" : "down");
	  	}*/
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
                        if (!ai){ ai = true; break; } //for ignoring first, intialize values
                        if(event.value > JoystickEvent::MIN_AXES_VALUE + AXIS_THRESHOLD){
                            std::cout << "elore" << std::endl;
                            isacc = true;
                            gpioServo(ESC_PIN, FORWARD_MOTOR_SPEED);
                        }else
                            isacc = false;
                        break;
                    case BRAKE_REVERSE:
                        if (!bi){ bi = true; break; } //for ignoring first, intialize values
                        if(event.value > JoystickEvent::MIN_AXES_VALUE + AXIS_THRESHOLD){
                            std::cout << "hatra / fek" << std::endl;
                            isbrake = true;
                            gpioServo(ESC_PIN, BACKWARDS_MOTOR_SPEED);
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