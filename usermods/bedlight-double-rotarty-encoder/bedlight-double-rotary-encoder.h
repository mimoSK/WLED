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
public:
  BedlightDoubleRottaryEncoder()
  {
    // strip.purgeSegments(true);
    // strip.appendSegment(s1);
    // strip.appendSegment(s2);
  }

  void setup()
  {
    Serial.println("Setup BedlightDoubleRotaryEncoder...");
    _initRotaryEncoders();
    _initButtons();
    bri = 255;

    Serial.println("Finished setup BedlightDoubleRotaryEncoder.");
  }

  void loop()
  {
    _enc1.loop();
    _enc2.loop();
    _enc1Button.loop();

    if (strip.isUpdating())
      return;

    static auto lastMillis = millis();
    if (millis() - lastMillis < 50)
    {
      return;
    }
    lastMillis = millis();

    _strip1.updateStrip(strip);
    strip.show();
  }

  void handleOverlayDraw()
  {
    _strip1.updateStrip(strip, true);
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
          // Serial.println("Enc1 button clicked");
        },
        []() {
          // Serial.println("Enc1 button pressed and held");
        });

    // _enc2Button.setup(
    //     _pinEnc2Btn,
    //     []() {
    //       Serial.println("Enc2 button pressed");
    //     },
    //     []() {
    //       Serial.println("Enc2 button released");
    //     },
    //     []() {
    //       Serial.println("Enc2 button clicked");
    //     },
    //     []() {
    //       Serial.println("Enc2 button pressed and held");
    //     });
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
      // Serial.println("Enc1 Right");
      // _strip1.addPercentage(5);
      _strip1.raiseColor(5);
    });
    _enc1.setLeftRotationHandler([this](ESPRotary &r) {
      // Serial.println("Enc1 Left");
      // _strip1.reducePercentage(5);
      _strip1.lowerColor(5);
    });

    _enc2.begin(_pinEnc2A, _pinEnc2B, 4);
    _enc2.setRightRotationHandler([](ESPRotary &r) {
      // Serial.println("Enc2 Right");
    });
    _enc2.setLeftRotationHandler([](ESPRotary &r) {
      // Serial.println("Enc2 Left");
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
  Strip<60> _strip1{0};
  // Button _enc2Button;
  unsigned long lastMillis{millis()};

  int8_t _pinEnc1A = 2;
  int8_t _pinEnc1B = 3;
  int8_t _pinEnc1Btn = 4;
  int8_t _pinEnc2A = 5;
  int8_t _pinEnc2B = 6;
  int8_t _pinEnc2Btn = 7;
  bool _initFailed{false};
};
