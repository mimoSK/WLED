#pragma once

#include "wled.h"

#include "StripControlWrapper.h"
#include <driver/temp_sensor.h>

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
    temp_sensor_config_t temp_sensor = TSENS_CONFIG_DEFAULT();
    temp_sensor.dac_offset = TSENS_DAC_L2; // TSENS_DAC_L2 is default; L4(-40°C ~ 20°C), L2(-10°C ~ 80°C), L1(20°C ~ 100°C), L0(50°C ~ 125°C)
    temp_sensor_set_config(temp_sensor);
    temp_sensor_start();

    Serial.println("Setup BedlightDoubleRotaryEncoder...");

    _strip1Wrapper.setup(4, 2, 3, 0, true);
    _strip2Wrapper.setup(7, 5, 6, 60, true);

    // load settings
    // _strip1.setColorMode(_strip1ControlMode);

    Serial.println("Finished setup BedlightDoubleRotaryEncoder.");
  }

  void loop()
  {
    // static auto tempMillis = millis();
    // if (millis() - tempMillis > 1000)
    // {
    //   float result = 0;
    //   temp_sensor_read_celsius(&result);
    //   Serial.println("Temp: " + String(result));
    //   tempMillis = millis();
    // }

    if (!_enabled)
    {
      return;
    }

    _strip1Wrapper.loop();
    _strip2Wrapper.loop();

    if (strip.isUpdating())
      return;

    // static auto lastMillis = millis();
    // if (millis() - lastMillis < 30)
    // {
    //   return;
    // }

    lastMillis = millis();
    strip.fill(0);
    // strip.show();

    _strip1Wrapper.updateStrip();
    _strip2Wrapper.updateStrip();
    _strip1Wrapper.updateStrip(true);
    _strip2Wrapper.updateStrip(true);
    strip.show();
  }

  void handleOverlayDraw()
  {
    if (!_enabled)
    {
      return;
    }

    // static auto lastMillis = millis();
    // if (millis() - lastMillis < 50)
    // {
    //   return;
    // }

    // lastMillis = millis();

    // Serial.println("drawing...");

    // _strip1Wrapper.updateStrip(true);
    // TODO fix invert
    // _strip2Wrapper.updateStrip(true);
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

  bool _enabled{false};
};
