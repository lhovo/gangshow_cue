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
IPAddress ip(192, 168, 1, 250);
WebSocketClient webClient;
EthernetClient client;

#define WS_PORT 8888
#define WS_PATH "/hardware"
#define WS_HOST "192.168.1.200"

char server[] = WS_HOST;
#define LED_PIN 13
#define CONF_TC_MODULE TC3

#define LED_RED 12
#define LED_ORANGE 11
#define LED_GREEN 10
#define BUTTON_RED 9
#define BUTTON_ORANGE 6 
#define BUTTON_GREEN 5

unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
// Variables will change:
unsigned long lastDebounceTimeRed = 0;  // the last time the output pin was toggled
int buttonStateRed = HIGH;       // the current reading from the input pin
int lastButtonStateRed = HIGH;   // the previous reading from the input pin
unsigned long lastDebounceTimeOrange = 0;  // the last time the output pin was toggled
int buttonStateOrange = HIGH;       // the current reading from the input pin
int lastButtonStateOrange = HIGH;   // the previous reading from the input pin
unsigned long lastDebounceTimeGreen = 0;  // the last time the output pin was toggled
int buttonStateGreen = HIGH;       // the current reading from the input pin
int lastButtonStateGreen = HIGH;   // the previous reading from the input pin
bool inStandby = false, prevStandby = false;

#define NUM_OFFSET 48
#define MAX_16 0xFFFFFF
uint16_t cueCount = 0, prevCount = MAX_16;
uint16_t updateFreq = 0, ledFreq = 0;
bool sendUpdate = false, ledUpdate = false;
bool isLEDOn = false, isDbgLEDOn;

String stringBuffer;
char charBuffer[1024];
StaticJsonDocument<1024> jsonBuffer;

void setup() {
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_ORANGE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_ORANGE, LOW);
  digitalWrite(LED_GREEN, LOW);

  // Input pins
  pinMode(BUTTON_RED, INPUT);
  pinMode(BUTTON_ORANGE, INPUT);
  pinMode(BUTTON_GREEN, INPUT);
  
  alpha4.begin(0x70);  // pass in the address
  alpha4.setBrightness(1); // 1-15

  alpha4.writeDigitRaw(0, 0xFFFF);
  alpha4.writeDigitRaw(1, 0xFFFF);
  alpha4.writeDigitRaw(2, 0xFFFF);
  alpha4.writeDigitRaw(3, 0xFFFF);
  alpha4.writeDisplay();

  // start the Ethernet connection:
  Ethernet.init(A5);
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  Watchdog.enable(8192);
  
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
  webClient.path = WS_PATH;
  webClient.host = WS_HOST;
  
  client.connect(webClient.host, WS_PORT);
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
  else
  {
    alpha4.writeDigitAscii(0, 'F');
    alpha4.writeDigitAscii(1, 'A');
    alpha4.writeDigitAscii(2, 'I');
    alpha4.writeDigitAscii(3, 'L');
    alpha4.writeDisplay();
    while(true) {delay(300);}
  }

  alpha4.clear();
  alpha4.writeDigitRaw(0, 0xFFFF);
  alpha4.writeDisplay();
  digitalWrite(LED_RED, HIGH);
  delay(200);
  alpha4.writeDigitRaw(0, 0x0);
  alpha4.writeDigitRaw(1, 0xFFFF);
  alpha4.writeDisplay();
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_ORANGE, HIGH);
  delay(200);
  alpha4.writeDigitRaw(1, 0x0);
  alpha4.writeDigitRaw(2, 0xFFFF);
  alpha4.writeDisplay();
  digitalWrite(LED_ORANGE, LOW);
  digitalWrite(LED_GREEN, HIGH);
  delay(200);
  alpha4.writeDigitRaw(2, 0x0);
  alpha4.writeDigitRaw(3, 0xFFFF);
  alpha4.writeDisplay();
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN, LOW);
  alpha4.clear();
  alpha4.writeDisplay();
  Watchdog.reset();

  alpha4.writeDigitAscii(0, '/');
  alpha4.writeDigitAscii(1, '/');
  alpha4.writeDigitAscii(2, '/');
  alpha4.writeDigitAscii(3, '/');
  alpha4.writeDisplay();

   // Enable pullups
  digitalWrite(BUTTON_RED, HIGH);
  digitalWrite(BUTTON_ORANGE, HIGH);
  digitalWrite(BUTTON_GREEN, HIGH);

  attachInterrupt(BUTTON_RED, btn_red, CHANGE);
  attachInterrupt(BUTTON_ORANGE, btn_orange, CHANGE);
  attachInterrupt(BUTTON_GREEN, btn_green, CHANGE);

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

void debounce() {
  if ((millis() - lastDebounceTimeRed) > debounceDelay) {
    // if the button state has changed:
    if (lastButtonStateRed != buttonStateRed) {
      buttonStateRed = lastButtonStateRed;
      if(!buttonStateRed && client.connected() && cueCount > 1) {
        cueCount--;
        inStandby = false;
      }
    }
  }

  if ((millis() - lastDebounceTimeOrange) > debounceDelay) {
    // if the button state has changed:
    if (lastButtonStateOrange != buttonStateOrange) {
      buttonStateOrange = lastButtonStateOrange;

      if(!buttonStateOrange && client.connected()) {
        inStandby = true;
      }
      
    }
  }

  if ((millis() - lastDebounceTimeGreen) > debounceDelay) {
    // if the button state has changed:
    if (lastButtonStateGreen != buttonStateGreen) {
      buttonStateGreen = lastButtonStateGreen;

      if(!buttonStateGreen && client.connected()) {
        cueCount++;
        inStandby = false;
      }
    }
  }
}

void btn_red() {
  int buttonReading = digitalRead(BUTTON_RED);
  if(buttonReading != lastButtonStateRed){
    lastDebounceTimeRed = millis();
  }
  
  lastButtonStateRed = buttonReading;
}

void btn_orange() {
  int buttonReading = digitalRead(BUTTON_ORANGE);
  if(buttonReading != lastButtonStateOrange){
    lastDebounceTimeOrange = millis();
  }
  
  lastButtonStateOrange = buttonReading;
}

void btn_green() {
  int buttonReading = digitalRead(BUTTON_GREEN);
  if(buttonReading != lastButtonStateGreen){
    lastDebounceTimeGreen = millis();
  }
  
  lastButtonStateGreen = buttonReading;
}

void loop() {
  debounce();
  
  if(client.connected() && ((cueCount != prevCount) || (inStandby != prevStandby) ) ) {
    prevCount = cueCount;
    prevStandby = inStandby;
    
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

    if(inStandby) {
      alpha4.writeDigitAscii(0, 'X');
    }
    alpha4.writeDisplay();

    // Advance the next data send
    updateFreq = 0;
    sendUpdate = true;
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
        int update = root["update"];
        cueCount = update;
      }
    }
  }
  else {
    prevCount = MAX_16;
    client.stop();
    delay(10);
    client.connect(webClient.host, WS_PORT);
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

  if(ledUpdate) {
    isLEDOn = !isLEDOn;
    if(inStandby) {
      digitalWrite(LED_GREEN, isLEDOn);
      digitalWrite(LED_ORANGE, LOW);
    }
    else {
      digitalWrite(LED_ORANGE, isLEDOn);
      digitalWrite(LED_GREEN, LOW);
    }
    ledUpdate = false;
  }

  if(sendUpdate) {
    isDbgLEDOn = !isDbgLEDOn;
    digitalWrite(LED_PIN, isDbgLEDOn);
  
    if (client.connected()) {
      if(cueCount > 0) {
        sprintf(charBuffer, "{\"cue\":%d,\"standby\":%d}", cueCount, inStandby);
      }
      else {
        sprintf(charBuffer, "{\"hlp\":%d}", cueCount);
      }
      webClient.sendData(F(charBuffer));
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
  ledFreq++;

  if(updateFreq >= 120)
  {
    updateFreq = 0;
    sendUpdate = true;
  }
  if(ledFreq >= 15)
  {
    ledFreq = 0;
    ledUpdate = true;
  }
}

