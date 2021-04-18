//
// Created by Bence Deli on 2021. 04. 18..
//

#include "RemoteRC.h"

#include <utility>

void RemoteRC::steer(double percentage) const {
    if(percentage == 0.0){
        //set the steering servo to center
        set_servo_pulsewidth(session, steer_pin, center_steer_servo);
        return;
    }
    set_servo_pulsewidth(session, steer_pin, center_steer_servo - percentage * max_angle_steer_servo);

}

RemoteRC::RemoteRC(Controller _controller, const std::string &host, const std::string &port): controller(std::move(_controller)) {
    //connect to RPi, save session id to a variable
    session = pigpio_start(host.c_str(), port.c_str());
    std::cout << session;

    //set default values for servo, motor
    set_servo_pulsewidth(session, esc_pin, idle_motor_speed);
    set_servo_pulsewidth(session, steer_pin, center_steer_servo);
}

RemoteRC::~RemoteRC() {
    pigpio_stop(session);
}

void RemoteRC::motor_speed(double percentage) const {
    if(percentage == 0.0){
        set_servo_pulsewidth(session, esc_pin, idle_motor_speed);
        return;
    }
    set_servo_pulsewidth(session, esc_pin, idle_motor_speed + max_diff_motor_speed * percentage);
}

bool RemoteRC::connected() const {
    return session >= 0;
}

bool RemoteRC::processRawInput(JoystickEvent &joystickEvent) {
    reset_states();
    //if we got the quitbutton input we return false
    if(joystickEvent.isButton() && joystickEvent.number == controller.getQuitButton() && joystickEvent.value) return false;
    if(joystickEvent.isAxis()){
        if(joystickEvent.number == controller.getSteerId()){
            if (!si){ si = true; return true; } //for ignoring first, intialize values
            if(abs(joystickEvent.value) >= Controller::getAxisTreshold()){
                std::cout << "[RAW_STEER_INPUT]: " << joystickEvent.value << " - ";
                double steering_angle = controller.getSteer().input_as_percentage(joystickEvent.value);
                std::cout << "[STEERING]: " << steering_angle * 100 << std::endl;
                steer(steering_angle);
            }else{
                std::cout << "[STEER_CENTER]" << std::endl;
                steer();
            }
            //acceleration input
        }else if(joystickEvent.number == controller.getAccelerateId()){
            if (!ai){ ai = true; return true; } //for ignoring first, intialize values
            if(abs(joystickEvent.value) >= Controller::getAxisTreshold()){
                double acc_percentage = controller.getAcc().input_as_percentage(joystickEvent.value);
                std::cout << "[FORWARD]: " << acc_percentage * 100 << "%" << std::endl;
                motor_speed(acc_percentage);
                is_accelerating = true;
            }else
                is_accelerating = false;
        }else if(joystickEvent.number == controller.getBrakeId()){
            if (!bi){ bi = true; return true; } //for ignoring first, intialize values
            if((abs(joystickEvent.value) >= Controller::getAxisTreshold())){

                double b_percentage = -controller.getBrakeReverse().input_as_percentage(joystickEvent.value);
                std::cout << "[BACKWARDS]: " << b_percentage * 100 << "%" << std::endl;
                motor_speed(b_percentage);

                is_braking = true;
            }else{
                is_braking = false;
            }
        }
    }
    if(is_no_motor_input()){
        motor_speed();
    }
    return true;
}

void RemoteRC::reset_states() {
    is_accelerating = false; is_braking = false;
}

bool RemoteRC::is_no_motor_input() {
    return !is_braking && !is_accelerating;
}
