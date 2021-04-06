#include <pigpiod_if2.h>
#include <iostream>
#include <unistd.h>
#include "lib/joystick/joystick.hh"

const int steer_servo_gpio = 17;


const int AXIS_THRESHOLD = 0.10 * JoystickEvent::MAX_AXES_VALUE; //ten percent of max value

#define IDLE_MOTOR_SPEED 1500
#define MAX_DIFF_SPEED 200 //400 is dangerous for testing i guess
#define ESC_PIN 27

#define STEER_PIN 17
#define STEER_CENTER 700
#define STEER_MAX_ANGLE 180

//joystick
enum buttons_axes {
    STEER = 0,
    ACCELERATE = 5,//wired: 5, bluetooth: 4
    BRAKE_REVERSE = 2,//wired: 2, bluetooth: 5
    QUIT = 7 //menu button on xbox controller, wired: 7, wireless: 11
};


int main()
{
    char host[] = "192.168.1.123";
    char port[] = "8888";
    int session = pigpio_start(host, port);

    std::cout << session << std::endl;

    if(session >= 0){
        //init gpios
        set_servo_pulsewidth(session, ESC_PIN, IDLE_MOTOR_SPEED);
        set_servo_pulsewidth(session, STEER_PIN, STEER_CENTER);

        Joystick joystick("/dev/input/js0");

        if (!joystick.isFound())
        {
            std::cout << "joystick not found." << std::endl;
            return 1;
        }

        bool si, ai, bi; //steer, accelerate, brake is initialized
        bool quit = false;
        while (!quit)
        {
            // Restrict rate
            usleep(500);
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
                                std::cout << "[STEER_LEFT]: " <<  l_percent * 100<< "%" << std::endl;
                                set_servo_pulsewidth(session, STEER_PIN, STEER_CENTER + l_percent * STEER_MAX_ANGLE);
                            }else if(event.value > AXIS_THRESHOLD){
                                //jobbra mennyire
                                double r_percent = (event.value + 0.0 - AXIS_THRESHOLD) / (JoystickEvent::MAX_AXES_VALUE - AXIS_THRESHOLD);
                                std::cout << "[STEER_RIGHT]: " <<  r_percent * 100 << "%" << std::endl;
                                set_servo_pulsewidth(session, STEER_PIN, STEER_CENTER - r_percent * STEER_MAX_ANGLE);
                            }else{
                                std::cout << "[STEER_CENTER]" << std::endl;
                                set_servo_pulsewidth(session, STEER_PIN, STEER_CENTER);
                            }
                            break;
                        case ACCELERATE:
                            // (-32767; 32767)
                            // (  0%  ; 100% )
                            if (!ai){ ai = true; break; } //for ignoring first, intialize values
                            if(event.value > JoystickEvent::MIN_AXES_VALUE + AXIS_THRESHOLD){
                                double a_percent = (event.value+0.0 + JoystickEvent::MAX_AXES_VALUE - AXIS_THRESHOLD) / (2*JoystickEvent::MAX_AXES_VALUE - AXIS_THRESHOLD );
                                std::cout << "[FORWARD]: " << a_percent * 100 << "%" << std::endl;
                                set_servo_pulsewidth(session, ESC_PIN, IDLE_MOTOR_SPEED + MAX_DIFF_SPEED * a_percent);

                                isacc = true;
                            }else
                                isacc = false;
                            break;
                        case BRAKE_REVERSE:
                            if (!bi){ bi = true; break; } //for ignoring first, intialize values
                            if(event.value > JoystickEvent::MIN_AXES_VALUE + AXIS_THRESHOLD){

                                double b_percent = (event.value+0.0 + JoystickEvent::MAX_AXES_VALUE - AXIS_THRESHOLD) / (2*JoystickEvent::MAX_AXES_VALUE - AXIS_THRESHOLD );
                                
                                std::cout << "[BACKWARDS]: " << b_percent * 100 << "%" << std::endl;
                                
                                set_servo_pulsewidth(session, ESC_PIN, IDLE_MOTOR_SPEED - MAX_DIFF_SPEED * b_percent);

                                isbrake = true;
                            }else{
                                isbrake = false;
                            }
                    }
                }
            }
            if(!isbrake && !isacc){
                set_servo_pulsewidth(session, ESC_PIN, IDLE_MOTOR_SPEED);
                //std::cout << "still" << std::endl;
            }
        }   
    }
    pigpio_stop(session);
    



    /* TEST FOR STEERING SERVO

    int n_it = 2;

    uint32_t from = 530;
    uint32_t to = 900;

    while(n_it-- > 0){
        for(uint32_t i = from; i <= to; i+=10){
            std::cout << "send: " << i << std::endl; 
            
            pgc.send_servo_pulsewidth(17, i);

            usleep(5000); //0.005 sec
        }
        usleep(100000);
        for(uint32_t i = to; i >= from; i-=10){
            std::cout << "send: " << i << std::endl; 
            
            pgc.send_servo_pulsewidth(17, i);

            usleep(5000); //0.005 sec
        }
    }
    */

    
    return 0;
}