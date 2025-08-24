#include "runtime_config.h"
#include <Preferences.h>

static Preferences prefs;

RuntimeConfig g_config;

void RuntimeConfig::load() {
    prefs.begin("cfg", true);
    frameFull = prefs.getUInt("frameFull", frameFull);
    frameRate = prefs.getUInt("frameRate", frameRate);
    minS = prefs.getUChar("minS", minS);
    minV = prefs.getUChar("minV", minV);
    maxS = prefs.getUChar("maxS", maxS);
    maxV = prefs.getUChar("maxV", maxV);
    matrixResolution = prefs.getUChar("matrixRes", matrixResolution);
    hueLesson = prefs.getUChar("hLesson", hueLesson);
    hueReview = prefs.getUChar("hReview", hueReview);
    hueReviewFuture = prefs.getUChar("hReviewF", hueReviewFuture);
    wifiSsid = prefs.getString("wifiSsid", wifiSsid);
    wifiPass = prefs.getString("wifiPass", wifiPass);
    wifiBackupSsid = prefs.getString("wifiBackupSsid", wifiBackupSsid);
    wifiBackupPass = prefs.getString("wifiBackupPass", wifiBackupPass);
    prefs.end();
}

void RuntimeConfig::save() {
    prefs.begin("cfg", false);
    prefs.putUInt("frameFull", frameFull);
    prefs.putUInt("frameRate", frameRate);
    prefs.putUChar("minS", minS);
    prefs.putUChar("minV", minV);
    prefs.putUChar("maxS", maxS);
    prefs.putUChar("maxV", maxV);
    prefs.putUChar("matrixRes", matrixResolution);
    prefs.putUChar("hLesson", hueLesson);
    prefs.putUChar("hReview", hueReview);
    prefs.putUChar("hReviewF", hueReviewFuture);
    prefs.putString("wifiSsid", wifiSsid);
    prefs.putString("wifiPass", wifiPass);
    prefs.putString("wifiBackupSsid", wifiBackupSsid);
    prefs.putString("wifiBackupPass", wifiBackupPass);
    prefs.end();
}

void RuntimeConfig::reset() {
    frameFull = 700;
    frameRate = 60;
    minS = 50;
    minV = 20;
    maxS = 255;
    maxV = 255;
    matrixResolution = 10;
    hueLesson = 220;
    hueReview = 142;
    hueReviewFuture = 35;
    wifiSsid = WIFI_SSID;
    wifiPass = WIFI_PASS;
    wifiBackupSsid = WIFI_BACKUP_SSID;
    wifiBackupPass = WIFI_BACKUP_PASS;
    save();
}

