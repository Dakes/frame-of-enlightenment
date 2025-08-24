#include "utils.h"
#include "runtime_config.h"

void Utils::WifiConnect()
{
    const uint8_t retrySeconds = 60;

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    WiFi.setHostname("WaniKani frame of enlightenment");
    WiFi.begin(g_config.wifiSsid.c_str(), g_config.wifiPass.c_str());

    Serial.println("Connecting to WiFi");
    uint8_t wifiCounter = 0;
    while (!WiFi.isConnected())
    {
        // Check to see if connecting failed.
        // This is due to incorrect credentials
        if (WiFi.status() == WL_CONNECT_FAILED)
        {
            Serial.println("Failed to connect to WIFI. Please verify credentials");
        }

        if (g_config.wifiBackupSsid.length() > 0)
        {
            if (wifiCounter % (retrySeconds * 2) == retrySeconds * 2 - 1)
            {
                Serial.println("Changing WiFi to " + g_config.wifiSsid);
                WiFi.disconnect();
                WiFi.begin(g_config.wifiSsid.c_str(), g_config.wifiPass.c_str());
            }
            else if (wifiCounter % retrySeconds == retrySeconds - 1)
            {
                Serial.println("Changing WiFi to " + g_config.wifiBackupSsid);
                WiFi.disconnect();
                WiFi.begin(g_config.wifiBackupSsid.c_str(), g_config.wifiBackupPass.c_str());
            }
        }
        delay(1000);
        wifiCounter++;
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

bool Utils::WiFiConnected()
{
    return WiFi.localIP().toString() != "0.0.0.0";
    // return WiFi.status() == WL_CONNECTED;
}

String Utils::httpGETRequest(const String url, const String headerName, const String headerValue)
{
    WiFiClientSecure client;
    HTTPClient http;

    // don't validate certificates
    client.setInsecure();

    // Your Domain name with URL path or IP address with path
    http.useHTTP10(true);
    http.begin(client, url);
    http.addHeader(headerName, headerValue);

    // Send HTTP GET request
    int httpResponseCode = http.GET();
    String payload = "";

    if (httpResponseCode > 0 && httpResponseCode < 400)
    {
        payload = http.getString();
    }
    else
    {
        Serial.print("HTTP Request, Error code: ");
        Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();

    return payload;
}

uint32_t Utils::pair(int16_t a, int16_t b)
{
    return a << 16 | b;
}
