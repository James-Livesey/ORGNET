#include "cmd.h"

std::vector<Command> availableCommands = {
    Command("HELLO", []() -> CommsBuffer {
        printf("HELLO command called\n");

        return stringBuffer(ReturnCode::SUCCESS, "Hello, world!");
    }),
    Command("WISTAT", []() -> CommsBuffer {
        return stringBuffer(ReturnCode::SUCCESS, "\x01\x03" "Test Network");
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