#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <MCP23017.h>
#include <tm1630.hpp>

MCP23017 mcp = MCP23017(0x20);

// コントローラーにセットして電源を入れたら左のスイッチを押しながら右のスイッチを押すと起動する。
const int upSwitch = 8;
const int downSwitch = 9;
const int rightSwitch = 10;
const int leftSwitch = 11;
const int buzzerSwitch = 12;
const int tempSwitch = 13;
// const int redLedSwitch = 6;
// const int blueLedSwitch = 7;

unsigned long lastTime = 0;
unsigned long timerDelay = 500; // send readings timer

typedef struct struct_message
{
  int up;
  int down;
  int right;
  int left;
  int onRedLed;
  int onBlueLed;
  int buzzer;
  int battery;
  int temp;
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
}

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0)
  {
    Serial.println("Delivery success");
  }
  else
  {
    Serial.println("Delivery fail");
    // Serial.println(mcp.digitalRead(8));
  }
}

void gpioSetup()
{
  for (int i = 0; i <= 7; i++)
  {
    mcp.pinMode(i, OUTPUT);
  }
  for (int i = 8; i <= 13; i++)
  {
    mcp.pinMode(i, INPUT);
  }
}

void inPutValue()
{
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
  Serial.begin(9600);
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
  /* if ((millis() - lastTime) > timerDelay) // 50msに1回
  {
    // Set values to send
    inPutValue();
    // Send message via ESP-NOW
    // esp_now_send(broadcastAddress, (uint8_t *)&controllerData, sizeof(controllerData));
    if (shipData.battery == 0 || controllerData.up == 0)
    {
      digitalWrite(15, HIGH);
    }
    else
    {
      digitalWrite(15, LOW);
    }
    lastTime = millis(); // プログラム実行から経過した時間
  } */
  /*
   for (int i = 0; i < 8; i++)
   {
     mcp.digitalWrite(i, 1);
   }
   delay(500);
   */
  for (int i = 0; i < 8; i++)
  {
    mcp.digitalWrite(i, 0);
  }
  inPutValue();
  if (controllerData.up == 0)
  {
    mcp.digitalWrite(1, 1);
  }
  if (controllerData.down == 0)
  {
    mcp.digitalWrite(2, 1);
  }
  if (controllerData.right == 0)
  {
    mcp.digitalWrite(3, 1);
  }
  if (controllerData.left == 0)
  {
    mcp.digitalWrite(4, 1);
  }
  if (controllerData.buzzer == 0)
  {
    mcp.digitalWrite(5, 1);
  }
  if (controllerData.temp == 0)
  {
    mcp.digitalWrite(6, 1);
  }
  displayNumbers(800);
  delay(500);
  esp_now_send(broadcastAddress, (uint8_t *)&controllerData, sizeof(controllerData));
}