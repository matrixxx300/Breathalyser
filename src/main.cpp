/**
  @file     main.cpp
  @brief    Breathalyser
  @author   Mateusz Sznejkowski
  @version  2.0 11/02/2021
*/
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ENGLISH    ->  0
// POLISH     ->  1
#define LANGUAGE 0
// CONTINUED  ->  0
// SINGLE     ->  1
#define MODE 1

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
uint16_t TIME_UNTIL_WARMUP = 300;
unsigned long time;
volatile bool timer_flag = false;

// Prototypes
void PrintTitle(void);
void PrintWarming(void);
void PrintAlcohol(uint16_t value);
void PrintAlcoholLevel(uint16_t value);
void PrintInstruction(void);
void PrintTimer(void);

uint16_t ReadAlcohol(void);
uint16_t Measure(void);

void ButtonHandler();

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

  //display.display();
  //delay(2000);

  PrintTitle();
  PrintWarming();

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  do
  {
    if (digitalRead(BUTTON_PIN) == LOW)
    {
      while (digitalRead(BUTTON_PIN) == LOW)
        ;
      delay(10);
      break;
    }
    time = millis() / 1000;
    uint16_t mapedValue = map(time, 0, TIME_UNTIL_WARMUP, 0, 110);
    display.drawRect(9, 50, 110, 10, WHITE);
    display.fillRect(9, 50, mapedValue, 10, WHITE);
    display.display();
  } while ((uint16_t)time <= TIME_UNTIL_WARMUP);

#if MODE == 1
  PrintInstruction();
#endif

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), ButtonHandler, LOW);
}

void loop()
{
#if MODE == 1
  if (timer_flag == true)
  {
    PrintTimer();
    PrintTitle();
    display.setTextSize(2);
#if LANGUAGE == 1
    display.setCursor(6, 25);
    display.print(" DMUCHAJ! ");
#else
    display.setCursor(0, 25);
    display.print("   BLOW!   ");
#endif
    display.display();
    uint16_t value = Measure();
    PrintTitle();
    PrintAlcohol(value);
    PrintAlcoholLevel(value);
    display.display();
    delay(5000);
    PrintInstruction();
    timer_flag = false;
  }
#else
  if (millis() % DELAY_TIME == 0)
  {
    uint16_t value = ReadAlcohol();
    PrintTitle();
    PrintAlcohol(value);
    PrintAlcoholLevel(value);
    display.display();
  }
#endif
}

void PrintTitle(void)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(3, 0);
#if LANGUAGE == 1
  display.println("       ALKOMAT       ");
#else
  display.println("    BREATHALYSER    ");
#endif
}

void PrintWarming(void)
{
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(6, 20);
#if LANGUAGE == 1
  display.println("NAGRZEWNIE");
#else
  display.println("WARMING UP");
#endif
}

void PrintAlcohol(uint16_t value)
{
  display.setTextSize(2);
  display.setTextColor(WHITE);
  (value < 100) ? display.setCursor(51, 25) : display.setCursor(45, 25);
  display.println(value);
}

void PrintAlcoholLevel(uint16_t value)
{
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // LETTER WIDTH: 6 PIXELS
  if (value > 450)
  {
#if LANGUAGE == 1
    display.setCursor(3, 55);
    display.println("        ZGON        ");
#else
    display.setCursor(3, 55);
    display.println("        DEAD        ");
#endif
  }
  else if (value >= 350)
  {
#if LANGUAGE == 1
    display.setCursor(3, 55);
    display.println("    MOCNO PIJANY    ");
#else
    display.setCursor(3, 55);
    display.println("     VERY DRUNK     ");
#endif
  }
  else if (value >= 280)
  {
#if LANGUAGE == 1
    display.setCursor(3, 55);
    display.println("       PIJANY       ");
#else
    display.setCursor(0, 55);
    display.println("        DRUNK        ");
#endif
  }
  else if (value >= 200 && value < 280)
  {
#if LANGUAGE == 1
    display.setCursor(3, 55);
    display.println("      PO PIWKU      ");
#else
    display.setCursor(0, 55);
    display.println("     A FEW SHOTS     ");
#endif
  }
  else if (value < 200)
  {
#if LANGUAGE == 1
    display.setCursor(0, 55);
    display.println("       TRZEZWY       ");
#else
    display.setCursor(0, 55);
    display.println("        SOBER        ");
#endif
  }
}

void PrintInstruction(void)
{
  PrintTitle();
  display.setTextSize(1);
  display.setTextColor(WHITE);
#if LANGUAGE == 1
  display.setCursor(3, 25);
  display.println(" WCISNIJ PRZYCISK I ");
  display.setCursor(0, 40);
  display.println("       DMUCHAJ       ");
#else
  display.setCursor(3, 25);
  display.println("  PRESS BUTTON AND  ");
  display.setCursor(3, 40);
  display.println("        BLOW        ");
#endif
  display.display();
}

void PrintTimer(void)
{
  for (uint8_t i = 5; i > 0; i--)
  {
    PrintTitle();
    display.setTextSize(2);
    display.setCursor(6, 25);
#if LANGUAGE == 1
    display.print("  UWAGA!  ");
#else
    display.print("ATTENTION!");
#endif
    display.setTextSize(1);
    display.setCursor(61, 55);
    display.println(i);
    display.display();
    delay(1000);
  }
}

uint16_t ReadAlcohol(void)
{
  int value = 0;
  int val1;
  int val2;
  int val3;
  int val4;
  int val5;
  int val6;

  val1 = analogRead(ANALOG_PIN);
  delay(20);
  val2 = analogRead(ANALOG_PIN);
  delay(20);
  val3 = analogRead(ANALOG_PIN);
  delay(20);
  val4 = analogRead(ANALOG_PIN);
  delay(20);
  val5 = analogRead(ANALOG_PIN);
  delay(20);
  val6 = analogRead(ANALOG_PIN);

  value = (val1 + val2 + val3 + val4 + val5 + val6) / 6;
  return value;
}

uint16_t Measure(void)
{
  uint16_t sum = 0;
  for (uint16_t i = 0; i < 50; i++)
  {
    uint16_t value = ReadAlcohol();
    // if (value > max)
    //   max = value;
    sum += value;
  }
  return sum / 50;
}

void ButtonHandler()
{
  timer_flag = true;
}
