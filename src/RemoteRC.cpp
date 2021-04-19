//
// Created by Bence Deli on 2021. 04. 18..
//

#include "RemoteRC.h"

bool RemoteRC::steer(double percentage) const {
    //if status is negative that means the connection lost, or any other pigpio error occured
    int status;
    if(percentage == 0.0){
        //set the steering servo to center
        status = set_servo_pulsewidth(session, steer_pin, center_steer_servo);
        return status >= 0;
    }
    status = set_servo_pulsewidth(session, steer_pin, center_steer_servo - percentage * max_angle_steer_servo);
    return status >= 0;

}
bool RemoteRC::motor_speed(double percentage) const {
    //if status is negative that means the connection lost, or any other pigpio error occured
    int status = 0;
    if(percentage == 0.0){
        set_servo_pulsewidth(session, esc_pin, idle_motor_speed);
        return status >= 0;
    }
    set_servo_pulsewidth(session, esc_pin, idle_motor_speed + max_diff_motor_speed * percentage);
    return status >= 0;
}

RemoteRC::RemoteRC(Controller* _controller, const std::string &host, const std::string &port): controller(_controller) {
    //connect to RPi, save session id to a variable
    session = pigpio_start(host.c_str(), port.c_str());
    std::cout << session;

    //set default values for servo, motor
    set_servo_pulsewidth(session, esc_pin, idle_motor_speed);
    set_servo_pulsewidth(session, steer_pin, center_steer_servo);
}

RemoteRC::~RemoteRC() {
    //also reset stuff
    motor_speed();
    steer();
    pigpio_stop(session);
    delete controller;
}

bool RemoteRC::connected() const {
    return session >= 0;
}

bool RemoteRC::processRawInput(JoystickEvent &joystickEvent) {
    reset_states();
    bool status = true;
    //if we got the quitbutton input we return false
    if(joystickEvent.isButton() && joystickEvent.number == controller->getQuitButton() && joystickEvent.value){
        reset_states();
        return false;
    }
    if(joystickEvent.isAxis()){
        if(joystickEvent.number == controller->getSteerId()){
            if(abs(joystickEvent.value) >= Controller::getAxisTreshold()){
                std::cout << "[RAW_STEER_INPUT]: " << joystickEvent.value << " - ";
                double steering_angle = controller->getSteer().input_as_percentage(joystickEvent.value);
                std::cout << "[STEERING]: " << steering_angle * 100 << std::endl;
                status = steer(steering_angle);
            }else{
                std::cout << "[STEER_CENTER]" << std::endl;
                status = steer();
            }
            //acceleration input
        }else if(joystickEvent.number == controller->getAccelerateId()){
            if(abs(joystickEvent.value) >= Controller::getAxisTreshold()){
                double acc_percentage = controller->getAcc().input_as_percentage(joystickEvent.value);
                std::cout << "[FORWARD]: " << acc_percentage * 100 << "%" << std::endl;
                status = motor_speed(acc_percentage);
                is_accelerating = true;
            }else
                is_accelerating = false;
        }else if(joystickEvent.number == controller->getBrakeId()){
            if((abs(joystickEvent.value) >= Controller::getAxisTreshold())){

                double b_percentage = -controller->getBrakeReverse().input_as_percentage(joystickEvent.value);
                std::cout << "[BACKWARDS]: " << b_percentage * 100 << "%" << std::endl;
                status = motor_speed(b_percentage);

                is_braking = true;
            }else{
                is_braking = false;
            }
        }
    }
    if(is_no_motor_input()){
        status = motor_speed();
    }
    return status;
}

void RemoteRC::reset_states() {
    is_accelerating = false; is_braking = false;
}

bool RemoteRC::is_no_motor_input() {
    return !is_braking && !is_accelerating;
}
