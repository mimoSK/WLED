#pragma once

#include "Strip.h"
#include "ESPRotary.h"
#include "Button.h"

template <int N>
class StripControlWrapper
{
    ColorMode toggleMode(ColorMode mode)
    {
        if (mode == ColorMode::CCT)
        {
            return ColorMode::RGB;
        }
        return static_cast<ColorMode>(static_cast<int>(mode) + 1);
    }

public:
    StripControlWrapper()
    {
    }

    void loop()
    {
        _enc.loop();
        _btn.loop();

        if (_colorChange && millis() - _colorChangeMillis > 3000)
        {
            _colorChange = false;
        }
    }

    void updateStrip(bool forceDraw = false)
    {
        if (forceDraw)
        {
            _strip.refreshPixels();
        }
        else
        {
            _strip.updateStrip(strip, forceDraw);
        }
    }

    void setup(int8_t btnPin, int8_t encPinA, int8_t encPinB, int16_t stripStartIdx, bool invert)
    {
        _strip.setup(stripStartIdx, invert);

        _btn.setup(
            btnPin,
            []() {
                Serial.println("Enc1 button pressed");
            },
            []() {
                Serial.println("Enc1 button released");
            },
            [this]() {
                Serial.println("Enc1 button clicked");
                if (_colorChange)
                {
                    _colorChangeMillis = millis();
                    _strip.setColorMode(toggleMode(_strip.getColorMode()));
                }
                else
                {
                    _strip.toggleOnOff();
                }
            },
            [this]() {
                _colorChange = !_colorChange;
                if (_colorChange)
                {
                    _colorChangeMillis = millis();
                }
                Serial.println("Enc1 button pressed and held");
            });

        _enc.begin(encPinA, encPinB, 4);
        _enc.setRightRotationHandler([this](ESPRotary &r) {
            Serial.println("Enc1 right rotation");
            if (_colorChange)
            {
                _colorChangeMillis = millis();
                _strip.raiseColor(15);
            }
            else
            {
                _strip.addPercentage(5);
            }
        });

        _enc.setLeftRotationHandler([this](ESPRotary &r) {
            Serial.println("Enc1 left rotation");
            if (_colorChange)
            {
                _colorChangeMillis = millis();
                _strip.lowerColor(15);
            }
            else
            {
                _strip.reducePercentage(5);
            }
        });
    }

private:
    ESPRotary _enc;
    Button _btn;
    Strip<N> _strip;
    bool _colorChange{false};
    unsigned long _colorChangeMillis{millis()};
};
