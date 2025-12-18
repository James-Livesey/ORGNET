#ifndef WIFI_H_
#define WIFI_H_

#include <vector>
#include <list>
#include <string>

#include "pico/cyw43_arch.h"
#include "cyw43.h"

enum WifiSecurityMode {
    OPEN = 0
};

class WifiAccessPoint {
    public:
    WifiAccessPoint(const cyw43_ev_scan_result_t* scanResult);
    
    std::string getSsid();
    char getStrength();
    WifiSecurityMode getSecurityMode();
    
    private:
    std::string _ssid;
    int _rssi;
};

namespace wifi {
    extern std::vector<WifiAccessPoint> accessPoints;

    void init();
    void scan();
}

#endif