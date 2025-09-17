#include <stdio.h>
#include "pico/stdlib.h"

#include "datapack.h"

Datapack* Datapack::_instance = nullptr;

Datapack::Datapack(DataBus pins) {
    _pins = pins;

    _data.fill(0xFF);

    for (size_t i = 0; i < _pins.size(); i++) {
        gpio_init(_pins[i]);
    }
}

Datapack* Datapack::the() {
    if (!_instance) {
        _instance = new Datapack(PIN_BUS);
    }

    return _instance;
}

void Datapack::step() {
    readState();

    if (_state[SS_B] == HIGH) {
        return;
    }

    if (_state[SOE_B] == LOW) {
        setDataValue(_data[getAddress()]);
    }

    if (_state[SMR] == HIGH) {
        _mainCounter = 0;
        _pageCounter = 0;

        return;
    }

    if (getStateChange(SCLK) == Edge::FALLING) {
        _mainCounter += 2;
    }

    if (getStateChange(SPGM_B) == Edge::FALLING) {
        _pageCounter++;

        if (_pageCounter >= _data.size() >> 8) {
            _pageCounter = 0;
        }
    }

    if (_state[SOE_B] == HIGH) {
        _data[getAddress()] = getDataValue();
    }
}

void Datapack::readState() {
    _prevState = _state;
    _input = !!(sio_hw->gpio_in & (1 << _pins[SOE_B]));

    if (!_input) {
        for (size_t i = 0; i < 8; i++) {
            if (_pinDirections[i] != PinDirection::OUTPUT) {
                gpio_set_dir(_pins[i], GPIO_OUT);
            }

            _pinDirections[i] = PinDirection::OUTPUT;
        }

        _forceWrite = true;
    }

    for (size_t i = _input ? 0 : 8; i < _pins.size(); i++) {
        if (_pinDirections[i] != PinDirection::INPUT) {
            gpio_init(_pins[i]);
            gpio_set_dir(_pins[i], GPIO_IN);
        }

        _pinDirections[i] = PinDirection::INPUT;

        _state[i] = !!(sio_hw->gpio_in & (1 << _pins[i]));
    }
}

char Datapack::getDataValue() {
    char value = 0;

    for (size_t i = 0; i < 8; i++) {
        value |= _state[i] << i;
    }

    return value;
}

void Datapack::setDataValue(char value) {
    if (!_forceWrite && value == _lastValue) {
        return;
    }

    _forceWrite = false;
    _lastValue = value;

    unsigned long pinState = 0;

    for (size_t i = 0; i < 8; i++) {
        if (!(value & (1 << i))) {
            continue;
        }

        pinState |= (1 << _pins[i]);
    }

    sio_hw->gpio_clr = ~(pinState);
    sio_hw->gpio_out = pinState;
}

size_t Datapack::getAddress() {
    return (_pageCounter << 8) | _mainCounter | _state[SCLK];
}

Edge Datapack::getStateChange(size_t index) {
    if (_state[index] == _prevState[index]) {
        return Edge::NONE;
    }

    return _state[index] ? Edge::RISING : Edge::FALLING;
}

void Datapack::loadCode(const char* code, size_t length) {
    std::copy(code, code + length, std::begin(_data));
}