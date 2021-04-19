//
// Created by Bence Deli on 2021. 04. 19..
//

#pragma once
#include "Controller.h"

class ControllerFactory{
public:
    enum CONTROLLER_TYPES{
        XBOX_WIRED,
        XBOX_WIRELESS,
        G29
    };
    
    static Controller* create(CONTROLLER_TYPES type){
        switch (type) {
            case(CONTROLLER_TYPES::XBOX_WIRELESS):
                return new Controller(Controller::Axis(0,-32767,32767, 0),
                        Controller::Axis(4,-32767, 32767, -32767),
                        Controller::Axis(5, -32767, 32767, -32767),
                        11);
            case(CONTROLLER_TYPES::G29):
                return new Controller(
                        Controller::Axis(0, -3276, 3276, 0),
                        Controller::Axis(2, 32767, -32767, 32767),
                        Controller::Axis(3, 32767, -32767, 32767),
                        23);
            case(CONTROLLER_TYPES::XBOX_WIRED):
                //only the key id-s differ from the wireless
                return new Controller(Controller::Axis(0,-32767,32767, 0),
                                      Controller::Axis(5,-32767, 32767, -32767),
                                      Controller::Axis(2, -32767, 32767, -32767),
                                      7);

            default: return nullptr;

        }
    }
};