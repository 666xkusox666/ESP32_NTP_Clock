#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
//---------------------------------------ST7796S Test START---------------------------------------// 
#include <Adafruit_GFX.h>
#include <Adafruit_ST7796S.h>
#include <Fonts/FreeSansBold18pt7b.h> // A custom font

// Define display pin connections
#define TFT_CS        5
#define TFT_RST       17 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC        16

// Define Push Button Pin
#define BUTTON_PIN    33

// Initialize the display
Adafruit_ST7796S tft = Adafruit_ST7796S(TFT_CS, TFT_DC, TFT_RST);

// --- BGR Panel Color Definitions ---
#define COLOR_RED    0x001F  // RED on BGR panels
#define COLOR_GREEN  0x07E0  // GREEN
#define COLOR_BLUE   0xF800  // BLUE on BGR panels
#define COLOR_YELLOW 0x07FF  // RED + GREEN on BGR panels
#define COLOR_BLACK  0x0000
#define COLOR_WHITE  0xFFFF

#define PAUSE 1000  // Delay (milliseconds) between examples
uint8_t rotate = 0; // Current screen orientation (0-3)
#define CORNER_RADIUS 0
//---------------------------------------ST7796S Test END---------------------------------------// 

const char* ssid       = "SSID";
const char* password   = "password";

WiFiUDP ntpUDP;
// GMT+8 offset = 8 * 3600 = 28800 seconds
NTPClient timeClient(ntpUDP, "pool.ntp.org", 28800); //Malaysia Time

//---------------------------------------RTC_DS3231 Test START---------------------------------------// 
const char* daysOfWeek[] = {
  " SUNDAY  ", 
  " MONDAY  ", 
  " TUESDAY ", 
  "WEDNESDAY", 
  "THURSDAY ", 
  " FRIDAY  ", 
  "SATURDAY "
};

#include "RTClib.h"
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {
  " SUNDAY  ", 
  " MONDAY  ", 
  " TUESDAY ", 
  "WEDNESDAY", 
  "THURSDAY ", 
  " FRIDAY  ", 
  "SATURDAY "
};
//---------------------------------------RTC_DS3231 Test END---------------------------------------// 


void setup() {
  Serial.begin(115200);

  // Initialize Push Button Pin with Internal Pull-Up
  pinMode(BUTTON_PIN, INPUT_PULLUP);

//---------------------------------------ST7796S Test START---------------------------------------// 
  tft.init(320, 480);
  tft.setRotation(0);        // Portrait mode
  tft.invertDisplay(true);   // IPS Panel Color Correction
  tft.fillScreen(COLOR_BLACK);

  // Display UI Header
  tft.setTextSize(3);
  tft.setTextColor(COLOR_YELLOW, COLOR_BLACK);
  tft.setCursor(20, 40);
  tft.println("ESP32 NTP CLOCK");

  tft.drawFastHLine(10, 80, 300, COLOR_WHITE);
//---------------------------------------ST7796S Test END---------------------------------------// 

//---------------------------------------RTC_DS3231 Test START---------------------------------------// 
#ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
//---------------------------------------RTC_DS3231 Test END---------------------------------------// 

//---------------------------------------WiFi Test START---------------------------------------// 
  tft.setTextSize(2);
  tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
  tft.setCursor(20, 100);
  tft.print("Connecting WiFi...");

  Serial.print("[WiFi] Connecting to ");
  Serial.println(ssid);

  //manualSyncWithNTP();

  WiFi.begin(ssid, password);
  int tryDelay = 100;
  int numberOfTries = 50;

  while (true) {
      switch(WiFi.status()) {
        case WL_NO_SSID_AVAIL:
          Serial.println("[WiFi] SSID not found");
          tft.setCursor(20, 100);
          tft.print("SSID not found....");
          break;
        case WL_CONNECT_FAILED:
          Serial.print("[WiFi] Failed - WiFi not connected!");
          tft.setCursor(20, 100);
          tft.print("WiFi not connected");
          return;
          break;
        case WL_CONNECTION_LOST:
          Serial.println("[WiFi] Connection was lost");
          tft.setCursor(20, 100);
          tft.print("WiFi LOST.........");
          break;
        case WL_DISCONNECTED:
          Serial.println("[WiFi] WiFi is disconnected");
          tft.setCursor(20, 100);
          tft.print("WiFi DISCONNECTED.");
          break;
        case WL_CONNECTED:
          Serial.println("[WiFi] WiFi is connected!");
          tft.setCursor(20, 100);
          tft.print("WiFi is connected!");
          Serial.print("[WiFi] IP address: ");
          Serial.println(WiFi.localIP());
          Serial.println("Initiate syncRTCWithNTP()");
          syncRTCWithNTP();  //Sync DS3231 to NTP
          Serial.println("END syncRTCWithNTP()");
          return;
          break;
        default:
          break;
      }
      delay(tryDelay);
      
      if(numberOfTries <= 0){
        Serial.print("[WiFi] Failed to connect to WiFi!");
        WiFi.disconnect();
        return;
      } else {
        numberOfTries--;
      }
  }
  
//---------------------------------------WiFi Test END---------------------------------------// 
  
  tft.fillRect(10, 95, 300, 30, COLOR_BLACK); // Clear status message
  timeClient.begin();
  timeClient.setTimeOffset(0); 
}

void loop() {
  // --- Check Button Press (Active LOW) ---
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50); // Basic software debounce
    if (digitalRead(BUTTON_PIN) == LOW) {
      manualSyncWithNTP();
      // Wait for button release to prevent multiple instant triggers
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }
    }
  }

  DateTime now = rtc.now();
  DateTime future (now + TimeSpan(0,0,0,0));  //Offset +0s 

  if (WiFi.status() == WL_CONNECTED) {
    //timeClient.update();
    //syncRTCWithNTP();
    tft.setCursor(20, 100);
    tft.setTextColor(COLOR_RED, COLOR_BLACK);
    tft.setTextSize(2);    
    tft.print("WiFi is connected!");    
    Serial.println(timeClient.getFormattedTime());
  } else {
    tft.setCursor(20, 100);
    tft.setTextColor(COLOR_RED, COLOR_BLACK);
    tft.setTextSize(2);
    tft.print("WiFi DisConnected!");
    Serial.println(now.second(), DEC);  
  }
      
  unsigned long epochTime = timeClient.getEpochTime();
  time_t rawTime = (time_t)epochTime;
  struct tm *ti;
  ti = localtime(&rawTime);

  // --- Date ---
  tft.setTextSize(3);
  tft.setTextColor(COLOR_YELLOW, COLOR_BLACK);
  tft.setCursor(60, 150);
  tft.print(future.year(), DEC); 
  if(future.month()<10)tft.print("-0"); 
  if(future.month()>=10)tft.print("-");    
  tft.print(future.month()); 
  if(future.day()<10)tft.print("-0"); 
  if(future.day()>=10)tft.print("-");  
  tft.print(future.day(), DEC); 
  
  // --- Time ---
  tft.setTextSize(4);
  tft.setTextColor(COLOR_GREEN, COLOR_BLACK);
  tft.setCursor(60, 230);
  if(now.hour()<10)tft.print("0");  
  tft.print(now.hour(), DEC); 
  tft.print(':'); 
  if(now.minute()<10)tft.print("0");  
  tft.print(now.minute(), DEC); 
  tft.print(':'); 
  if(now.second()<10)tft.print("0");   
  tft.print(now.second(), DEC);    
  if(now.second()>=10)tft.print(" ");   

  // --- Day of Week ---
  tft.setTextSize(3);
  tft.setTextColor(COLOR_RED, COLOR_BLACK);
  tft.setCursor(80, 310);
  tft.print(daysOfTheWeek[now.dayOfTheWeek()]); 

  if (WiFi.status() == WL_CONNECTION_LOST) {
    tft.setCursor(20, 100);
    tft.setTextColor(COLOR_RED, COLOR_BLACK);
    tft.setTextSize(2);
    tft.print("WiFi DisConnected!");
  }
}

// Regular NTP synchronization function
void syncRTCWithNTP() {
  if (WiFi.status() == WL_CONNECTED) {
    if (timeClient.forceUpdate()) {
      unsigned long epochTime = timeClient.getEpochTime();
      rtc.adjust(DateTime(epochTime));
      Serial.println("RTC successfully synchronized with NTP time!");
    } else {
      Serial.println("NTP update failed. RTC time remains unchanged.");
    }
  } else {
    Serial.println("Cannot sync RTC: No Wi-Fi connection.");
  }
}

// New Button Trigger Function to Reconnect WiFi, Get NTP Time, and Update RTC
void manualSyncWithNTP() {
  Serial.println("[Button] Manual sync requested...");
  tft.fillRect(10, 95, 300, 30, COLOR_BLACK);
  tft.setCursor(20, 100);
  tft.setTextSize(2);
  tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
  tft.print("Manual Sync...");

  // If WiFi is disconnected, attempt to reconnect 
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[Button] Reconnecting to WiFi...");
    tft.fillRect(10, 95, 300, 30, COLOR_BLACK);
    tft.setCursor(20, 100);
    tft.print("Reconnecting WiFi.");
    
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      attempts++;
    }
  }

  // If connected, sync NTP to RTC
  if (WiFi.status() == WL_CONNECTED) {
    tft.fillRect(10, 95, 300, 30, COLOR_BLACK);
    tft.setCursor(20, 100);
    tft.setTextColor(COLOR_GREEN, COLOR_BLACK);
    tft.print("Syncing with NTP...");
    
    syncRTCWithNTP();
    
    delay(1000);
    tft.fillRect(10, 95, 300, 30, COLOR_BLACK);
    tft.setCursor(20, 100);
    tft.print("Sync Complete!    ");
    delay(1500);
    tft.fillRect(10, 95, 300, 30, COLOR_BLACK);
  } else {
    tft.fillRect(10, 95, 300, 30, COLOR_BLACK);
    tft.setCursor(20, 100);
    tft.setTextColor(COLOR_RED, COLOR_BLACK);
    tft.print("Sync Failed (WiFi)");
    delay(1500);
    tft.fillRect(10, 95, 300, 30, COLOR_BLACK);
  }
}
