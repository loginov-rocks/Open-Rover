#include <Arduino.h>

#include "BluetoothController.h"
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

BluetoothController bluetoothController(&Serial3, 115200, 9);
DriveController driveController(&frontLeftMotor, &frontRightMotor, &rearLeftMotor, &rearRightMotor);

const float JOYSTICK_DEAD_ZONE = 0.01;
const unsigned long BLUETOOTH_TIMEOUT_MS = 1000;
unsigned long lastBluetoothUpdate = 0;
const unsigned long SERIAL_OUTPUT_INTERVAL_MS = 300;
unsigned long lastSerialOutputTime = 0;

void handleBluetoothReceive(const char *message)
{
  Serial.print("Message received: \"");
  Serial.print(message);
  Serial.println("\"");
}

void handleBluetoothUpdate(float x, float y)
{
  Serial.print("Bluetooth Command: X = ");
  Serial.print(x, 3);
  Serial.print(", Y = ");
  Serial.print(y, 3);

  float joystickX = joystick.getX();
  float joystickY = joystick.getY();

  if (abs(joystickX) < JOYSTICK_DEAD_ZONE && abs(joystickY) < JOYSTICK_DEAD_ZONE)
  {
    driveController.drive(x, y);
    lastBluetoothUpdate = millis();
  }
}

void setup()
{
  Serial.begin(115200);

  Serial.println("Setting up joystick...");
  joystick.setup();

  Serial.println("Setting up differential drive...");
  driveController.setup();

  Serial.println("Setting up Bluetooth...");
  bluetoothController.setup();
  bluetoothController.onReceive(handleBluetoothReceive);
  bluetoothController.onUpdate(handleBluetoothUpdate);
}

void loop()
{
  bluetoothController.loop();

  float x = joystick.getX();
  float y = joystick.getY();

  if (abs(x) >= JOYSTICK_DEAD_ZONE || abs(y) >= JOYSTICK_DEAD_ZONE)
  {
    driveController.drive(x, y);
    lastBluetoothUpdate = 0;
  }
  else if (lastBluetoothUpdate == 0 || (millis() - lastBluetoothUpdate) >= BLUETOOTH_TIMEOUT_MS)
  {
    driveController.drive(x, y);
  }

  unsigned long currentTime = millis();
  if (currentTime - lastSerialOutputTime >= SERIAL_OUTPUT_INTERVAL_MS)
  {
    lastSerialOutputTime = currentTime;

    Serial.print("[JOYSTICK] X: ");
    Serial.print(x, 3);
    Serial.print(" | Y: ");
    Serial.print(y, 3);
    Serial.print(" | Button: ");
    Serial.print(joystick.isPressed() ? "Pressed" : "Released");
    Serial.print(" | Bluetooth: ");
    Serial.println(bluetoothController.isConnected() ? "Connected" : "Disconnected");
  }
}
