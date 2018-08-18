// Demo the quad alphanumeric display LED backpack kit
// scrolls through every character, then scrolls Serial
// input onto the display

#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <SPI.h>
#include <Ethernet2.h>
#include <WebSocketClient.h>
#include <ArduinoJson.h>
#include <Adafruit_SleepyDog.h>
#include <tc.h>
#include <tc_interrupt.h>

Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();
struct tc_module tc_instance;

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0x98, 0x76, 0xB6, 0x10, 0xDB, 0x2C };

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);
char server[] = "192.168.0.53";
WebSocketClient webClient;
EthernetClient client;
bool isLEDOn = false;

#define LED_PIN 13
#define CONF_TC_MODULE TC3

void setup() {
  Watchdog.enable(8192);
  Serial.begin(9600);
  
  alpha4.begin(0x70);  // pass in the address
  alpha4.setBrightness(1); // 1-15

  alpha4.writeDigitRaw(0, 0xFFFF);
  alpha4.writeDigitRaw(1, 0xFFFF);
  alpha4.writeDigitRaw(2, 0xFFFF);
  alpha4.writeDigitRaw(3, 0xFFFF);
  alpha4.writeDisplay();

  Watchdog.reset();
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  Watchdog.reset();
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");

  Watchdog.reset();
  static char assignedIP[12];
  ip2CharArray(Ethernet.localIP(), assignedIP);
  Serial.println(Ethernet.localIP());

  // display every character
  for (uint8_t i=0; i<=sizeof(assignedIP)-3; i+=3) {
    alpha4.writeDigitRaw(0, 0x0);
    alpha4.writeDigitAscii(1, assignedIP[i]);
    alpha4.writeDigitAscii(2, assignedIP[i+1]);
    alpha4.writeDigitAscii(3, assignedIP[i+2]);
    alpha4.writeDisplay();
    Watchdog.reset();
    
    delay(1000);
  }
  
  alpha4.clear();
  alpha4.writeDisplay();

  alpha4.writeDigitAscii(0, '-');
  alpha4.writeDigitAscii(1, '-');
  alpha4.writeDigitAscii(2, '-');
  alpha4.writeDigitAscii(3, '-');
  alpha4.writeDisplay();

  // Handshake with the server
  webClient.path = "/";
  webClient.host = "192.168.0.53";
  
  client.connect(webClient.host, 9000);
  Watchdog.reset();
  
  if (client.connected()) {
    if(!webClient.handshake(client))
    {
        alpha4.writeDigitAscii(0, 'F');
        alpha4.writeDigitAscii(1, 'A');
        alpha4.writeDigitAscii(2, 'I');
        alpha4.writeDigitAscii(3, 'L');
        alpha4.writeDisplay();
        while(true) {delay(300);}
    }
  }

  alpha4.clear();
  alpha4.writeDigitRaw(0, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(0, 0x0);
  alpha4.writeDigitRaw(1, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(1, 0x0);
  alpha4.writeDigitRaw(2, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(2, 0x0);
  alpha4.writeDigitRaw(3, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.clear();
  alpha4.writeDisplay();
  Watchdog.reset();

  alpha4.writeDigitAscii(0, '/');
  alpha4.writeDigitAscii(1, '/');
  alpha4.writeDigitAscii(2, '/');
  alpha4.writeDigitAscii(3, '/');
  alpha4.writeDisplay();

  pinMode(LED_PIN, OUTPUT);
  configure_tc();
  configure_tc_callbacks();
}

void configure_tc(void)
{
  struct tc_config config_tc;
  tc_get_config_defaults(&config_tc);
  config_tc.counter_size = TC_COUNTER_SIZE_8BIT;
  config_tc.clock_source = GCLK_GENERATOR_1;
  config_tc.clock_prescaler = TC_CLOCK_PRESCALER_DIV1024;
  config_tc.counter_8_bit.period = 1;
  config_tc.counter_8_bit.compare_capture_channel[0] = 50;
  config_tc.counter_8_bit.compare_capture_channel[1] = 54;
  tc_init(&tc_instance, CONF_TC_MODULE, &config_tc);
  tc_enable(&tc_instance);
}

void configure_tc_callbacks(void)
{
  tc_register_callback(&tc_instance, heartbeat, TC_CALLBACK_OVERFLOW);
  tc_register_callback(&tc_instance, heartbeat, TC_CALLBACK_CC_CHANNEL0);
  tc_register_callback(&tc_instance, heartbeat, TC_CALLBACK_CC_CHANNEL1);
  tc_enable_callback(&tc_instance, TC_CALLBACK_OVERFLOW);
  tc_enable_callback(&tc_instance, TC_CALLBACK_CC_CHANNEL0);
  tc_enable_callback(&tc_instance, TC_CALLBACK_CC_CHANNEL1);
}

void ip2CharArray(IPAddress ip, char* buf) {
  sprintf(buf, "%03d%03d%03d%03d", ip[0], ip[1], ip[2], ip[3]);
}

void dataArrived(WebSocketClient client, String data) {
  Serial.println("Data Arrived: " + data);
}

extern "C" char *sbrk(int i);
 
int FreeRam () {
  char stack_dummy = 0;
  return &stack_dummy - sbrk(0);
}

#define NUM_OFFSET 48
#define MAX_16 0xFFFFFF
uint16_t cueCount = 0, prevCount = MAX_16;
uint16_t updateFreq = 0;
bool sendUpdate = false;

String stringBuffer;
char charBuffer[256];
StaticJsonDocument<256> jsonBuffer;

void loop() {
  if(client.connected() && cueCount != prevCount) {
    prevCount = cueCount;
    if(cueCount == 0) {
      alpha4.writeDigitAscii(0, '/');
      alpha4.writeDigitAscii(1, '-');
      alpha4.writeDigitAscii(2, '/');
      alpha4.writeDigitAscii(3, '-');
    }
    else if (cueCount < 10) {
      alpha4.writeDigitAscii(0, ' ');
      alpha4.writeDigitAscii(1, ' ');
      alpha4.writeDigitAscii(2, ' ');
      alpha4.writeDigitAscii(3, cueCount + NUM_OFFSET);
    }
    else if(cueCount < 100) {
      alpha4.writeDigitAscii(0, ' ');
      alpha4.writeDigitAscii(1, ' ');
      alpha4.writeDigitAscii(2, ((cueCount/10)%10) + NUM_OFFSET);
      alpha4.writeDigitAscii(3, (cueCount%10) + NUM_OFFSET);
    }
    else if(cueCount < 1000) {
      alpha4.writeDigitAscii(0, ' ');
      alpha4.writeDigitAscii(1, ((cueCount/100)%10) + NUM_OFFSET);
      alpha4.writeDigitAscii(2, ((cueCount/10)%10) + NUM_OFFSET);
      alpha4.writeDigitAscii(3, (cueCount%10) + NUM_OFFSET);
    }
    else if(cueCount < 10000) {
      alpha4.writeDigitAscii(0, ((cueCount/1000)%10) + NUM_OFFSET);
      alpha4.writeDigitAscii(1, ((cueCount/100)%10) + NUM_OFFSET);
      alpha4.writeDigitAscii(2, ((cueCount/10)%10) + NUM_OFFSET);
      alpha4.writeDigitAscii(3, (cueCount%10) + NUM_OFFSET);
    }
    
    alpha4.writeDisplay();
  }

  if (client.connected()) {
    stringBuffer = "";
    webClient.getData(stringBuffer);    
    delay(10);
    
    if (stringBuffer.length() > 0) {
      Serial.println(stringBuffer);
      
      int pos = stringBuffer.indexOf('{');
      jsonBuffer.clear();
      DeserializationError error = deserializeJson(jsonBuffer, stringBuffer.substring(pos));

      if (error) {
        Serial.println("parseObject() failed");
      }
      else {
        JsonObject root = jsonBuffer.as<JsonObject>();
        int update = root["upd"];
        cueCount = update;
      }
    }
  }
  else {
    prevCount = MAX_16;
    client.stop();
    delay(10);
    client.connect(webClient.host, 8000);
    delay(10);
    Serial.println("Disconnected :O");
    if (client.connected()) {
      if(!webClient.handshake(client)) {
        alpha4.writeDigitAscii(0, 'F');
        alpha4.writeDigitAscii(1, 'A');
        alpha4.writeDigitAscii(2, 'I');
        alpha4.writeDigitAscii(3, 'L');
        alpha4.writeDisplay();
        client.stop();
      }
    }
    else {
      alpha4.writeDigitAscii(0, 'F');
      alpha4.writeDigitAscii(1, 'A');
      alpha4.writeDigitAscii(2, 'I');
      alpha4.writeDigitAscii(3, 'L');
      alpha4.writeDisplay();
      client.stop();
    }
  }

  if(sendUpdate) {
    digitalWrite(LED_PIN, isLEDOn);
    isLEDOn = !isLEDOn;
    stringBuffer = "";
  
    if (client.connected()) {
      jsonBuffer.clear();
      JsonObject obj = jsonBuffer.as<JsonObject>();
      if(cueCount != 0) {
        sprintf(charBuffer, "{ \"cue\": %d }", cueCount);
      }
      else {
        sprintf(charBuffer, "{ \"hlp\": %d }", cueCount);
      }
      delay(10);
      webClient.sendData(charBuffer);
      delay(10);
    }
    else if (prevCount != MAX_16)
    {
      alpha4.writeDigitAscii(0, 'F');
      alpha4.writeDigitAscii(1, 'A');
      alpha4.writeDigitAscii(2, 'I');
      alpha4.writeDigitAscii(3, 'L');
      alpha4.writeDisplay();
      
      client.stop();
      prevCount = MAX_16;
    }
    sendUpdate = false;
  }
}

void heartbeat(tc_module*) {  
  Watchdog.reset();
    
  updateFreq++;

  if(updateFreq >= 30)
  {
    updateFreq = 0;
    sendUpdate = true;
  }
}

