#ifndef BLUETOOTH_CONTROLLER_H
#define BLUETOOTH_CONTROLLER_H

#include <Arduino.h>
#include <ArduinoJson.h>

class BluetoothController
{
public:
  using ReceiveHandler = void (*)(const char *);
  using UpdateHandler = void (*)(float x, float y);

  BluetoothController(HardwareSerial *ser = &Serial3, long baud = 115200, int statePinNum = -1)
  {
    serial = ser;
    statePin = statePinNum;
    baudRate = baud;
    receiveBuffer = "";
    isReceiveBufferComplete = false;
    onReceiveCallback = nullptr;
    onUpdateCallback = nullptr;
  }

  void setup()
  {
    serial->begin(baudRate);

    if (statePin >= 0)
    {
      pinMode(statePin, INPUT);
    }

    receiveBuffer = "";
    isReceiveBufferComplete = false;
  }

  void onReceive(ReceiveHandler callback)
  {
    onReceiveCallback = callback;
  }

  void onUpdate(UpdateHandler callback)
  {
    onUpdateCallback = callback;
  }

  bool isConnected()
  {
    if (statePin >= 0)
    {
      return digitalRead(statePin) == HIGH;
    }
    return false;
  }

  void loop()
  {
    while (serial->available())
    {
      char c = serial->read();

      if (c != '\n' && c != '\r')
      {
        receiveBuffer += c;
      }
      else if (receiveBuffer.length() > 0)
      {
        isReceiveBufferComplete = true;
      }
    }

    if (isReceiveBufferComplete)
    {
      bool isCommand = false;

      /*
       * Echo back only when device is connected to avoid infinite loop.
       * When not connected, echoing goes to the module itself which responds
       * with "ERROR", triggering another echo, creating an infinite loop.
       */
      if (isConnected() && receiveBuffer != "TX=")
      {
        if (executeCommand(receiveBuffer))
        {
          serial->print("OK");
          isCommand = true;
        }
        else
        {
          serial->print("UNKNOWN: ");
          serial->print(receiveBuffer);
        }
        serial->print("\r\n");
      }

      // Trigger callback only for non-command messages.
      if (!isCommand && onReceiveCallback)
      {
        onReceiveCallback(receiveBuffer.c_str());
      }

      receiveBuffer = "";
      isReceiveBufferComplete = false;
    }
  }

private:
  /*
   * Parse and execute command. Returns true if valid, false otherwise.
   * Both x and y are optional, but at least one is required.
   */
  bool executeCommand(String message)
  {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error)
    {
      return false;
    }

    bool hasX = doc["x"].is<float>();
    bool hasY = doc["y"].is<float>();

    if (!hasX && !hasY)
    {
      return false;
    }

    // Skip value extraction if no callback set.
    if (!onUpdateCallback)
    {
      return true;
    }

    float x = 0.0;
    float y = 0.0;

    if (hasX)
    {
      x = doc["x"];
      x = constrain(x, -1.0, 1.0);
    }

    if (hasY)
    {
      y = doc["y"];
      y = constrain(y, -1.0, 1.0);
    }

    onUpdateCallback(x, y);

    return true;
  }

  HardwareSerial *serial;
  // STATE pin for hardware connection detection.
  int statePin;
  long baudRate;
  String receiveBuffer;
  bool isReceiveBufferComplete;
  ReceiveHandler onReceiveCallback;
  UpdateHandler onUpdateCallback;
};

#endif
