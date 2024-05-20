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
    Strip(uint16_t startPixel)
    {
        for (uint16_t i = startPixel; i < startPixel + _pixels.size(); ++i)
        {
            _pixels.at(i).setIndex(i);
        }
    }

    void addPercentage(double amount)
    {
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
        _colorChangeDirection = ColorChangeDirection::Increase;
        _requestedColor += amount;
        _pendingUpdate = true;
    }

    void lowerColor(byte amount)
    {
        _colorChangeDirection = ColorChangeDirection::Decrease;
        _requestedColor -= amount;
        _pendingUpdate = true;
    }

    void updateStrip(WS2812FX &strip, bool force = false)
    {
        if (!_pendingUpdate && !force)
        {
            return;
        }

        _pendingUpdate = false;

        if (_currentColor != _requestedColor)
        {
            _currentColor = _colorChangeDirection == ColorChangeDirection::Decrease
                                ? decrementValue(_currentColor, byte{1}, _requestedColor)
                                : incrementValue(_currentColor, byte{1}, _requestedColor);
        }

        if (_currentColor != _requestedColor)
        {
            _pendingUpdate = true;
        }

        for (auto &pixel : _pixels)
        {
            const auto requiredBrightness = _getPixelBrightness(pixel.getIndex());
            const auto currentBrightness = pixel.getBrightness();

            if (currentBrightness != requiredBrightness)
            {
                pixel.setBrightness(currentBrightness > requiredBrightness
                                        ? decrementValue(currentBrightness, byte{6}, requiredBrightness)
                                        : incrementValue(currentBrightness, byte{6}, requiredBrightness));
            }
            else if (!force)
            {
                continue;
            }

            auto newBrightness = pixel.getBrightness();

            if (newBrightness != requiredBrightness)
            {
                _pendingUpdate = true;
            }

            strip.setPixelColor(pixel.getIndex(), CHSV{_currentColor, 255, newBrightness});
        }

        colorUpdated(CALL_MODE_DIRECT_CHANGE);
    }

private:
    byte _getPixelBrightness(uint16_t index) const
    {
        const auto firstIndex = _pixels.front().getIndex();
        const auto distance = std::floor(_pixels.size() * _litLengthPercent / 100 + firstIndex);

        // return 255 if index is less or equal to distance, fade next 10 percent of pixels if index is greater than distance and pixels are available
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

private:
    std::array<Pixel, PixelCount> _pixels;
    uint16_t _brightness{100};
    double _litLengthPercent{50};
    byte _currentColor{0};
    byte _requestedColor{0};
    ColorChangeDirection _colorChangeDirection{ColorChangeDirection::Increase};
    bool _pendingUpdate{true};
};
