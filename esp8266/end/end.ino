#include <ESP8266WiFi.h>
#include <PubSubClient.h>
// ==================== WiFi 配置 ====================
const char* WIFI_SSID     = "WIFI_NAME";
const char* WIFI_PASSWORD = "WIFI_PASSWD";
// ==================== OneNET MQTT 配置 ====================
const char* MQTT_HOST     = "mqtts.heclouds.com";
const int   MQTT_PORT     = 1883;
const char* PRODUCT_ID    = "PRODUCT_ID_PLACEHOLDER";
const char* DEVICE_ID     = "DEVICE_ID_PLACEHOLDER";
const char* AUTH_INFO     = "version=2018-10-31&res=products%2FPRODUCT_ID_PLACEHOLDER%2Fdevices%2FDEVICE_ID_PLACEHOLDER&et=EXPIRE_TIME&method=md5&sign=SIGN_DATA";
const char* TOPIC_POST = "$sys/PRODUCT_ID_PLACEHOLDER/DEVICE_ID_PLACEHOLDER/thing/property/post";
const char* TOPIC_SET  = "$sys/PRODUCT_ID_PLACEHOLDER/DEVICE_ID_PLACEHOLDER/thing/property/set";

WiFiClient espClient;
PubSubClient client(espClient);

// ==================== 全局变量 ====================
bool streamstate = false;
bool lockstate   = false;
bool cmd         = false;
bool mode        = false;

// ==================== 云平台下发回调 ====================
void callback(char* topic, byte* payload, unsigned int length) {
  char jsonBuf[256];
  for (int i = 0; i < length; i++) jsonBuf[i] = (char)payload[i];
  jsonBuf[length] = '\0';
  if (strstr(jsonBuf, "\"cmd\":true"))    cmd = true;
  if (strstr(jsonBuf, "\"cmd\":false"))   cmd = false;
  if (strstr(jsonBuf, "\"mode\":true"))   mode = true;
  if (strstr(jsonBuf, "\"mode\":false"))  mode = false;
  // 👉 发给 STM32（唯一串口输出）
  char sendBuf[64];
  sprintf(sendBuf, "[mode:%s,cmd:%s]\n",
          mode ? "true" : "false",
          cmd ? "true" : "false");
  Serial.print(sendBuf);   // ⚠️ 这里只能发业务数据
}

// ==================== MQTT 重连 ====================
void mqttReconnect() {
  while (!client.connected()) {
    if (client.connect(DEVICE_ID, PRODUCT_ID, AUTH_INFO)) {
      client.subscribe(TOPIC_SET);
    } else {
      delay(2000);
    }
  }
}

// ==================== 上报云端 ====================
void uploadToCloud() {
  char jsonBuf[256];
  sprintf(jsonBuf,
    "{"
    "\"id\":\"123\","
    "\"version\":\"1.0\","
    "\"params\":{"
      "\"streamstate\":{\"value\":%s},"
      "\"lockstate\":{\"value\":%s},"
      "\"cmd\":{\"value\":%s},"
      "\"mode\":{\"value\":%s}"
    "}"
    "}",
    streamstate ? "true" : "false",
    lockstate ? "true" : "false",
    cmd ? "true" : "false",
    mode ? "true" : "false"
  );
  client.publish(TOPIC_POST, jsonBuf);
}

// ==================== 接收 STM32 ====================
void receiveFromSTM32() {
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    data.trim();
    if (data.indexOf("[") != -1 && data.indexOf("]") != -1) {
      if (data.indexOf("streamstate:true") != -1)        streamstate = true;
      else if (data.indexOf("streamstate:false") != -1) streamstate = false;
      if (data.indexOf("lockstate:true") != -1)          lockstate = true;
      else if (data.indexOf("lockstate:false") != -1)   lockstate = false;
      if (data.indexOf("cmd:true") != -1)                cmd = true;
      else if (data.indexOf("cmd:false") != -1)          cmd = false;
      if (data.indexOf("mode:true") != -1)               mode = true;
      else if (data.indexOf("mode:false") != -1)         mode = false;
    }
  }
}

// ==================== 初始化 ====================
void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(callback);
}

// ==================== 主循环 ====================
void loop() {
  if (!client.connected()) mqttReconnect();
  client.loop();
  receiveFromSTM32();
  static unsigned long t = 0;
  if (millis() - t > 5000) {
    t = millis();
    uploadToCloud();
  }
}
