#pragma once

#include <vector>
#include <algorithm>
#include <Arduino.h>
#include <FX.h>
#include <fcn_declare.h>

template <typename T>
T clamp(T value, T min, T max)
{
    return std::max(min, std::min(max, value));
}

template <typename T>
T incrementValue(T current, T amount, T max)
{
    if (max - current < amount)
    {
        return max;
    }
    return current + amount;
}

template <typename T>
T decrementValue(T current, T amount, T min)
{
    if (current - min <= amount)
    {
        return min;
    }
    return current - amount;
}

double mapToRange(
    double value,
    double inputMin,
    double inputMax,
    double outputMin,
    double outputMax)
{
    const auto inputRange = inputMax - inputMin;
    const auto outputRange = outputMax - outputMin;

    return ((value - inputMin) / inputRange) * outputRange + outputMin;
}

enum class ColorMode
{
    RGB,
    CCT
};

template <int PixelCount>
class Strip
{
    enum class ColorChangeDirection
    {
        Increase,
        Decrease
    };

    // define class Pixel that will contain index, color and requiredBrightness
    class Pixel
    {
    public:
        void setIndex(uint16_t index)
        {
            _index = index;
        }

        uint16_t getIndex() const
        {
            return _index;
        }

        byte getBrightness() const
        {
            return _brightness;
        }

        void setBrightness(byte requiredBrightness)
        {
            _brightness = requiredBrightness;
        }

    private:
        uint16_t _index;
        byte _brightness{255};
    };

public:
    Strip() = default;

    void setup(uint16_t startPixel, bool invert = false)
    {
        _invert = invert;
        for (uint16_t i = startPixel; i < startPixel + _pixels.size(); ++i)
        {
            _pixels.at(i - startPixel).setIndex(i);
        }
    }

    void turnOff()
    {
        _on = false;
        _pendingUpdate = true;
    }

    void turnOn()
    {
        _on = true;
        _pendingUpdate = true;

        auto t = readTemperature();
    }

    void toggleOnOff()
    {
        _on = !_on;
        _pendingUpdate = true;
    }

    void setColorMode(ColorMode colorMode)
    {
        if (!_on)
        {
            return;
        }
        _colorMode = colorMode;
        _pendingUpdate = true;
    }

    ColorMode getColorMode() const
    {
        return _colorMode;
    }

    void addPercentage(double amount)
    {
        if (!_on)
        {
            return;
        }

        auto newPercentage = clamp(_litLengthPercent + amount, 0.0, 100.0);
        if (newPercentage == _litLengthPercent)
        {
            return;
        }

        _pendingUpdate = true;
        _litLengthPercent = newPercentage;
    }

    void reducePercentage(double amount)
    {
        if (!_on)
        {
            return;
        }

        auto newPercentage = clamp(_litLengthPercent - amount, 0.0, 100.0);
        if (newPercentage == _litLengthPercent)
        {
            return;
        }

        _litLengthPercent = newPercentage;
        _pendingUpdate = true;
    }

    void raiseColor(byte amount)
    {
        if (!_on)
        {
            return;
        }

        if (_colorMode == ColorMode::CCT)
        {
            _requestedColorTemperature = clamp(_requestedColorTemperature + amount, 0, 255);
            _pendingUpdate = true;
        }
        else
        {
            _colorChangeDirection = ColorChangeDirection::Increase;
            _requestedColor += amount;
        }

        _pendingUpdate = true;
        // Serial.printf("Color Temperature: %d\n", _colorTemperature);
    }

    void lowerColor(byte amount)
    {
        if (!_on)
        {
            return;
        }

        if (_colorMode == ColorMode::CCT)
        {
            _requestedColorTemperature = clamp(_requestedColorTemperature - amount, 0, 255);
            _pendingUpdate = true;
        }
        else
        {
            _colorChangeDirection = ColorChangeDirection::Decrease;
            _requestedColor -= amount;
        }
        _pendingUpdate = true;
        // Serial.printf("Color Temperature: %d\n", _colorTemperature);
    }

    void updateStrip(WS2812FX &strip, bool force = false)
    {
        if (!_pendingUpdate && !force)
        {
            return;
        }

        _pendingUpdate = false;

        if (_colorMode == ColorMode::RGB && _currentColor != _requestedColor)
        {
            _currentColor = _colorChangeDirection == ColorChangeDirection::Decrease
                                ? decrementValue(_currentColor, byte{1}, _requestedColor)
                                : incrementValue(_currentColor, byte{1}, _requestedColor);
            _pendingUpdate = true;
        }
        else if (_colorMode == ColorMode::CCT && _colorTemperature != _requestedColorTemperature)
        {
            _colorTemperature = _colorChangeDirection == ColorChangeDirection::Decrease
                                    ? decrementValue(_colorTemperature, byte{1}, _requestedColorTemperature)
                                    : incrementValue(_colorTemperature, byte{1}, _requestedColorTemperature);
            const auto asKelvin = std::round(
                mapToRange(
                    _colorTemperature,
                    0,
                    255,
                    1000,
                    10000));
            colorKtoRGB(asKelvin, _cctColor);
            _pendingUpdate = true;
            // Serial.printf("Color Temperature: %d\n", _colorTemperature);
        }

        for (auto &pixel : _pixels)
        {
            auto requiredBrightness = _getPixelBrightness(pixel.getIndex());
            if (!_on)
            {
                requiredBrightness = 0;
            }
            const auto currentBrightness = pixel.getBrightness();

            if (currentBrightness != requiredBrightness)
            {
                pixel.setBrightness(currentBrightness > requiredBrightness
                                        ? decrementValue(currentBrightness, byte{4}, requiredBrightness)
                                        : incrementValue(currentBrightness, byte{4}, requiredBrightness));
                _pendingUpdate = true;
            }
            else if (!force)
            {
                continue;
            }

            const auto newBrightness = pixel.getBrightness();

            if (newBrightness != requiredBrightness)
            {
                _pendingUpdate = true;
            }

            if (_colorMode == ColorMode::RGB)
            {
                strip.setPixelColor(pixel.getIndex(), CHSV{_currentColor, 255, newBrightness});
            }
            else if (_colorMode == ColorMode::CCT)
            {
                auto newBrightnessPercentage = newBrightness / 255.0;
                strip.setPixelColor(
                    pixel.getIndex(),
                    RGBW32(_cctColor[0] * newBrightnessPercentage,
                           _cctColor[1] * newBrightnessPercentage,
                           _cctColor[2] * newBrightnessPercentage,
                           newBrightness));
            }
        }

        colorUpdated(CALL_MODE_DIRECT_CHANGE);
    }

private:
    byte _getPixelBrightness(uint16_t index) const
    {
        const auto firstIndex = _pixels.front().getIndex();
        const auto distance = std::floor(_pixels.size() * _litLengthPercent / 100 + firstIndex);

        if (_invert)
        {
            if (index >= distance)
            {
                return 255;
            }
            else if (distance - index > 0)
            {
                return std::floor(255 - (distance - index) * 25.5);
            }
            else
            {
                return 0;
            }
        }
        else
        {
            if (index <= distance)
            {
                return 255;
            }
            else if (index - distance > 0)
            {
                return std::floor(255 - (index - distance) * 25.5);
            }
            else
            {
                return 0;
            }
        }
    }

private:
    std::array<Pixel, PixelCount> _pixels;
    double _litLengthPercent{50};
    byte _currentColor{0};
    byte _requestedColor{0};
    byte _cctColor[4]{0, 0, 0, 0};
    ColorChangeDirection _colorChangeDirection{ColorChangeDirection::Increase};
    ColorMode _colorMode{ColorMode::CCT};
    uint8_t _colorTemperature{0};
    uint8_t _requestedColorTemperature{0};
    bool _pendingUpdate{true};
    bool _invert{false};
    bool _on{true};
};
