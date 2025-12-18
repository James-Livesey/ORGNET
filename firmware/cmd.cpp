#include "cmd.h"

CommsBuffer stringBuffer(ReturnCode returnCode, std::string data);
uint16_t fromWord(std::string word);
std::string toWord(uint16_t value);

std::vector<WifiAccessPoint> currentAccessPoints;

std::vector<Command> cmd::availableCommands = {
    Command("WISTAT", [](CommandArguments args) -> CommsBuffer {
        static int variant = 1;
        std::string response;

        switch (variant) {
            case 0: response = std::string("\x00\x00", 2); break;
            case 1: response = std::string("\x01\x00", 2) + "Hello SSID"; break;
            case 2: response = std::string("\x01\x01", 2) + "SSID With A Really Long Name"; break;
            case 3: response = std::string("\x01\x02", 2) + "Test Network"; break;
        }

        variant = (variant + 1) % 4;

        return stringBuffer(ReturnCode::SUCCESS, response);
    }),
    Command("WISCAN", [](CommandArguments args) -> CommsBuffer {
        currentAccessPoints = wifi::accessPoints;

        return stringBuffer(ReturnCode::SUCCESS, toWord(currentAccessPoints.size()));
    }),
    Command("WILIST", [](CommandArguments args) -> CommsBuffer {
        size_t index = fromWord(std::string {args[0], args[1]});

        if (index >= currentAccessPoints.size()) {
            return stringBuffer(ReturnCode::SUCCESS, "");
        }

        WifiAccessPoint ap = currentAccessPoints[index];

        return stringBuffer(ReturnCode::SUCCESS, std::string {ap.getStrength(), (char)ap.getSecurityMode()} + ap.getSsid());
    })
};

Command::Command(std::string name, CommandCallback callback) {
    _name = name;
    _callback = callback;
}

const std::string Command::getName() {
    return _name;
}

CommsBuffer Command::call(CommandArguments args) {
    return _callback(args);
}

bool Command::matches(CommsBuffer* buffer) {
    return (
        (*buffer)[0] >= _name.length() + 1 &&
        std::equal(buffer->begin() + 1, buffer->begin() + _name.length() + 1, _name.begin()) &&
        (*buffer)[_name.length() + 1] == ' '
    );
}

void cmd::processCommand(CommsBuffer* buffer) {
    for (Command& command : cmd::availableCommands) {
        if (command.matches(buffer)) {
            size_t argsOffset = command.getName().length() + 2;
            CommandArguments args(buffer->data() + argsOffset, buffer->size() - argsOffset);
            CommsBuffer result = command.call(args);

            std::copy(result.begin(), result.end(), buffer->begin());
            break;
        }
    }
}

CommsBuffer stringBuffer(ReturnCode returnCode, std::string data) {
    CommsBuffer buffer;

    buffer.fill(0x00);

    buffer[0] = data.length() + 1;
    buffer[1] = returnCode;

    std::copy(data.begin(), data.end(), buffer.begin() + 2);

    return buffer;
}

uint16_t fromWord(std::string word) {
    return (word[0] << 8) | word[1];
}

std::string toWord(uint16_t value) {
    return {(char)((value >> 8) & 0xFF), (char)(value & 0xFF)};
}