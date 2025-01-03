#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <MCP23017.h>
#include <tm1630.hpp>

MCP23017 mcp = MCP23017(0x20);

// コントローラーにセットして電源を入れたら左のスイッチを押しながら右のスイッチを押すと起動する。
const int tempsensorGood = 0;
const int tempsensorBad = 1;
const int watersensorGood = 2;
const int watersensorBad = 4;
const int batteryvoltageGood = 3;
const int batteryvoltageBad = 6;
const int communicationstatusGood = 5;
const int communicationstatusBad = 7;

const int upSwitch = 9;
const int downSwitch = 11;
const int rightSwitch = 8;
const int leftSwitch = 12;
const int buzzerSwitch = 10;
const int tempSwitch = 13;
// const int redLedSwitch = 6;
// const int blueLedSwitch = 7;

unsigned long lastTime = 0;
unsigned long timerDelay = 100; // send readings timer

typedef struct struct_message
{
  int up;
  int down;
  int right;
  int left;
  int buzzer;
  int temp;
  int battery;
  int water;
} struct_message;

// Create a struct_message called controllerData
struct_message controllerData;
struct_message shipData;

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x18, 0xFE, 0x34, 0xEF, 0x08, 0x83};

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
{
  memcpy(&shipData, incomingData, sizeof(shipData));
  if (shipData.temp < 500)// tempは10倍の値で渡される
  {
    mcp.digitalWrite(tempsensorGood, 1);
    mcp.digitalWrite(tempsensorBad, 0);
  }
  else
  {
    mcp.digitalWrite(tempsensorGood, 0);
    mcp.digitalWrite(tempsensorBad, 1);
  }
  if (shipData.water == 1)
  {
    mcp.digitalWrite(watersensorGood, 1);
    mcp.digitalWrite(watersensorBad, 0);
  }
  else
  {
    mcp.digitalWrite(watersensorGood, 0);
    mcp.digitalWrite(watersensorBad, 1);
  }
  if (shipData.battery > 660) // 1セル当たり3.3Vで警告
  {
    mcp.digitalWrite(batteryvoltageGood, 1);
    mcp.digitalWrite(batteryvoltageBad, 0);
  }
  else
  {
    mcp.digitalWrite(batteryvoltageGood, 0);
    mcp.digitalWrite(batteryvoltageBad, 1);
  }
}

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
  // Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0)
  {
    // Serial.println("Delivery success");
    mcp.digitalWrite(communicationstatusGood, 1);
    mcp.digitalWrite(communicationstatusBad, 0);
  }
  else
  {
    // Serial.println("Delivery fail");
    mcp.digitalWrite(communicationstatusGood, 0);
    mcp.digitalWrite(communicationstatusBad, 1);
    mcp.digitalWrite(batteryvoltageGood, 0);
    mcp.digitalWrite(batteryvoltageBad, 1);
    mcp.digitalWrite(watersensorGood, 0);
    mcp.digitalWrite(watersensorBad, 1);
    mcp.digitalWrite(tempsensorGood, 0);
    mcp.digitalWrite(tempsensorBad, 1);
  }
}

void gpioSetup()
{
  for (int i = 0; i <= 7; i++)
  {
    mcp.pinMode(i, OUTPUT);
    mcp.digitalWrite(i, 0);
  }
  for (int i = 8; i <= 13; i++)
  {
    mcp.pinMode(i, INPUT);
  }
}

void inPutValue()
{
  // 基本的に"1"を送信。"0"はスイッチが押されている状態。
  controllerData.up = mcp.digitalRead(upSwitch);
  controllerData.down = mcp.digitalRead(downSwitch);
  controllerData.right = mcp.digitalRead(rightSwitch);
  controllerData.left = mcp.digitalRead(leftSwitch);
  controllerData.buzzer = mcp.digitalRead(buzzerSwitch);
  controllerData.temp = mcp.digitalRead(tempSwitch);
  controllerData.battery = 1;
  controllerData.water = 1;
}

void setup()
{
  // Init Serial Monitor
  // Serial.begin(9600);
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Wire.begin(2, 14);
  mcp.init();
  gpioSetup();   // I/Oエキスパンダ
  tm1630setup(); // 7セグドライバ
  // Init ESP-NOW
  if (esp_now_init() != 0)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop()
{
  if ((millis() - lastTime) > timerDelay)
  {
    inPutValue();
    if (controllerData.temp == 0)
    {
      displayNumbers(shipData.temp, 1);
    }
    else
    {
      displayNumbers(shipData.battery, 2);
    }
    esp_now_send(broadcastAddress, (uint8_t *)&controllerData, sizeof(controllerData));
    lastTime = millis(); // プログラム実行から経過した時間
  }
}