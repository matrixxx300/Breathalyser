/**
  @file     main.cpp
  @brief    Breathalyser
  @author   Mateusz Sznejkowski
  @version  1.0 11/02/2021
*/
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Defines
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define BUTTON_PIN 2
#define ANALOG_PIN 0
#define DELAY_TIME 100

#define OLED_MOSI 6
#define OLED_CLK 8
#define OLED_RESET 7
#define OLED_DC 4
#define OLED_CS 5
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// Globals
int TIME_UNTIL_WARMUP = 600;
unsigned long time;
int value = 0;
volatile bool skip_flag = false;

// Prototypes
void PrintTitle(void);
void PrintWarming(void);
void PrintAlcohol(int value);
void PrintAlcoholLevel(int value);
int ReadAlcohol(void);
void SkipWarmingHandler();

void setup()
{
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC))
  {
    Serial.println(F("SSD1306 allocation failed"));
    while (true)
      ;
  }

  display.display();
  delay(2000);
  display.clearDisplay();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), SkipWarmingHandler, LOW);

  PrintTitle();
  PrintWarming();
}

void loop()
{
  if (millis() % DELAY_TIME == 0)
  {
    value = ReadAlcohol();
    time = millis() / DELAY_TIME;

    if ((int)time <= TIME_UNTIL_WARMUP)
    {
      time = map(time, 0, TIME_UNTIL_WARMUP, 0, 100);
      display.drawRect(10, 50, 110, 10, WHITE);
      display.fillRect(10, 50, time, 10, WHITE);
    }
    else
    {
      PrintTitle();
      PrintAlcohol(value);
      PrintAlcoholLevel(value);
    }
    display.display();
  }
}

void PrintTitle(void)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(43, 0);
  display.println("ALKOMAT");
}

void PrintWarming(void)
{
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(22, 20);
  display.println("GRZANIE");
}

void PrintAlcohol(int value)
{
  display.setTextSize(2);
  display.setTextColor(WHITE);
  (value < 100) ? display.setCursor(51, 25) : display.setCursor(45, 25);
  display.println(value);
}

void PrintAlcoholLevel(int value)
{
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // SZEROKOŚĆ LITERY: 6 PIXELI
  if (value > 450)
  {
    display.setCursor(52, 55);
    display.println("ZGON");
  }
  else if (value >= 350)
  {
    display.setCursor(28, 55);
    display.println("MOCNO PIJANY");
  }
  else if (value >= 280)
  {
    display.setCursor(46, 55);
    display.println("PIJANY");
  }
  else if (value >= 200 && value < 280)
  {
    display.setCursor(40, 55);
    display.println("PO PIWKU");
  }
  else if (value < 200)
  {
    display.setCursor(43, 55);
    display.println("TRZEZWY");
  }
}

int ReadAlcohol(void)
{
  int value = 0;
  int val1;
  int val2;
  int val3;

  val1 = analogRead(ANALOG_PIN);
  delay(10);
  val2 = analogRead(ANALOG_PIN);
  delay(10);
  val3 = analogRead(ANALOG_PIN);

  value = (val1 + val2 + val3) / 3;
  return value;
}

void SkipWarmingHandler()
{
  if (skip_flag == false)
  {
    skip_flag = true;
    TIME_UNTIL_WARMUP = 0;
    PrintTitle();
    PrintAlcohol(value);
    PrintAlcoholLevel(value);
  }
}
