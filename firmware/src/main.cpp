#include <Arduino.h>

#include "Joystick.cpp"
#include "Motor.cpp"

Joystick joystick(A0, A1, 2, 255);

// Motor PWM pins: 5-8 chosen for:
// - Sequential ordering (easy to track)
// - PWM support with 31kHz capability (eliminates motor noise)
// - Timer 3 (pins 2,3,5) and Timer 4 (pins 6,7,8) - don't affect millis()/delay()
// - Avoids pin 4 (Timer 0) which controls system timing functions
Motor frontLeftMotor(5, 22, 23);
Motor rearRightMotor(6, 25, 24);
Motor rearLeftMotor(7, 26, 27);
Motor frontRightMotor(8, 29, 28);

void setup()
{
  Serial.begin(9600);

  joystick.begin();

  frontLeftMotor.begin();
  frontRightMotor.begin();
  rearLeftMotor.begin();
  rearRightMotor.begin();
}

void loop()
{
  int yValue = joystick.getY();

  frontLeftMotor.setSpeed(yValue);
  frontRightMotor.setSpeed(yValue);
  rearLeftMotor.setSpeed(yValue);
  rearRightMotor.setSpeed(yValue);

  Serial.print("X: ");
  Serial.print(joystick.getX());
  Serial.print(" | Y: ");
  Serial.print(yValue);
  Serial.print(" | Motor Speed: ");
  Serial.print(yValue);
  Serial.print(" | Button: ");
  Serial.println(joystick.isPressed() ? "PRESSED" : "RELEASED");

  delay(100);
}
