#pragma once

#include <ESPRotary.h>
#include <math.h>
#include "wled.h"
#include <functional>
#include <memory>
#include <algorithm>

#include "Button.h"
#include "Strip.h"

class BedlightDoubleRottaryEncoder : public Usermod
{
  enum class StripControlMode
  {
    Color = 0,
    CCT,
    Brightness
  };

  StripControlMode toggleMode(StripControlMode mode)
  {
    if (mode == StripControlMode::Brightness)
    {
      return StripControlMode::Color;
    }
    return static_cast<StripControlMode>(static_cast<int>(mode) + 1);
  }

public:
  BedlightDoubleRottaryEncoder()
  {
  }

  void setup()
  {
    if (!_enabled)
    {
      return;
    }

    Serial.println("Setup BedlightDoubleRotaryEncoder...");
    _initRotaryEncoders();
    _initButtons();
    switch (_strip1ControlMode)
    {
    case StripControlMode::Color:
      _strip1.setColorMode(ColorMode::RGB);
      break;
    case StripControlMode::CCT:
      _strip1.setColorMode(ColorMode::CCT);
      break;
    }

    bri = 20;

    Serial.println("Finished setup BedlightDoubleRotaryEncoder.");
  }

  void loop()
  {
    if (!_enabled)
    {
      return;
    }
    _enc1.loop();
    _enc2.loop();
    _enc1Button.loop();
    _enc2Button.loop();

    if (strip.isUpdating())
      return;

    static auto lastMillis = millis();
    if (millis() - lastMillis < 50)
    {
      return;
    }
    lastMillis = millis();

    _strip1.updateStrip(strip);
    _strip2.updateStrip(strip);
    strip.show();
  }

  void handleOverlayDraw()
  {
    if (!_enabled)
    {
      return;
    }
    _strip1.updateStrip(strip, true);
    _strip2.updateStrip(strip, true);
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
  void _initButtons()
  {
    _enc1Button.setup(
        _pinEnc1Btn,
        []() {
          // Serial.println("EEnc1 button pressed");
        },
        []() {
          // Serial.println("Enc1 button released");
        },
        []() {
          toggleOnOff();
          // Serial.println("Enc1 button clicked");
        },
        [this]() {
          _strip1ControlMode = toggleMode(_strip1ControlMode);
          // Serial.println("Enc1 button pressed and held");
          switch (_strip1ControlMode)
          {
          case StripControlMode::Color:
            _strip1.setColorMode(ColorMode::RGB);
            break;
          case StripControlMode::CCT:
            _strip1.setColorMode(ColorMode::CCT);
            break;
          }
        });

    _enc2Button.setup(
        _pinEnc2Btn,
        []() {
          // Serial.println("Enc2 button pressed");
        },
        []() {
          // Serial.println("Enc2 button released");
        },
        []() {
          // Serial.println("Enc2 button clicked");
        },
        [this]() {
            _strip2ControlMode = toggleMode(_strip2ControlMode);
          // Serial.println("Enc2 button pressed and held");
          switch (_strip2ControlMode)
          {
          case StripControlMode::Color:
            _strip2.setColorMode(ColorMode::RGB);
            break;
          case StripControlMode::CCT:
            _strip2.setColorMode(ColorMode::CCT);
            break;
          }
        });
  }
  void _initRotaryEncoders()
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
      Serial.println("Could not allocate pins for BedlightDoubleRotaryEncoder.");
      _initFailed = true;
      return;
    }

    _enc1.begin(_pinEnc1A, _pinEnc1B, 4);
    _enc1.setRightRotationHandler([this](ESPRotary &r) {
      // Serial.println("Enc1 right rotation");
      switch (_strip1ControlMode)
      {
      case StripControlMode::Color:
      case StripControlMode::CCT:
        _strip1.raiseColor(15);
        break;
      case StripControlMode::Brightness:
        _strip1.addPercentage(5);
        break;
      }
    });

    _enc1.setLeftRotationHandler([this](ESPRotary &r) {
      // Serial.println("Enc1 left rotation");
      switch (_strip1ControlMode)
      {
      case StripControlMode::Color:
      case StripControlMode::CCT:
        _strip1.lowerColor(15);
        break;
      case StripControlMode::Brightness:
        _strip1.reducePercentage(5);
        break;
      }
    });

    _enc2.begin(_pinEnc2A, _pinEnc2B, 4);
    _enc2.setLeftRotationHandler([this](ESPRotary &r) {
      switch (_strip2ControlMode)
      {
      case StripControlMode::Color:
      case StripControlMode::CCT:
        _strip2.raiseColor(15);
        break;
      case StripControlMode::Brightness:
        _strip2.addPercentage(5);
        break;
      }
    });
    _enc2.setRightRotationHandler([this](ESPRotary &r) {
      switch (_strip2ControlMode)
      {
      case StripControlMode::Color:
      case StripControlMode::CCT:
        _strip2.lowerColor(15);
        break;
      case StripControlMode::Brightness:
        _strip2.reducePercentage(5);
        break;
      }
    });

    Serial.println("Initialized BedlightDoubleRotaryEncoder.");
  }

  void _cleanup()
  {
    pinManager.deallocatePin(_pinEnc1A, PinOwner::UM_BedlightDoubleRotaryEncoder);
    pinManager.deallocatePin(_pinEnc1B, PinOwner::UM_BedlightDoubleRotaryEncoder);
    pinManager.deallocatePin(_pinEnc1Btn, PinOwner::UM_BedlightDoubleRotaryEncoder);
    pinManager.deallocatePin(_pinEnc2A, PinOwner::UM_BedlightDoubleRotaryEncoder);
    pinManager.deallocatePin(_pinEnc2B, PinOwner::UM_BedlightDoubleRotaryEncoder);
    pinManager.deallocatePin(_pinEnc2Btn, PinOwner::UM_BedlightDoubleRotaryEncoder);
  }

private:
  ESPRotary _enc1;
  ESPRotary _enc2;
  Button _enc1Button;
  Button _enc2Button;
  Strip<60> _strip1{0, true};
  Strip<60> _strip2{60, true};
  StripControlMode _strip1ControlMode{StripControlMode::CCT};
  StripControlMode _strip2ControlMode{StripControlMode::CCT};
  unsigned long lastMillis{millis()};

  int8_t _pinEnc1A = 2;
  int8_t _pinEnc1B = 3;
  int8_t _pinEnc1Btn = 4;
  int8_t _pinEnc2A = 5;
  int8_t _pinEnc2B = 6;
  int8_t _pinEnc2Btn = 7;
  bool _initFailed{false};
  bool _enabled{true};
};
