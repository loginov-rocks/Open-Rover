#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

class Motor
{
private:
  // PWM pin for speed control.
  int enPin;
  // Direction pin 1.
  int in1Pin;
  // Direction pin 2.
  int in2Pin;
  // Default 8-bit PWM resolution.
  int pwmMax = 255;

public:
  Motor(int en, int in1, int in2)
  {
    enPin = en;
    in1Pin = in1;
    in2Pin = in2;
  }

  ~Motor()
  {
    stop();
  }

  void setup()
  {
    pinMode(enPin, OUTPUT);
    pinMode(in1Pin, OUTPUT);
    pinMode(in2Pin, OUTPUT);

    /*
     * Arduino Mega PWM frequency configuration:
     * Default PWM is ~490Hz which causes audible motor whine at low speeds.
     * Setting prescaler to 0x01 increases frequency to ~31kHz (above hearing range).
     * This eliminates motor noise while maintaining smooth speed control.
     * Note: Avoid Timer 0 (pin 4) - it controls millis(), delay(), micros().
     * Modifying it breaks timing functions used throughout Arduino code.
     */
    if (enPin == 2 || enPin == 3 || enPin == 5)
    {
      // Timer 3 (pins 2, 3, 5) - set to ~31kHz.
      TCCR3B = (TCCR3B & 0xF8) | 0x01;
    }
    else if (enPin == 6 || enPin == 7 || enPin == 8)
    {
      // Timer 4 (pins 6, 7, 8) - set to ~31kHz.
      TCCR4B = (TCCR4B & 0xF8) | 0x01;
    }

    stop();
  }

  // Set the PWM resolution (max PWM value).
  void setResolution(int maxValue)
  {
    pwmMax = maxValue;
  }

  /*
   * Set motor speed using normalized float.
   * speed: -1.0 (full backward) to 0.0 (stop) to +1.0 (full forward).
   */
  void setSpeed(float speed)
  {
    // Constrain to valid range.
    speed = constrain(speed, -1.0, 1.0);

    // Scale to PWM resolution.
    int pwmValue = abs(speed) * pwmMax;

    if (speed > 0.0)
    {
      // Forward direction.
      digitalWrite(in1Pin, HIGH);
      digitalWrite(in2Pin, LOW);
      analogWrite(enPin, pwmValue);
    }
    else if (speed < 0.0)
    {
      // Backward direction.
      digitalWrite(in1Pin, LOW);
      digitalWrite(in2Pin, HIGH);
      analogWrite(enPin, pwmValue);
    }
    else
    {
      stop();
    }
  }

private:
  void stop()
  {
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
    analogWrite(enPin, 0);
  }
};

#endif
