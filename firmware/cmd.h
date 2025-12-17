#ifndef CMD_H_
#define CMD_H_

#include <string>
#include <functional>
#include <vector>
#include <span>

#include "datapack.h"

typedef std::span<char> CommandArguments;
typedef std::function<CommsBuffer(CommandArguments)> CommandCallback;

enum ReturnCode {
    SUCCESS = 1,
    ERROR = 2
};

class Command {
    public:
        Command(std::string name, CommandCallback callback);

        const std::string getName();

        bool matches(CommsBuffer* buffer);
        CommsBuffer call(CommandArguments args);

    private:
        std::string _name;
        CommandCallback _callback;
};

extern std::vector<Command> availableCommands;

void processCommand(CommsBuffer* buffer);

CommsBuffer stringBuffer(ReturnCode returnCode, std::string data);

#endif