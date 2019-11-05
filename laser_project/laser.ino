//#include <GyverTM1637.h>
#include <EEPROM.h>
#include <Wire.h> 
// #include <LiquidCrystal_I2C.h>

TimerManager *timeCounter;

bool isStarted = false;
unsigned long gameTimer = 0;
unsigned int detectsCounter = 0;
const byte START_BUTTON = 2;
const byte TRIGGER_PORT = 3;

const byte TIMER_ADDRESS_1 = 0;
const byte TIMER_ADDRESS_2 = 1;

const byte DETECTS_COUNTER_ADDRESS_1 = 2;
const byte DETECTS_COUNTER_ADDRESS_2 = 3;

// will be called on each second
void countOnStart() {
    if (isStarted) {
        gameTimer = gameTimer + 1;
        saveTimer(gameTimer);
        Serial.print("TIME:" + String(gameTimer));
    }
}

void setup() {
    erase();
    timeCounter = new TimerManager(1000, *countOnStart);
    pinMode(START_BUTTON, INPUT);
    detectsCounter = loadDetects();
    gameTimer = loadTimer();
    Serial.begin(9600);
}

void loop() {
    timeCounter->loop();
    byte state = digitalRead(TRIGGER_PORT);
    byte buttonState = digitalRead(START_BUTTON);
    if (state == HIGH) {
        detectsCounter = detectsCounter + 1;
        saveDetects(detectsCounter);
        Serial.print("DETECTS:" + String(detectsCounter));
    }
    if (Serial.available()) {
        String command = Serial.readString();
        if (command == "START") {
            isStarted = true;
        }
        if (command == "STOP") {
            isStarted = false;
        }
    }
    if (buttonState != HIGH) {
        isStarted = !isStarted;
    }
}

void saveDetects(unsigned int detects) {
    byte detects1 = (detects - detects % 100) / 100;
    byte detects2 = detects % 100;
    EEPROM.write(DETECTS_COUNTER_ADDRESS_1, detects1);
    EEPROM.write(DETECTS_COUNTER_ADDRESS_2, detects1);
}

void saveTimer(unsigned long timer) {
    byte timer1 = (timer - timer % 100) / 100;
    byte timer2 = timer % 100;
    EEPROM.write(TIMER_ADDRESS_1, timer1);
    EEPROM.write(TIMER_ADDRESS_2, timer2);
}

unsigned long loadTimer() {
    byte timer1 = EEPROM.read(TIMER_ADDRESS_1);
    byte timer2 = EEPROM.read(TIMER_ADDRESS_2);

    if (timer1 > 100) {
        timer1 = 99;
    }
    
    if (timer2 > 100) {
        timer2 = 99;
    }
    unsigned long timer = timer1 * 100 + timer2;

    return timer;
}

unsigned int loadDetects() {
    byte detects1 = EEPROM.read(DETECTS_COUNTER_ADDRESS_1);
    byte detects2 = EEPROM.read(DETECTS_COUNTER_ADDRESS_2);

    if (detects1 > 100) {
        detects1 = 99;
    }
    
    if (detects2 > 100) {
        detects2 = 99;
    }
    unsigned int detects = detects1 * 100 + detects2;

    return detects;
}


void erase() {
  EEPROM.write(0, 0);
  EEPROM.write(1, 0);
  EEPROM.write(2, 0);
  EEPROM.write(3, 0);
  EEPROM.write(4, 0);
  EEPROM.write(5, 0);
  EEPROM.write(6, 0);
  EEPROM.write(7, 0);
  EEPROM.write(8, 0);
  EEPROM.write(9, 0);
  EEPROM.write(11, 0);
  EEPROM.write(12, 0);
  EEPROM.write(13, 0);
  EEPROM.write(14, 0);
  EEPROM.write(15, 0);
  EEPROM.write(16, 0);
  EEPROM.write(17, 0);
  EEPROM.write(18, 0);
  EEPROM.write(19, 0);
  EEPROM.write(20, 0);
}