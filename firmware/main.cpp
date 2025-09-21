#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"

#include "datapack.h"
#include "cmd.h"
#include "_app.h"

Datapack* datapack;

void stepper() {
    while (true) {
        Datapack::the()->step();
    }
}

int main() {
    stdio_init_all();

    datapack = Datapack::the();

    datapack->loadCode((char*)appCode, appCode_len);

    irq_set_mask_enabled(0x0F, false);
    multicore_launch_core1(stepper);

    while (true) {
        if (datapack->hasAvailableCommand()) {
            sleep_ms(10);

            CommsBuffer buffer = datapack->getCommsBuffer();

            processCommand(&buffer);
            datapack->setCommsBuffer(&buffer);
        }

        tight_loop_contents();
    }

    return 0;
}