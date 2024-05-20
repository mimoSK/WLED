#pragma once

#include <functional>
#include <esp32-hal.h>

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
        std::function<void()> onClicked,
        std::function<void()> onPressAndHold,
        bool pullUp = true)
    {
        _onPressed = onPressed;
        _onReleased = onReleased;
        _onClicked = onClicked;
        _onPressAndHold = onPressAndHold;
        pinMode(pin, pullUp ? INPUT_PULLUP : INPUT_PULLDOWN);
        _pin = pin;
        _lastDebounceTime = millis();
    }

    void loop()
    {
        auto newState = digitalRead(_pin);
        _validatePressAndHold();
        if (newState == static_cast<int>(_lastState))
        {
            return;
        }

        if (millis() - _lastDebounceTime < 20)
        {
            return;
        }

        _lastDebounceTime = millis();

        _validate(static_cast<State>(newState));
        // Serial.printf("Button %d %s\n", _pin, _state == State::Pressed ? "pressed" : "released");
    }

private:
    void _validate(State newState)
    {
        if (newState == State::Pressed)
        {
            _lastPressMillis = millis();
            if (_onPressed)
            {
                _onPressed();
            }
        }
        else if (_onReleased)
        {
            _onReleased();
        }

        if (_onClicked &&
            newState == State::Released &&
            _lastState == State::Pressed &&
            !_held)
        {
            _onClicked();
        }

        if (newState == State::Released)
        {
            _held = false;
        }

        _lastState = newState;
    }

    void _validatePressAndHold()
    {
        if (_onPressAndHold &&
            _lastState == State::Pressed &&
            !_held &&
            millis() - _lastPressMillis > _pressAndHoldIntervalMs)
        {
            _held = true;
            _onPressAndHold();
        };
    }

private:
    std::function<void()> _onPressed;
    std::function<void()> _onReleased;
    std::function<void()> _onClicked;
    std::function<void()> _onPressAndHold;
    bool _held{false};
    uint16_t _pressAndHoldIntervalMs = 250;
    unsigned long _lastPressMillis{millis()};
    unsigned long _lastDebounceTime{millis()};
    State _lastState{State::Released};
    int8_t _pin;
};
