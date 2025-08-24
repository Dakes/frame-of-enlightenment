#include <Arduino.h>
#include <WebServer.h>
#include "local_api.h"
#include "runtime_config.h"
#include "utils.h"
#include <pgmspace.h>

static WaniKani* g_wk = nullptr;
static WebServer server(80);

static const char CONFIG_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>Frame of Enlightenment Config</title>
<style>
body{font-family:Arial,Helvetica,sans-serif;background:#f5f5f5;margin:0;display:flex;justify-content:center;}
.container{max-width:480px;margin-top:40px;background:#fff;padding:20px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1);}
h1{text-align:center;margin-top:0;font-size:1.5em;}
label{display:block;margin-bottom:12px;}
input{width:100%;padding:8px;margin-top:4px;box-sizing:border-box;}
button{margin-top:10px;padding:10px 16px;border:none;border-radius:4px;background:#007bff;color:#fff;cursor:pointer;}
button#reset{background:#dc3545;}
button:hover{opacity:0.9;}
</style>
</head>
<body>
<div class="container">
<h1>Frame of Enlightenment config</h1>
<form id="cfgForm">
<label title="Total items required to light the entire frame">Frame Full:<input type="number" id="frame_full" name="frame_full"></label>
<label title="Display refresh rate (frames per second)">Framerate:<input type="number" id="framerate" name="framerate"></label>
<label title="Minimum saturation used for animated cells">Minimum Saturation:<input type="number" id="min_s" name="min_s"></label>
<label title="Base brightness level applied to animated cells">Base Brightness:<input type="number" id="min_v" name="min_v"></label>
<label title="Base color saturation">Max S:<input type="number" id="max_s" name="max_s"></label>
<label title="Base brightness value">Max V:<input type="number" id="max_v" name="max_v"></label>
<label title="Multiplier for the internal simulation resolution. 10x means 10x higher resolution per axis, so 100 simulated cells per LED">Matrix Resolution:<input type="number" id="matrix_resolution" name="matrix_resolution"></label>
<label title="Hue for lesson cells">Hue Lesson:<input type="number" id="hue_lesson" name="hue_lesson"></label>
<label title="Hue for review cells">Hue Review:<input type="number" id="hue_review" name="hue_review"></label>
<label title="Hue for upcoming reviews">Hue Review Future:<input type="number" id="hue_review_future" name="hue_review_future"></label>
<label title="Primary WiFi network name">WiFi SSID:<input type="text" id="wifi_ssid" name="wifi_ssid"></label>
<label title="Primary WiFi password">WiFi Pass:<input type="password" id="wifi_pass" name="wifi_pass"></label>
<label title="Backup WiFi network name">WiFi Backup SSID:<input type="text" id="wifi_backup_ssid" name="wifi_backup_ssid"></label>
<label title="Backup WiFi password">WiFi Backup Pass:<input type="password" id="wifi_backup_pass" name="wifi_backup_pass"></label>
<button type="submit">Save</button>
</form>
<button id="reset">Reset to defaults</button>
<pre id="status"></pre>
</div>
<script>
function load(){
 fetch('/config').then(r=>r.json()).then(cfg=>{
  for(const k in cfg){
   const el=document.getElementById(k);
   if(el){
    el.value=(cfg[k]==='***')?'':cfg[k];
   }
  }
 });
}
document.getElementById('cfgForm').addEventListener('submit',e=>{
 e.preventDefault();
 const data=new FormData(e.target);
 const params=new URLSearchParams();
 for(const pair of data.entries()){
  if(pair[1]) params.append(pair[0], pair[1]);
 }
 fetch('/config?'+params.toString()).then(r=>r.json()).then(cfg=>{
  document.getElementById('status').textContent='Saved';
  load();
 });
});
document.getElementById('reset').addEventListener('click',()=>{
 fetch('/config/reset',{method:'POST'}).then(()=>{load();});
});
load();
</script>
</body>
</html>
)rawliteral";

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
    server.on("/", HTTP_GET, [](){
        server.send_P(200, "text/html", CONFIG_HTML);
    });
    server.on("/config", HTTP_GET, [](){
        bool updated = false;
        bool wifiUpdated = false;
        if (server.hasArg("frame_full")) { g_config.frameFull = server.arg("frame_full").toInt(); updated = true; }
        if (server.hasArg("framerate")) { g_config.frameRate = server.arg("framerate").toInt(); updated = true; }
        if (server.hasArg("min_s")) { g_config.minS = server.arg("min_s").toInt(); updated = true; }
        if (server.hasArg("min_v")) { g_config.minV = server.arg("min_v").toInt(); updated = true; }
        if (server.hasArg("max_s")) { g_config.maxS = server.arg("max_s").toInt(); updated = true; }
        if (server.hasArg("max_v")) { g_config.maxV = server.arg("max_v").toInt(); updated = true; }
        if (server.hasArg("matrix_resolution")) { g_config.matrixResolution = server.arg("matrix_resolution").toInt(); updated = true; }
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
        resp += ",\"max_s\":" + String(g_config.maxS);
        resp += ",\"max_v\":" + String(g_config.maxV);
        resp += ",\"matrix_resolution\":" + String(g_config.matrixResolution);
        resp += ",\"hue_lesson\":" + String(g_config.hueLesson);
        resp += ",\"hue_review\":" + String(g_config.hueReview);
        resp += ",\"hue_review_future\":" + String(g_config.hueReviewFuture);
        resp += ",\"wifi_ssid\":\"" + g_config.wifiSsid + "\"";
        resp += ",\"wifi_pass\":\"" + String(g_config.wifiPass.length() ? "***" : "") + "\"";
        resp += ",\"wifi_backup_ssid\":\"" + g_config.wifiBackupSsid + "\"";
        resp += ",\"wifi_backup_pass\":\"" + String(g_config.wifiBackupPass.length() ? "***" : "") + "\"";
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
