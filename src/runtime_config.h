#ifndef RUNTIME_CONFIG_H
#define RUNTIME_CONFIG_H

#include <Arduino.h>
#include "config.h"

struct RuntimeConfig {
    uint16_t frameFull = 700;
    uint16_t frameRate = 60;
    uint8_t minS = 50;
    uint8_t minV = 20;
    uint8_t hueLesson = 220;
    uint8_t hueReview = 142;
    uint8_t hueReviewFuture = 35;
    String wifiSsid = WIFI_SSID;
    String wifiPass = WIFI_PASS;
    String wifiBackupSsid = WIFI_BACKUP_SSID;
    String wifiBackupPass = WIFI_BACKUP_PASS;

    void load();
    void save();
    void reset();
};

extern RuntimeConfig g_config;

#endif

