#include <queue>
#include <Arduino.h>
#include "util/led.h"
#include "util/button.h"
#include "util/irReciever.hpp"
#include "util/rotaryDipSwitch.hpp"
#include "util/logger.hpp"
#include <HTTPClient.h>
#include <ArduinoJson.h>

/* I/O定義 */
#define RECIEVE1_PIN        36
#define MODE_SWITCH_PIN     26
#define ROTARY_SWITCH_PIN1  33
#define ROTARY_SWITCH_PIN2  32
#define ROTARY_SWITCH_PIN3  3
#define ROTARY_SWITCH_PIN4  1
#define IS_RECEIVED_PIN     18
#define IS_HIT_PIN          19
#define LED_PIN             25

#define ONE_FRAME_MILLIS 550

/* 演出トリガー用フラグ */
int damageFlag = -1;
bool is_led_on = false;

//作成タスクのHandleへのポインタ
TaskHandle_t th[4];

const char *ssid = "your-ssid";
const char *password = "your-password";

// Your Domain name with URL path or IP address with path
String serverName = "your-url";
HTTPClient http;

Led led;
Button button(39, true, 10);
IrReceiver ir_receiver = IrReceiver(RECIEVE1_PIN, {});
enum Mode
{
  test = 0,
  production
};
Mode run_mode = Mode::test;
Logger logger;

// 0始まりの個体識別子
static int unit_id = 1;

/* LCD制御タスク */
void lcdControl(void *pvParameters)
{
  while (1)
  {
    //被弾演出
    if (damageFlag >= 0)
    {
      if (run_mode == Mode::production)
      {
        String serverPath = serverName + "points";

        StaticJsonDocument<JSON_OBJECT_SIZE(2)> json_request;
        json_request["from_id"] = damageFlag + 1;
        damageFlag = -1;
        json_request["to_id"] = unit_id + 1;
        char buffer[255];
        serializeJson(json_request, buffer, sizeof(buffer));

        // Your Domain name with URL path or IP address with path
        http.begin(serverPath.c_str());

        http.addHeader("Content-Type", "application/json");
        // Send HTTP POST request
        int httpResponseCode = http.POST((uint8_t *)buffer, strlen(buffer));
        
        // Free resources
        http.end();
      }
      digitalWrite(IS_HIT_PIN, HIGH);
      led.drawpix(0x001000);
    }

    unsigned long now = millis();
    unsigned long received_time = ir_receiver.received_time;
    if (is_led_on)
    {
      if (now - received_time > ONE_FRAME_MILLIS)
      {
        digitalWrite(IS_RECEIVED_PIN, LOW);
        digitalWrite(LED_PIN, LOW);
        is_led_on = false;
      }
    }
    else if (now - received_time < ONE_FRAME_MILLIS)
    {
      digitalWrite(IS_RECEIVED_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
      is_led_on = true;
    }
    delay(1);
  }
}

void setupWifi(Mode run_mode)
{
  if (run_mode == Mode::test)
  {
    return;
  }

  WiFi.begin(ssid, password);

  int retry_count = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    retry_count++;
    if (retry_count > 20)
    {
      ESP.restart();
    }
    delay(500);
  }
}

void getMacAddress(char baseMacChr[18])
{
  uint8_t baseMac[6];
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  logger.printIfEnable("MAC address: %s\n", baseMacChr);
}

void postDeviceInfo(Mode run_mode, char mac_address[18])
{
  if (run_mode == Mode::test)
  {
    return;
  }
  String serverPath = serverName + "devices";

  StaticJsonDocument<JSON_OBJECT_SIZE(3)> json_request;
  if (run_mode == Mode::test)
  {
    json_request["mode"] = "test";
  }
  else
  {
    json_request["mode"] = "production";
  }
  json_request["mac"] = mac_address;
  json_request["unit_id"] = unit_id + 1;
  char buffer[255];
  serializeJson(json_request, buffer, sizeof(buffer));

  // Your Domain name with URL path or IP address with path
  http.begin(serverPath.c_str());

  http.addHeader("Content-Type", "application/json");
  // Send HTTP POST request
  int httpResponseCode = http.POST((uint8_t *)buffer, strlen(buffer));
  // Free resources
  http.end();
}

void setup()
{
  digitalWrite(IS_RECEIVED_PIN, LOW);
  digitalWrite(IS_HIT_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  pinMode(RECIEVE1_PIN, INPUT);
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);

  pinMode(IS_RECEIVED_PIN, OUTPUT);
  pinMode(IS_HIT_PIN, OUTPUT);

  pinMode(LED_PIN, OUTPUT);

  if (digitalRead(MODE_SWITCH_PIN) == HIGH)
  {
    run_mode = Mode::production;
  }
  else
  {
    logger.enable();
    logger.printIfEnable("run test mode\n");
  }

  setupWifi(run_mode);

  char mac_address[18] = {};
  getMacAddress(mac_address);

  postDeviceInfo(run_mode, mac_address);

  // IR受信割り込みの設定
  ir_receiver.setup([]
                    { ir_receiver.update_data(); });

  // LCD制御タスク
  xTaskCreatePinnedToCore(lcdControl, "lcdControl", 4096, NULL, 1, &th[0], 0);

  uint8_t hue = 0;
  for (int i = 0; i < 100; i++)
  {
    led.drawpix(CHSV(hue++, 255, 255));
    delay(10);
  }
  while (led._led != CRGB(0, 0, 0))
  {
    led.fadeToBlackBy(40);
    delay(50);
  }
}

void loop()
{
  //ボタン判定
  button.read();

  if (button.wasPressed()) {
    logger.printIfEnable("button pressed -> reboot\n");
    ESP.restart();
  }

  /* IR受信判定 */
  int ir_data = ir_receiver.ir_received_data;
  if (ir_data > 0)
  {
    if (ir_data == 0b1110100011)
    {
      //被弾フラグ
      damageFlag = 0;
    }
    else if (ir_data == 0b1110010011)
    {
      //被弾フラグ
      damageFlag = 1;
    }
    else if (ir_data == 0b1110001011)
    {
      //被弾フラグ
      damageFlag = 2;
    }
    ir_receiver.ir_received_data = 0;
  }
}
