//
// Created by Bence Deli on 2021. 04. 18..
//

#include "Controller.h"

//static member initialization
double Controller::axis_treshold = 0.005 * JoystickEvent::MAX_AXES_VALUE;
double Controller::getAxisTreshold() {
    return axis_treshold;
}


int Controller::getAccelerateId() const {
    return acc.getId();
}

int Controller::getSteerId() const {
    return steer.getId();
}

int Controller::getBrakeId() const {
    return brake_reverse.getId();
}

int Controller::getQuitButton() const {
    return quit_button;
}

const Controller::Axis &Controller::getSteer() const {
    return steer;
}

const Controller::Axis &Controller::getAcc() const {
    return acc;
}

const Controller::Axis &Controller::getBrakeReverse() const {
    return brake_reverse;
}
