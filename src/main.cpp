#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <thread_manager.h>
#include <request_manager.h>

#include <GY_21.h>
#include <ChainableLED.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1   // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // If not work please try 0x3D
#define SDA D5         // Stock firmware shows wrong pins
#define SCL D6         // They swap SDA with SCL ;)

Adafruit_SSD1306 display;
GY21 sensor;
ThreadManager threadManager;
WiFiServer server(80);
RequestManager request_manager("CasaGuizzz-Camere", "abcde12345", &server);

#define DATA_PIN 7
#define CLOCK_PIN 8

ChainableLED leds(DATA_PIN, CLOCK_PIN, 1);


bool running = false;
short max_value = 100;

float hue = 0.0;

bool prev_relay = false;
bool relay = false;

int phase = 0;

void init_oled() {
  
  display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
  {
    Serial.println(F("SSD1306 allocation failed"));
    return;
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.println("OLED READY:)");
  display.println("");
  display.display();
}

// void show()
// {
//   display.fillRect(0, 0, 128, 48, SSD1306_BLACK);
//   display.setCursor(0,0);
//   display.setTextSize(2);
//   display.setTextColor(SSD1306_WHITE);
//   display.println(red);
//   display.println(green);
//   display.println(blue);
//   display.display();
// }

void shutdown()
{
    static float brightness = 0.5;

    if (brightness <= 0.0)
        return;

    brightness -= 0.01;

    if (brightness < 0.0)
        brightness = 0.0;

    leds.setColorHSB(0, hue, 1.0, brightness);
    // show();
}

void manage_led()
{
    static float brightness = 0.5;

    if (!running)
    {
        shutdown();
        return;
    }

    brightness = 0.5;

    leds.setColorHSB(0, hue, 1.0, brightness);

    hue += 0.002;

    if (hue >= 1.0)
        hue = 0.0;
}


void manage_relay()
{
  if(prev_relay == relay)
    return;

  prev_relay = relay;

  Serial.println("writing");
  display.setCursor(0, 48);
  display.fillRect(0, 48, 128, 16, SSD1306_BLACK);
  display.print(relay? "ON " : "OFF");
  display.display();
}

void manage_temp()
{
  float temp = sensor.GY21_Temperature();
  display.setCursor(64, 48);
  display.fillRect(64, 48, 128, 16, SSD1306_BLACK);
  display.print(temp);
  display.display();
}

JsonDocument set_relay(JsonDocument param)
{
    relay = !relay;
    JsonDocument ret;
    ret["relay"] = relay;
    return ret;
}

JsonDocument set_led(JsonDocument param)
{
    running = !running;
    JsonDocument ret;
    ret["led"] = running;
    return ret;
}

JsonDocument get_temp(JsonDocument param)
{
    float temp = sensor.GY21_Temperature();
    float hum = sensor.GY21_Humidity();
    JsonDocument ret;
    ret["temp"] = temp;
    ret["hum"] = hum;
    return ret;
}

void setup() 
{
  Serial.begin(9600);
  Wire.begin(D5, D6);
  init_oled();
  
  request_manager.init_request();

  request_manager.add_request("GET", "/get_temp", &get_temp);
  request_manager.add_request("POST", "/set_relay", &set_relay);
  request_manager.add_request("POST", "/set_led", &set_led);

  threadManager.add_method( &manage_led );
  threadManager.add_method( &manage_relay );
  threadManager.add_method( &manage_temp );
}

void loop() 
{
  threadManager.thread_loop();
  request_manager.handle_request();
}