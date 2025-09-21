#ifndef DATAPACK_H_
#define DATAPACK_H_

#include <cstddef>
#include <cstdint>
#include <array>

#define PIN_SD 3, 8, 4, 9, 5, 10, 6, 11
#define PIN_SMR 12
#define PIN_SCLK 7
#define PIN_SOE_B 13
#define PIN_SS_B 14
#define PIN_SPGM_B 15

#define PIN_BUS {PIN_SD, PIN_SMR, PIN_SCLK, PIN_SOE_B, PIN_SS_B, PIN_SPGM_B}

#define SMR 8
#define SCLK 9
#define SOE_B 10
#define SS_B 11
#define SPGM_B 12

#define HIGH 1
#define LOW 0

#define PACK_COMMS_LOCATION 0x2000

typedef int Pin;
typedef std::array<Pin, 13> DataBus;

enum Edge {
    NONE = 0,
    RISING = 1,
    FALLING = -1
};

enum PinDirection {
    UNSET = 0,
    INPUT = 1,
    OUTPUT = 2
};

class Datapack {
    public:
        Datapack(DataBus pins);

        static Datapack* the();

        void step();
        void readState();
        char getDataValue();
        void setDataValue(char value);
        size_t getAddress();
        Edge getStateChange(size_t index);
        void loadCode(const char* code, size_t length);
        void reportInfo();

    private:
        DataBus _pins;
        DataBus _pinDirections;
        DataBus _state;
        DataBus _prevState;

        bool _input = false;
        bool _forceWrite = false;
        char _lastValue = 0;

        static Datapack* _instance;
        std::array<char, 32 * 1024> _data;
        unsigned char _mainCounter = 0;
        unsigned char _pageCounter = 0;

        bool _addressChanged = false;
        bool _commsBufferWrittenTo = false;
};

#endif