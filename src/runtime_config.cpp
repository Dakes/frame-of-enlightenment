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
    hueLesson = prefs.getUChar("hLesson", hueLesson);
    hueReview = prefs.getUChar("hReview", hueReview);
    hueReviewFuture = prefs.getUChar("hReviewF", hueReviewFuture);
    prefs.end();
}

void RuntimeConfig::save() {
    prefs.begin("cfg", false);
    prefs.putUInt("frameFull", frameFull);
    prefs.putUInt("frameRate", frameRate);
    prefs.putUChar("minS", minS);
    prefs.putUChar("minV", minV);
    prefs.putUChar("hLesson", hueLesson);
    prefs.putUChar("hReview", hueReview);
    prefs.putUChar("hReviewF", hueReviewFuture);
    prefs.end();
}

void RuntimeConfig::reset() {
    frameFull = 700;
    frameRate = 60;
    minS = 50;
    minV = 20;
    hueLesson = 220;
    hueReview = 142;
    hueReviewFuture = 35;
    save();
}

