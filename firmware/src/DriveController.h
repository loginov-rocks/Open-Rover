#ifndef DRIVE_CONTROLLER_H
#define DRIVE_CONTROLLER_H

#include <Arduino.h>

#include "Motor.h"

class DriveController
{
private:
  Motor *frontLeft;
  Motor *frontRight;
  Motor *rearLeft;
  Motor *rearRight;

public:
  DriveController(Motor *fl, Motor *fr, Motor *rl, Motor *rr)
  {
    frontLeft = fl;
    frontRight = fr;
    rearLeft = rl;
    rearRight = rr;
  }

  void setup()
  {
    frontLeft->setup();
    frontRight->setup();
    rearLeft->setup();
    rearRight->setup();
  }

  /*
   * Tank steering control.
   * x: steering input (-1.0 to +1.0, negative = left, positive = right).
   * y: throttle input (-1.0 to +1.0, negative = backward, positive = forward).
   */
  void drive(float x, float y)
  {
    /*
     * Calculate left and right side speeds.
     * Positive steering (right) reduces left side, increases right side.
     * Negative steering (left) increases left side, reduces right side.
     */
    float leftSpeed = y + x;
    float rightSpeed = y - x;

    // Constrain to valid range.
    leftSpeed = constrain(leftSpeed, -1.0, 1.0);
    rightSpeed = constrain(rightSpeed, -1.0, 1.0);

    leftSpeed = mapInputToMotor(leftSpeed);
    rightSpeed = mapInputToMotor(rightSpeed);

    // Apply speeds to motors.
    frontLeft->setSpeed(leftSpeed);
    rearLeft->setSpeed(leftSpeed);
    frontRight->setSpeed(rightSpeed);
    rearRight->setSpeed(rightSpeed);
  }

private:
  // Boost low-end values since motors do not really moves until ~50% of PWM range.
  float mapInputToMotor(float input)
  {
    float sign = (input >= 0) ? 1.0 : -1.0;
    float magnitude = fabs(input);
    float curved = pow(magnitude, 0.5);

    return sign * curved;
  }
};

#endif
