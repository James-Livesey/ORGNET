#include "cmd.h"

std::vector<Command> availableCommands = {
    Command("WISTAT", []() -> CommsBuffer {
        static int variant = 1;
        std::string response;

        switch (variant) {
            case 0: response = std::string("\x00\x00", 2); break;
            case 1: response = std::string("\x01\x00" "Hello SSID", 12); break;
            case 2: response = std::string("\x01\x01" "SSID With A Really Long Name", 30); break;
            case 3: response = std::string("\x01\x02" "Test Network", 14); break;
        }

        variant = (variant + 1) % 4;

        return stringBuffer(ReturnCode::SUCCESS, response);
    })
};

Command::Command(std::string name, CommandCallback callback) {
    _name = name;
    _callback = callback;
}

CommsBuffer Command::call() {
    return _callback();
}

bool Command::matches(CommsBuffer* buffer) {
    return (
        (*buffer)[0] >= _name.length() + 1 &&
        std::equal(buffer->begin() + 1, buffer->begin() + _name.length() + 1, _name.begin()) &&
        (*buffer)[_name.length() + 1] == ' '
    );
}

void processCommand(CommsBuffer* buffer) {
    for (Command& command : availableCommands) {
        if (command.matches(buffer)) {
            CommsBuffer result = command.call();

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