#include <Arduino.h>
#include <WebServer.h>
#include "local_api.h"

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
    server.begin();
}

void handleLocalApi()
{
    server.handleClient();
}
