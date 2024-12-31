#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <espnow.h>

#include <tm1630.hpp>

// コントローラーにセットして電源を入れたら左のスイッチを押しながら右のスイッチを押すと起動する。


unsigned long lastTime = 0;
unsigned long timerDelay = 500; // send readings timer

typedef struct struct_message
{
  int white;
  int red;
  int yellow;
  int orange;
  int Onled;
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
}

void gpioSetup()
{

}

void inPutValue()
{

}

void setup()
{
  // Init Serial Monitor
  Serial.begin(9600);
  shipData.battery = 1;
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  gpioSetup();
  tm1630setup();
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
  delay(1000);
  digitalWrite(15, HIGH);
  delay(1000);
  digitalWrite(15, LOW);
  delay(1000);
  digitalWrite(15, HIGH);
  delay(1000);
  digitalWrite(15, LOW);
  delay(1000);
}

void loop()
{
  if ((millis() - lastTime) > timerDelay) // 50msに1回
  {
    // Set values to send
    // inPutValue();
    // Send message via ESP-NOW
    // esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    displayNumbers(800);
    lastTime = millis(); // プログラム実行から経過した時間
  }
}