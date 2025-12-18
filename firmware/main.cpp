#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"

#include "datapack.h"
#include "cmd.h"
#include "wifi.h"
#include "_app.h"

#define WIFI_SCAN_INTERVAL 3000

Datapack* datapack;
uint64_t lastWifiScan = 0;

void stepper() {
    while (true) {
        Datapack::the()->step();
    }
}

int main() {
    stdio_init_all();

    datapack = Datapack::the();

    datapack->loadCode((char*)appCode, appCode_len);

    wifi::init();

    set_sys_clock_khz(260'000, true);
    irq_set_mask_enabled(0x0F, false);
    multicore_launch_core1(stepper);

    while (true) {
        if (datapack->hasAvailableCommand()) {
            sleep_ms(10);

            CommsBuffer buffer = datapack->getCommsBuffer();

            cmd::processCommand(&buffer);
            datapack->setCommsBuffer(&buffer);
        }

        if (time_us_64() - lastWifiScan > WIFI_SCAN_INTERVAL * 1000) {
            wifi::scan();

            lastWifiScan = time_us_64();
        }

        tight_loop_contents();
    }

    return 0;
}