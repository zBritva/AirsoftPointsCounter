#include <GyverTM1637.h>
#include <EEPROM.h>
#include <Wire.h> 

#define YELLOW_LED_CLK 12
#define YELLOW_LED_DIO 13

#define BLUE_LED_CLK 10
#define BLUE_LED_DIO 11

#define WAIT_SEC 1000

GyverTM1637 yellow_tm1637(YELLOW_LED_CLK, YELLOW_LED_DIO);
GyverTM1637 blue_tm1637(BLUE_LED_CLK, BLUE_LED_DIO);

int counter1;
int counter2;

unsigned long timer = 0;

void setup() {
  yellow_tm1637.displayInt(1); 
  blue_tm1637.displayInt(2);

  counter1 = 0;
  counter2 = 0;
  Serial.begin(9600);
  timer = millis();
}

void loop() {
  if (millis() - timer > WAIT_SEC) {
    timer = millis();
    eachSecond();
  }
}

void eachSecond() {
    yellow_tm1637.displayInt(counter1); 
    blue_tm1637.displayInt(counter2); 
    counter1 += 1;
    counter2 += 2;
    Serial.print(String(timer) + "\n");
}

