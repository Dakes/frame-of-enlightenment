#include <Arduino.h>
#include <WebServer.h>
#include "local_api.h"
#include "runtime_config.h"
#include "utils.h"

static WaniKani* g_wk = nullptr;
static WebServer server(80);

static void handleWaniKaniPost()
{
    if (!server.hasArg("plain"))
    {
        server.send(400, "text/plain", "Missing body");
        return;
    }
    String body = server.arg("plain");
    if (g_wk && g_wk->setSummaryJson(body))
    {
        g_wk->refresh();
        server.send(200, "text/plain", "OK");
    }
    else
    {
        server.send(400, "text/plain", "Invalid JSON");
    }
}

void setupLocalApi(WaniKani* wk)
{
    g_wk = wk;
    server.on("/wanikani_test_data", HTTP_POST, handleWaniKaniPost);
    server.on("/config", HTTP_GET, [](){
        bool updated = false;
        bool wifiUpdated = false;
        if (server.hasArg("frame_full")) { g_config.frameFull = server.arg("frame_full").toInt(); updated = true; }
        if (server.hasArg("framerate")) { g_config.frameRate = server.arg("framerate").toInt(); updated = true; }
        if (server.hasArg("min_s")) { g_config.minS = server.arg("min_s").toInt(); updated = true; }
        if (server.hasArg("min_v")) { g_config.minV = server.arg("min_v").toInt(); updated = true; }
        if (server.hasArg("hue_lesson")) { g_config.hueLesson = server.arg("hue_lesson").toInt(); updated = true; }
        if (server.hasArg("hue_review")) { g_config.hueReview = server.arg("hue_review").toInt(); updated = true; }
        if (server.hasArg("hue_review_future")) { g_config.hueReviewFuture = server.arg("hue_review_future").toInt(); updated = true; }
        if (server.hasArg("wifi_ssid")) { g_config.wifiSsid = server.arg("wifi_ssid"); updated = true; wifiUpdated = true; }
        if (server.hasArg("wifi_pass")) { g_config.wifiPass = server.arg("wifi_pass"); updated = true; wifiUpdated = true; }
        if (server.hasArg("wifi_backup_ssid")) { g_config.wifiBackupSsid = server.arg("wifi_backup_ssid"); updated = true; wifiUpdated = true; }
        if (server.hasArg("wifi_backup_pass")) { g_config.wifiBackupPass = server.arg("wifi_backup_pass"); updated = true; wifiUpdated = true; }
        if (updated) { g_config.save(); }
        String resp = "{";
        resp += "\"frame_full\":" + String(g_config.frameFull);
        resp += ",\"framerate\":" + String(g_config.frameRate);
        resp += ",\"min_s\":" + String(g_config.minS);
        resp += ",\"min_v\":" + String(g_config.minV);
        resp += ",\"hue_lesson\":" + String(g_config.hueLesson);
        resp += ",\"hue_review\":" + String(g_config.hueReview);
        resp += ",\"hue_review_future\":" + String(g_config.hueReviewFuture);
        resp += ",\"wifi_ssid\":\"" + g_config.wifiSsid + "\"";
        resp += ",\"wifi_pass\":\"" + g_config.wifiPass + "\"";
        resp += ",\"wifi_backup_ssid\":\"" + g_config.wifiBackupSsid + "\"";
        resp += ",\"wifi_backup_pass\":\"" + g_config.wifiBackupPass + "\"";
        resp += "}";
        server.send(200, "application/json", resp);
        if (wifiUpdated) { Utils::WifiConnect(); }
    });
    server.on("/config/reset", HTTP_POST, [](){
        g_config.reset();
        server.send(200, "text/plain", "OK");
        Utils::WifiConnect();
    });
    server.begin();
}

void handleLocalApi()
{
    server.handleClient();
}
