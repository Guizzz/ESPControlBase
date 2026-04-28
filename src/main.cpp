#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "ThreadManager.h"

#include "GY_21.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1   // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // If not work please try 0x3D
#define OLED_SDA D5         // Stock firmware shows wrong pins
#define OLED_SCL D6         // They swap SDA with SCL ;)

Adafruit_SSD1306 display;
GY21 sensor;
ThreadManager threadManager;

bool running = false;
short max_value = 100;

short red = 0;
short green = 0;
short blue = 0;

bool prev_relay = false;
bool relay = false;

int phase = 0;

void init_oled() {
  // OLED used nonstandard SDA and SCL pins
  Wire.begin(D5, D6);
  display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    return;
  }

  display.fillRect(0, 0, 128, 48, SSD1306_BLACK);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.println("OLED READY:)");
  display.println("");
  display.display();
}

void handleSerial() {
  if (!Serial.available()) 
    return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  
  Serial.print("recived: ");
  Serial.println(cmd);

  if (cmd == "start") {
    running = true;
  } else if (cmd == "stop") {
    running = false;
  } else if ( cmd == "on"){
    relay = true;
  } else if ( cmd == "off"){
    relay = false;
  }

}

void show()
{
  display.fillRect(0, 0, 128, 48, SSD1306_BLACK);
  display.setCursor(0,0);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.println(red);
  display.println(green);
  display.println(blue);
  display.display();
}

void shutdown()
{
  if(red ==  0 && green == 0 && blue == 0)
    return;
    
  if (red > 0)
    red --;
  if (green > 0)
    green --;
  if (blue > 0)
    blue --;
  
  show();

}

void manage_led()
{
  if (!running) 
    return shutdown();

  switch (phase) 
  {
    case 0: // red++
      if (red >= max_value)
      {
        phase = 1;
        break;
      }
      red++;
      break;
    
    case 1: // blue--
      if (blue <= 0) 
      {
        phase = 2;
        break;
      }
      
      blue--;
      break;
    
    case 2: // green++
      if (green >= max_value) 
      {
        phase = 3;
        break;
      }  
      green++;
      break;

      
    case 3: // red--
      if (red <= 0) 
      {
        phase = 4;
        break;
      }
      red--;
      break;

    case 4: // blue++
      if (blue >= max_value) 
      {
        phase = 5;
        break;
      }
      blue++;
      break;
      
    case 5: // green--
      if (green <= 0) 
      {
        phase = 0;
        break;
      }
      green--;
      break;

    }
        
  show();
}

void manage_relay()
{
  if(prev_relay == relay)
    return;

  display.setCursor(0, 48);
  display.fillRect(0, 48, 128, 16, SSD1306_BLACK);
  display.print(relay? "ON " : "OFF");
  display.display();
}

void setup() {
  Serial.begin(9600);
  init_oled();
  threadManager.add_method( &manage_led );
  threadManager.add_method( &manage_relay );
}

void loop() {
  handleSerial();
  threadManager.thread_loop();
}