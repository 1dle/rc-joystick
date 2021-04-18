#include <pigpiod_if2.h>
#include <iostream>
#include <unistd.h>
#include <joystick.hh>

#define IDLE_MOTOR_SPEED 1500
#define MAX_DIFF_SPEED 200 //400 is dangerous for testing i guess
#define ESC_PIN 27

#define STEER_PIN 17
#define STEER_CENTER 700
#define STEER_MAX_ANGLE 180

struct axis {
    int id;
    int min;
    int max;
    int def;
    const double treshhold = 0.01 * JoystickEvent::MAX_AXES_VALUE;
    double input_as_percentage(int input) const{
        double percentage = (0.0 + input - def) / (max - def + 0.0);
        if(percentage > 1.0) return 1.0;
        if(percentage < - 1.0) return -1.0;
        return percentage;
    };
};

struct controller {
    axis steer;
    axis acc;
    axis brake_reverse;
    int quit_button;
};

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
    constexpr controller con = {
            { 0, -32767, 32767, 0 }, //steer //10% jobbra és balra
            { 4, -32767, 32767, -32767 }, //acc pedal
            { 5, 32767, 32767, -32767 }, //brake pedal
            11 //quit btn
    };


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
            usleep(800);
            bool isacc, isbrake; //in current iteration are we accelerating? or breaking?

            // Attempt to sample an event from the joystick
            JoystickEvent event;
            if (joystick.sample(&event))
            {
                if (event.isButton())
                {
                    if(event.number == con.quit_button && event.value){
                        quit = true;
                    }
                }
                if (event.isAxis())
                {
                    //which axises?
                    switch(event.number){
                        case con.steer.id:
                            if (!si){ si = true; break; } //for ignoring first, intialize values
                            if(abs(event.value) >= con.steer.treshhold){
                                std::cout << "[RAW_STEER_INPUT]: " << event.value << " - ";
                                double steering_angle = con.steer.input_as_percentage(event.value);
                                std::cout << "[STEERING]: " << steering_angle * 100 << std::endl;
                                set_servo_pulsewidth(session, STEER_PIN, STEER_CENTER - steering_angle * STEER_MAX_ANGLE);
                            }else{
                                std::cout << "[STEER_CENTER]" << std::endl;
                                set_servo_pulsewidth(session, STEER_PIN, STEER_CENTER);
                            }
                            break;
                        case con.acc.id:
                            if (!ai){ ai = true; break; } //for ignoring first, intialize values
                            if(abs(event.value) >= con.acc.treshhold ){
                                double acc_percentage = con.acc.input_as_percentage(event.value);
                                std::cout << "[FORWARD]: " << acc_percentage * 100 << "%" << std::endl;
                                set_servo_pulsewidth(session, ESC_PIN, IDLE_MOTOR_SPEED + MAX_DIFF_SPEED * acc_percentage);
                                isacc = true;
                            }else
                                isacc = false;
                            break;
                        case con.brake_reverse.id:
                            if (!bi){ bi = true; break; } //for ignoring first, intialize values
                            if((abs(event.value) >= con.brake_reverse.treshhold)){

                                double b_percent = con.acc.input_as_percentage(event.value);
                                std::cout << "[BACKWARDS]: " << b_percent * 100 << "%" << std::endl;
                                set_servo_pulsewidth(session, ESC_PIN, IDLE_MOTOR_SPEED - MAX_DIFF_SPEED * b_percent);

                                isbrake = true;
                            }else{
                                isbrake = false;
                            }
                            break;
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
    
    return 0;
}