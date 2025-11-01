#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>

class Joystick
{
private:
  int vrxPin;
  int vryPin;
  int swPin;
  int centerX;
  int centerY;
  // Default 10-bit resolution.
  int analogMax = 1023;

public:
  Joystick(int vrx, int vry, int sw)
  {
    vrxPin = vrx;
    vryPin = vry;
    swPin = sw;
    centerX = analogMax / 2;
    centerY = analogMax / 2;
  }

  void setup()
  {
    pinMode(swPin, INPUT_PULLUP);
    calibrate();
  }

  /*
   * Set the analog resolution (max ADC value).
   * Should match the hardware ADC configuration (e.g., 1023 for 10-bit, 4095 for 12-bit).
   * Best practice: call before setup() if changing from default.
   */
  void setResolution(int maxValue)
  {
    analogMax = maxValue;
  }

  /*
   * Map X to -1.0 ... 0.0 ... +1.0 (normalized float).
   * Left = negative, Right = positive.
   * Uses symmetric scaling for proportional sensitivity on both sides.
   */
  float getX()
  {
    int raw = analogRead(vrxPin);

    // Use the smaller range for symmetric scaling.
    int leftRange = centerX;
    int rightRange = analogMax - centerX;
    int symmetricRange = min(leftRange, rightRange);

    if (raw < centerX)
    {
      // Left side: normalize to -1.0 to 0.0.
      float value = (centerX - raw) / (float)symmetricRange;
      return -constrain(value, 0.0, 1.0);
    }
    else if (raw > centerX)
    {
      // Right side: normalize to 0.0 to 1.0.
      float value = (raw - centerX) / (float)symmetricRange;
      return constrain(value, 0.0, 1.0);
    }
    else
    {
      return 0.0;
    }
  }

  /*
   * Map Y to -1.0 ... 0.0 ... +1.0 (normalized float).
   * Up = positive, Down = negative (inverted).
   * Uses symmetric scaling for proportional sensitivity on both sides.
   */
  float getY()
  {
    int raw = analogRead(vryPin);

    // Use the smaller range for symmetric scaling.
    int upRange = centerY;
    int downRange = analogMax - centerY;
    int symmetricRange = min(upRange, downRange);

    if (raw < centerY)
    {
      // Joystick pushed up (raw value decreases): normalize to 0.0 to 1.0.
      float value = (centerY - raw) / (float)symmetricRange;
      return constrain(value, 0.0, 1.0);
    }
    else if (raw > centerY)
    {
      // Joystick pushed down (raw value increases): normalize to 0.0 to -1.0.
      float value = (raw - centerY) / (float)symmetricRange;
      return -constrain(value, 0.0, 1.0);
    }
    else
    {
      return 0.0;
    }
  }

  bool isPressed()
  {
    return digitalRead(swPin) == LOW;
  }

private:
  void calibrate(int samples = 50)
  {
    Serial.println("=== JOYSTICK CALIBRATION ===");
    Serial.print("Analog resolution: ");
    Serial.print(analogMax);
    Serial.println(" (max value)");
    Serial.println("Calibrating center position...");

    long sumX = 0;
    long sumY = 0;

    for (int i = 0; i < samples; i++)
    {
      sumX += analogRead(vrxPin);
      sumY += analogRead(vryPin);
      delay(20);
    }

    centerX = sumX / samples;
    centerY = sumY / samples;

    Serial.println("Calibration complete!");
    Serial.print("Center X: ");
    Serial.println(centerX);
    Serial.print("Center Y: ");
    Serial.println(centerY);
    Serial.println("=========================\n");
  }
};

#endif
