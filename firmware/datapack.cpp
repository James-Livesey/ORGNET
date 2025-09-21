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
        _addressChanged = true;

        return;
    }

    if (getStateChange(SCLK) == Edge::FALLING) {
        _mainCounter += 2;
        _addressChanged = true;
    }

    if (getStateChange(SPGM_B) == Edge::FALLING) {
        _pageCounter++;

        if (_pageCounter >= _data.size() >> 8) {
            _pageCounter = 0;
        }

        _addressChanged = true;
    }

    if (_state[SOE_B] == HIGH) {
        size_t address = getAddress();

        _data[address] = getDataValue();

        if (address == PACK_COMMS_LOCATION) {
            _commsBufferRemainingLength = _data[address];
        } else if (
            _commsBufferRemainingLength != 0x00 &&
            _commsBufferRemainingLength != 0xFF &&
            address >= PACK_COMMS_LOCATION + 0x01 &&
            address <= PACK_COMMS_LOCATION + 0xFF
        ) {
            _commsBufferRemainingLength--;
        }
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

    size_t spacerRecordLength = PACK_COMMS_LOCATION - length - 8;

    // Spacer record
    _data[length++] = 0x02;
    _data[length++] = 0x80;
    _data[length++] = (spacerRecordLength & 0xFF00) >> 8;
    _data[length++] = spacerRecordLength & 0xFF;
    length += spacerRecordLength;

    _data[length++] = 0x02;
    _data[length++] = 0x80;
    _data[length++] = 0x00;
    _data[length++] = 0xFF;

    _data[length++] = 0x7F; // Indicate that comms record is present
}

bool Datapack::hasAvailableCommand() {
    bool result = _commsBufferRemainingLength == 0;

    if (result) {
        _commsBufferRemainingLength = -1;
    }

    return result;
}

CommsBuffer Datapack::getCommsBuffer() {
    CommsBuffer result;

    std::copy(_data.begin() + PACK_COMMS_LOCATION, _data.begin() + PACK_COMMS_LOCATION + 256, result.begin());

    return result;
}

void Datapack::setCommsBuffer(CommsBuffer* buffer) {
    _data[PACK_COMMS_LOCATION] = 0x00;

    std::copy(buffer->begin() + 1, buffer->end(), _data.begin() + PACK_COMMS_LOCATION + 1);

    _data[PACK_COMMS_LOCATION] = (*buffer)[0];
}