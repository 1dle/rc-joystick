//
// Created by Bence Deli on 2021. 04. 18..
//
#pragma once
#include "joystick.hh"


class Controller {

public:
    class Axis{
        int id;
        int min;
        int max;
        int def;

    public:
        Axis(int id, int min, int max, int def): id(id), min(min), max(max), def(def){}

        //copy constructor
        Axis(const Axis& axis): id(axis.getId()), min(axis.getMin()), max(axis.getMax()), def(axis.getDef()){}

        double input_as_percentage(int input) const{
            double percentage = (0.0 + input - def) / (max - def + 0.0);
            if(percentage > 1.0) return 1.0;
            if(percentage < - 1.0) return -1.0;
            return percentage;
        };
        int getId() const {
            return id;
        }

        int getMin() const {
            return min;
        }

        int getMax() const {
            return max;
        }

        int getDef() const {
            return def;
        }
    };
private:
    static double axis_treshold;

    Axis steer;
public:
    const Axis &getSteer() const;

    const Axis &getAcc() const;

    const Axis &getBrakeReverse() const;

private:
    Axis acc;
    Axis brake_reverse;
    int quit_button;
public:
    Controller(const Axis& axis_steer, const Axis& axis_accelerate, const Axis& axis_brake, int quit_button): steer(axis_steer), acc(axis_accelerate),
                                                                                                              brake_reverse(axis_brake), quit_button(quit_button){}
    int getSteerId() const;
    int getAccelerateId() const;
    int getBrakeId() const;

    int getQuitButton() const;

    static double getAxisTreshold();
};
