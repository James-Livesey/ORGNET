#include <wifi.h>

std::vector<WifiAccessPoint> wifi::accessPoints;
std::list<WifiAccessPoint> incomingAccessPoints;

WifiAccessPoint::WifiAccessPoint(const cyw43_ev_scan_result_t* scanResult) {
    _ssid = std::string((char*)scanResult->ssid, scanResult->ssid_len);
    _rssi = scanResult->rssi;
}

std::string WifiAccessPoint::getSsid() {
    return _ssid;
}

char WifiAccessPoint::getStrength() {
    if (_rssi > -70) {
        return 2;
    } else if (_rssi > -90) {
        return 1;
    } else {
        return 0;
    }
}

WifiSecurityMode WifiAccessPoint::getSecurityMode() {
    return WifiSecurityMode::OPEN;
}

int wifiScanCallback(void* env, const cyw43_ev_scan_result_t* result) {
    WifiAccessPoint ap(result);

    incomingAccessPoints.push_back(ap);

    return 0;
}

void wifi::init() {
    if (cyw43_arch_init()) {
        return;
    }

    cyw43_arch_enable_sta_mode();
}

void wifi::scan() {
    cyw43_arch_lwip_begin();

    if (cyw43_wifi_scan_active(&cyw43_state)) {
        return;
    }

    wifi::accessPoints = {std::begin(incomingAccessPoints), std::end(incomingAccessPoints)};
    incomingAccessPoints = {};

    cyw43_wifi_scan_options_t options = {0};

    cyw43_wifi_scan(&cyw43_state, &options, nullptr, wifiScanCallback);

    cyw43_arch_lwip_end();
}