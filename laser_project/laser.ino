//#include <GyverTM1637.h>
#include <EEPROM.h>
#include <Wire.h> 
// #include <LiquidCrystal_I2C.h>

TimerManager *timeCounter;

bool isStarted = false;
bool isLaserDetect = false;
bool isLaserDetectReset = true;
bool isLaserOn = false;
byte laserState = 0;
unsigned long gameTimer = 0;
unsigned int detectsCounter = 0;
unsigned long sensetive = 500;
unsigned long laserTime = 0;

const byte START_BUTTON = 2;
const byte TRIGGER_PORT = 3;
const byte LASER_POWER = 5;

const byte TIMER_ADDRESS_1 = 0;
const byte TIMER_ADDRESS_2 = 1;

const byte DETECTS_COUNTER_ADDRESS_1 = 2;
const byte DETECTS_COUNTER_ADDRESS_2 = 3;

const byte SENSETIVE_ADDRESS_1 = 3;
const byte SENSETIVE_ADDRESS_2 = 4;

// will be called on each second
void countOnStart() {
    if (isStarted) {
        gameTimer = gameTimer + 1;
        saveTimer(gameTimer);
        Serial.print("TIME:" + String(gameTimer)+"\n");
    }
}

void onLaserDetected() {
    if (isLaserDetect) {
        unsigned long laser = millis() - laserTime;
        if (laser > sensetive) {
            detectsCounter = detectsCounter + 1;
            isLaserDetectReset = false;
        }
        Serial.print("DETECT TIME "+String(laser)+" CNT:" + String(detectsCounter)+ "\n");
    }
}

void setup() {
    timeCounter = new TimerManager(1000, *countOnStart);
    pinMode(START_BUTTON, INPUT);
    pinMode(TRIGGER_PORT, INPUT);
    pinMode(LASER_POWER, OUTPUT);
    detectsCounter = loadDetects();
    gameTimer = loadTimer();
    sensetive = loadSensetive();
    Serial.begin(9600);
    Serial.print("SENSETIVE:"+String(sensetive) + "\n");
}

void loop() {
    timeCounter->loop();
    // byte state = digitalRead(TRIGGER_PORT);
    // if (state == HIGH) {
    //     detectsCounter = detectsCounter + 1;
    //     saveDetects(detectsCounter);
    //     Serial.print("DETECTS:" + String(detectsCounter) + "\n");
    // }
    // read command from bluetooth
    if (Serial.available()) {
        String command = Serial.readString();
        if (command.indexOf("RESTART") > -1) {
            timeCounter = 0;
            detectsCounter = 0;
            saveDetects(detectsCounter);
            saveTimer(gameTimer);
            Serial.print("RESTART\n");
        }
        if (command.indexOf("START") > -1) {
            isStarted = true;
            isLaserOn = true;
            Serial.print("START\n");
        }
        if (command.indexOf("LASERON") > -1) {
            isLaserOn = true;
            Serial.print("LASERON\n");
        }
        if (command.indexOf("STOP") > -1) {
            isStarted = false;
            Serial.print("STOP\n");
        }
        if (command.indexOf("SENSETIVE:") > -1) {
            command.replace("SENSETIVE:", "");
            sensetive = command.toInt();
            saveSensetive(sensetive);
            Serial.print("SENSETIVE:" + String(sensetive) + "\n");
        }
    }
    // check start/stop button
    byte buttonState = digitalRead(START_BUTTON);
    if (buttonState != HIGH) {
        // isStarted = !isStarted;
    }
    // check laser detects
    laserState = digitalRead(TRIGGER_PORT);
    if (laserState == HIGH && isStarted && isLaserDetectReset) {
        // save first detect time
        if (!isLaserDetect) {
            Serial.print("REGISTER DETECT\n");
            isLaserDetect = true;
            laserTime = millis();
        }
        onLaserDetected();
    } else {
        isLaserDetect = false;
    }
    if (laserState != HIGH) {
        Serial.print("REGISTER RESET\n");
        isLaserDetectReset = true;
    }
    if (isLaserOn) {
        digitalWrite(LASER_POWER, HIGH);
    }
}

void saveDetects(unsigned int detects) {
    byte detects1 = (detects - detects % 100) / 100;
    byte detects2 = detects % 100;
    EEPROM.write(DETECTS_COUNTER_ADDRESS_1, detects1);
    EEPROM.write(DETECTS_COUNTER_ADDRESS_2, detects2);
}

void saveSensetive(int sensetive) {
    byte sensetive1 = (sensetive - sensetive % 100) / 100;
    byte sensetive2 = sensetive % 100;
    EEPROM.write(SENSETIVE_ADDRESS_1, sensetive1);
    EEPROM.write(SENSETIVE_ADDRESS_2, sensetive2);
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

int loadSensetive() {
    byte sensetive1 = EEPROM.read(SENSETIVE_ADDRESS_1);
    byte sensetive2 = EEPROM.read(SENSETIVE_ADDRESS_2);

    if (sensetive1 > 100) {
        sensetive1 = 99;
    }
    
    if (sensetive2 > 100) {
        sensetive2 = 99;
    }
    int sensetive = sensetive1 * 100 + sensetive2;

    return sensetive;
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