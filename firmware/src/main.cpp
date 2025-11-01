#include <Arduino.h>

#include "DriveController.h"
#include "Joystick.h"
#include "Motor.h"

Joystick joystick(A0, A1, 22);

/*
 * Motors connected through L298N diagonally.
 * Motor PWM pins: 5-8 chosen for:
 * - Sequential ordering (easy to track).
 * - PWM support with 31kHz capability (eliminates motor noise).
 * - Timer 3 (pins 2,3,5) and Timer 4 (pins 6,7,8) - don't affect millis()/delay().
 * - Avoids pin 4 (Timer 0) which controls system timing functions.
 */
Motor frontLeftMotor(8, 30, 32);
Motor rearRightMotor(7, 36, 34);
Motor rearLeftMotor(6, 38, 40);
Motor frontRightMotor(5, 44, 42);

DriveController driveController(&frontLeftMotor, &frontRightMotor, &rearLeftMotor, &rearRightMotor);

void setup()
{
  Serial.begin(9600);

  Serial.println("Setting up joystick...");
  joystick.setup();

  Serial.println("Setting up differential drive...");
  driveController.setup();
}

void loop()
{
  float x = joystick.getX();
  float y = joystick.getY();

  driveController.drive(x, y);

  Serial.print("X: ");
  Serial.print(x, 3);
  Serial.print(" | Y: ");
  Serial.print(y, 3);
  Serial.print(" | Button: ");
  Serial.println(joystick.isPressed() ? "PRESSED" : "RELEASED");

  delay(100);
}
