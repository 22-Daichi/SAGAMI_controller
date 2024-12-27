#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <MCP23017.h>

MCP23017 mcp = MCP23017(0x20);

// コントローラーにセットして電源を入れたら左のスイッチを押しながら右のスイッチを押すと起動する。
const int upSwitch = 0;
const int downSwitch = 1;
const int rightSwitch = 2;
const int leftSwitch = 3;
const int buzzerSwitch = 4;
const int tempSwitch = 5;
const int redLedSwitch = 6;
const int blueLedSwitch = 7;

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

// Create a struct_message called myData
struct_message myData;
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
    mcp.pinMode(i, INPUT);
  }
  for (int i = 8; i <= 15; i++)
  {
    mcp.pinMode(i, OUTPUT);
  }
}

void inPutValue()
{
  myData.up = mcp.digitalRead(upSwitch);
  myData.down = mcp.digitalRead(downSwitch);
  myData.right = mcp.digitalRead(rightSwitch);
  myData.left = mcp.digitalRead(leftSwitch);
  myData.onRedLed = mcp.digitalRead(redLedSwitch);
  myData.onBlueLed = mcp.digitalRead(blueLedSwitch);
  myData.buzzer = mcp.digitalRead(buzzerSwitch);
  myData.temp = mcp.digitalRead(tempSwitch);
  myData.battery = 1;
  myData.water = 1;
}

void setup()
{
  // Init Serial Monitor
  Serial.begin(9600);
  shipData.battery = 1;
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Wire.begin(2, 14);
  mcp.init();
  gpioSetup();
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
    // esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    if (shipData.battery == 0 || myData.up == 0)
    {
      digitalWrite(15, HIGH);
    }
    else
    {
      digitalWrite(15, LOW);
    }
    lastTime = millis(); // プログラム実行から経過した時間
  } */
  mcp.digitalWrite(7, 1);
  delay(500);
  mcp.digitalWrite(7, 0);
  delay(500);
  esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
}