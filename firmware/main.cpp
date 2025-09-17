#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"

#include "datapack.h"
#include "asmtest.h"

void stepper() {
    while (true) {
        Datapack::the()->step();
    }
}

int main() {
    stdio_init_all();

    Datapack::the()->loadCode(asmTestCode, sizeof(asmTestCode));

    set_sys_clock_khz(260'000, true);
    irq_set_mask_enabled(0x0F, false);
    multicore_launch_core1(stepper);

    while (true) {}    

    return 0;
}