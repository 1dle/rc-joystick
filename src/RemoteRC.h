//
// Created by Bence Deli on 2021. 04. 18..
//
#pragma once

#include <string>
#include <pigpiod_if2.h>
#include "Controller.h"

class RemoteRC {
    /*
#define IDLE_MOTOR_SPEED 1500
#define MAX_DIFF_SPEED 200 //400 is dangerous for testing i guess
#define ESC_PIN 27

#define STEER_PIN 17
#define STEER_CENTER 700
#define STEER_MAX_ANGLE 180
     */
    const int idle_motor_speed = 1500;
    const int max_diff_motor_speed = 200;

    const int center_steer_servo = 700;
    const int max_angle_steer_servo = 180;

    //esc, sterr connected to:
    const int esc_pin = 27;
    const int steer_pin = 17;

    //connection related
    int session;

    //state of remote rc car (accelerating/decelerating)
    bool is_accelerating;
    bool is_braking;

    //the controller of the rc car
    Controller *controller;
public:
    //also establish the connection to the remote rpi
    RemoteRC(Controller *controller, const std::string& host = "192.168.1.123", const std::string& port = "8888");

    //close the connection with the rpi
    ~RemoteRC();

    /**
     * Steer the car with the given percentage. -100% -> +100%
     * steering left: (-180; 0) steering right: (0; +180)
     * center if the param is 0
     * @param percentage
     * @returns false if the connection lost
     */
    bool steer(double percentage = 0.0) const;

    /**
     * Sets the speed of the main motor on the RC car
     * accelerate (0-100%), decelerate/brake: (0; -100%)
     * @param percentage
     * @returns false if the connection lost
     */
    bool motor_speed(double percentage = 0.0) const;

    //Check if the connection is established.
    bool connected() const;

    /**
     * Processing input from the joystick/controller
     * @param joystickEvent
     * @return true if processedd successfully, false if the user pressed the quit button
     */
    bool processRawInput(JoystickEvent& joystickEvent);

private:
    void reset_states();
    bool is_no_motor_input();

};