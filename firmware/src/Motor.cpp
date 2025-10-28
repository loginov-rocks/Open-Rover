#include <Arduino.h>

class Motor
{
private:
  int enPin;  // PWM pin for speed control
  int in1Pin; // Direction pin 1
  int in2Pin; // Direction pin 2

public:
  Motor(int en, int in1, int in2)
  {
    enPin = en;
    in1Pin = in1;
    in2Pin = in2;
  }

  void begin()
  {
    pinMode(enPin, OUTPUT);
    pinMode(in1Pin, OUTPUT);
    pinMode(in2Pin, OUTPUT);

    // Arduino Mega PWM frequency configuration:
    // Default PWM is ~490Hz which causes audible motor whine at low speeds
    // Setting prescaler to 0x01 increases frequency to ~31kHz (above hearing range)
    // This eliminates motor noise while maintaining smooth speed control

    if (enPin == 2 || enPin == 3 || enPin == 5)
    {
      // Timer 3 (pins 2, 3, 5) - set to ~31kHz
      TCCR3B = (TCCR3B & 0xF8) | 0x01;
    }
    else if (enPin == 6 || enPin == 7 || enPin == 8)
    {
      // Timer 4 (pins 6, 7, 8) - set to ~31kHz
      TCCR4B = (TCCR4B & 0xF8) | 0x01;
    }
    // Note: Avoid Timer 0 (pin 4) - it controls millis(), delay(), micros()
    // Modifying it breaks timing functions used throughout Arduino code

    stop();
  }

  // Set motor speed: -255 (full backward) to +255 (full forward)
  void setSpeed(int speed)
  {
    if (speed > 255)
      speed = 255;
    if (speed < -255)
      speed = -255;

    if (speed > 0)
    {
      // Forward direction
      digitalWrite(in1Pin, HIGH);
      digitalWrite(in2Pin, LOW);
      analogWrite(enPin, speed);
    }
    else if (speed < 0)
    {
      // Backward direction
      digitalWrite(in1Pin, LOW);
      digitalWrite(in2Pin, HIGH);
      analogWrite(enPin, -speed); // Use absolute value for PWM
    }
    else
    {
      // Stop
      stop();
    }
  }

  void stop()
  {
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
    analogWrite(enPin, 0);
  }

  void forward(int speed)
  {
    if (speed < 0)
      speed = 0;
    if (speed > 255)
      speed = 255;
    digitalWrite(in1Pin, HIGH);
    digitalWrite(in2Pin, LOW);
    analogWrite(enPin, speed);
  }

  void backward(int speed)
  {
    if (speed < 0)
      speed = 0;
    if (speed > 255)
      speed = 255;
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, HIGH);
    analogWrite(enPin, speed);
  }
};
