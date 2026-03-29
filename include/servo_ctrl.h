#pragma once
#include <Arduino.h>
#include <Servo.h>
#include "config.h"

class ServoController {
public:
    ServoController() : _currentDeg(0) {}

    void begin() {
        _servo.attach(SERVO_PWM_PIN, SERVO_MIN_US, SERVO_MAX_US);
        moveTo(0);
    }

    // Instantly command angle
    void moveTo(float deg) {
        deg = constrain(deg, 0.0f, 180.0f);
        _currentDeg = deg;
        _servo.write((int)deg);
    }

    // movement with steps and delay
    void smoothMoveTo(float targetDeg, int stepDeg = 1, int stepDelayMs = 15) {
        float start = _currentDeg;
        float diff  = targetDeg - start;
        int steps   = (int)(abs(diff) / stepDeg) + 1;
        for (int i = 1; i <= steps; i++) {
            float interp = start + diff * ((float)i / steps);
            moveTo(interp);
            delay(stepDelayMs);
        }
        moveTo(targetDeg);
    }

    float currentDeg() const { return _currentDeg; }

    void detach() { _servo.detach(); }

private:
    Servo _servo;
    float _currentDeg;
};
