#include <Arduino.h>

class Joystick
{
private:
  int vrxPin;
  int vryPin;
  int swPin;
  int centerX;
  int centerY;
  int maxValue;

public:
  Joystick(int vrx, int vry, int sw, int maxVal = 255)
  {
    vrxPin = vrx;
    vryPin = vry;
    swPin = sw;
    centerX = 512;
    centerY = 512;
    maxValue = maxVal;
  }

  void begin()
  {
    pinMode(swPin, INPUT_PULLUP);
    calibrate();
  }

  void calibrate(int samples = 50)
  {
    Serial.println("=== JOYSTICK CALIBRATION ===");
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

  // Map X to -maxValue ... 0 ... +maxValue
  // Left = negative, Right = positive
  int getX()
  {
    int raw = analogRead(vrxPin);

    if (raw < centerX)
    {
      // Left side: map to negative values
      return map(raw, 0, centerX, -maxValue, 0);
    }
    else if (raw > centerX)
    {
      // Right side: map to positive values
      return map(raw, centerX, 1023, 0, maxValue);
    }
    else
    {
      return 0;
    }
  }

  // Map Y to -maxValue ... 0 ... +maxValue (INVERTED)
  // Up = positive, Down = negative
  int getY()
  {
    int raw = analogRead(vryPin);

    if (raw < centerY)
    {
      // Joystick pushed up (raw value decreases): map to positive values
      return map(raw, 0, centerY, maxValue, 0);
    }
    else if (raw > centerY)
    {
      // Joystick pushed down (raw value increases): map to negative values
      return map(raw, centerY, 1023, 0, -maxValue);
    }
    else
    {
      return 0;
    }
  }

  bool isPressed()
  {
    return digitalRead(swPin) == LOW;
  }

  int getCenterX()
  {
    return centerX;
  }

  int getCenterY()
  {
    return centerY;
  }

  void setMaxValue(int maxVal)
  {
    maxValue = maxVal;
  }

  int getMaxValue()
  {
    return maxValue;
  }
};
