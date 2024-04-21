#pragma once

#include "ESPRotary.h"
#include <math.h>
#include "wled.h"
#include <functional>

class Button
{
  enum class State : uint8_t
  {
    Pressed = LOW,
    Released = HIGH
  };

public:
  Button() = default;

  // add setup function that does the same as the constructor
  void setup(
      int8_t pin,
      std::function<void()> onPressed,
      std::function<void()> onReleased,
      bool pullUp = true)
  {
    _onPressed = onPressed;
    _onReleased = onReleased;
    pinMode(pin, pullUp ? INPUT_PULLUP : INPUT_PULLDOWN);
    _pin = pin;
    _lastDebounceTime = millis();
  }

  void loop()
  {
    auto newState = digitalRead(_pin);
    if (newState == static_cast<uint8_t>(_state))
    {
      return;
    }

    if (millis() - _lastDebounceTime < 50)
    {
      return;
    }

    _lastDebounceTime = millis();

    _state = newState == LOW ? State::Pressed : State::Released;

    Serial.printf("Button %d %s\n", _pin, _state == State::Pressed ? "pressed" : "released");

    if (_state == State::Pressed && _onPressed)
    {
      _onPressed();
    }
    else if (_onReleased)
    {
      _onReleased();
    }
  }

private:
  std::function<void()> _onPressed;
  std::function<void()> _onReleased;
  unsigned long _lastDebounceTime{0};
  State _state{State::Released};
  int8_t _pin;
};

class BedlightDoubleRottaryEncoder : public Usermod
{
public:
  void setup()
  {
    Serial.println("Setup BedlightDoubleRotaryEncoder...");
    _initRotaryEncoder();
    _enc1Button.setup(
        _pinEnc1Btn,
        []() {
          Serial.println("Enc1 button pressed");
        },
        []() {
          Serial.println("Enc1 button released");
        });

    _enc2Button.setup(
        _pinEnc2Btn,
        []() {
          Serial.println("Enc2 button pressed");
        },
        []() {
          Serial.println("Enc2 button released");
        });

    Serial.println("Finished setup BedlightDoubleRotaryEncoder.");
  }

  void loop()
  {
    _enc1.loop();
    _enc2.loop();
    _enc1Button.loop();
    _enc2Button.loop();
  }

  void addToConfig(JsonObject &root)
  {
  }

  /**
     * readFromConfig() is called before setup() to populate properties from values stored in cfg.json
     *
     * The function should return true if configuration was successfully loaded or false if there was no configuration.
     */
  bool readFromConfig(JsonObject &root)
  {
    return true;
  }

  /*
      * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
      * This could be used in the future for the system to determine whether your usermod is installed.
      */
  uint16_t getId()
  {
    return USERMOD_ID_BEDLIGHT_DOUBLE_ROTARTY_ENCODER;
  }

private:
  void _initRotaryEncoder()
  {
    //print via serial
    Serial.println("Initializing BedlightDoubleRotaryEncoder...");

    PinManagerPinType pins[6] = {
        {_pinEnc1A, false},
        {_pinEnc1B, false},
        {_pinEnc1Btn, false},
        {_pinEnc2A, false},
        {_pinEnc2B, false},
        {_pinEnc2Btn, false}};

    if (!pinManager.allocateMultiplePins(pins, 6, PinOwner::UM_BedlightDoubleRotaryEncoder))
    {
      _pinEnc1A = -1;
      Serial.println("Could not allocate pins for BedlightDoubleRotaryEncoder.");
      _cleanup();
      return;
    }

    _enc1.begin(_pinEnc1A, _pinEnc1B, 4);
    _enc1.setRightRotationHandler([](ESPRotary &r) {
      Serial.println("Enc1 Right");
    });
    _enc1.setLeftRotationHandler([](ESPRotary &r) {
      Serial.println("Enc1 Left");
    });

    _enc2.begin(_pinEnc2A, _pinEnc2B, 4);
    _enc2.setRightRotationHandler([](ESPRotary &r) {
      Serial.println("Enc2 Right");
    });
    _enc2.setLeftRotationHandler([](ESPRotary &r) {
      Serial.println("Enc2 Left");
    });

    Serial.println("Initialized BedlightDoubleRotaryEncoder.");
  }

  void _cleanup()
  {
    // Only deallocate pins if we allocated them ;)
    if (_pinEnc1A != -1)
    {
      pinManager.deallocatePin(_pinEnc1A, PinOwner::UM_BedlightDoubleRotaryEncoder);
      pinManager.deallocatePin(_pinEnc1B, PinOwner::UM_BedlightDoubleRotaryEncoder);
      pinManager.deallocatePin(_pinEnc1Btn, PinOwner::UM_BedlightDoubleRotaryEncoder);
    }
  }

private:
  ESPRotary _enc1;
  ESPRotary _enc2;
  Button _enc1Button;
  Button _enc2Button;
  int8_t _pinEnc1A = 2;
  int8_t _pinEnc1B = 3;
  int8_t _pinEnc1Btn = 4;
  int8_t _pinEnc2A = 5;
  int8_t _pinEnc2B = 6;
  int8_t _pinEnc2Btn = 7;
};
// byte BedlightDoubleRottaryEncoder::currentPos = 5;
