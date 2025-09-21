#ifndef CMD_H_
#define CMD_H_

#include <string>
#include <functional>
#include <vector>

#include "datapack.h"

typedef std::function<CommsBuffer()> CommandCallback;

class Command {
    public:
        Command(std::string name, CommandCallback callback);

        bool matches(CommsBuffer* buffer);
        CommsBuffer call();

    private:
        std::string _name;
        CommandCallback _callback;
};

extern std::vector<Command> availableCommands;

void processCommand(CommsBuffer* buffer);

CommsBuffer stringBuffer(char returnCode, std::string data);

#endif