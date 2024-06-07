#pragma once

#include "wled.h"

#include "StripControlWrapper.h"

class BedlightDoubleRottaryEncoder : public Usermod
{

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


    _strip1Wrapper.setup(4, 2, 3, 0, true);
    _strip2Wrapper.setup(7, 6, 5, 60, true);

    // load settings
    // _strip1.setColorMode(_strip1ControlMode);

    bri = 20;

    Serial.println("Finished setup BedlightDoubleRotaryEncoder.");
  }

  void loop()
  {
    if (!_enabled)
    {
      return;
    }

    _strip1Wrapper.loop();
    _strip2Wrapper.loop();

    if (strip.isUpdating())
      return;

    static auto lastMillis = millis();
    if (millis() - lastMillis < 30)
    {
      return;
    }

    lastMillis = millis();

    _strip1Wrapper.updateStrip();
    _strip2Wrapper.updateStrip();
    strip.show();
  }
  

  void handleOverlayDraw()
  {
    if (!_enabled)
    {
      return;
    }

    _strip1Wrapper.updateStrip(true);
    // TODO fix invert
    _strip2Wrapper.updateStrip(true);
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
  void _cleanup()
  {
  }

private:
  StripControlWrapper<60> _strip1Wrapper{};
  StripControlWrapper<60> _strip2Wrapper{};

  unsigned long lastMillis{millis()};

  bool _enabled{true};
};
