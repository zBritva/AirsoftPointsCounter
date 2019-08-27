//#include <GyverTM1637.h>
#include <EEPROM.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

int yellowCount = 0;
int greenCount = 0;

const int YELLOW_LED_CLK = 4;
const int YELLOW_LED_DIO = 3; 

const int GREEN_LED_CLK = 6;
const int GREEN_LED_DIO = 5;

const int GREEN_BUTTON = 8;
const int YELLOW_BUTTON = 9;

const int SERVICE_BUTTON = 2;


const int GREEN_POINTS_ADDR_1 = 0;
const int GREEN_POINTS_ADDR_2 = 1;


const int YELLOW_POINTS_ADDR_1 = 2;
const int YELLOW_POINTS_ADDR_2 = 3;

const int TEAM_FLAG_ADDR_2 = 4;

const int CAPTURE_TIME_ADDR = 5;
const int POINT_LIMITS_ADDR_1 = 6;
const int POINT_LIMITS_ADDR_2 = 7;
const int DEBUG_MODE_ADDR = 8;


const int TIME_FOR_POINT = 9;

const int MAX_POINTS = 9999;


int yellowButtonState = LOW;
int greenButtonState = LOW;
int serviceButtonState = LOW;
int selectedTeam = 0;
int captureTime = 10;
int captureCountDown = captureTime;
int pointLimits = MAX_POINTS;
int timeForOnePoint = 0;
bool debugMode = false;
//1 -green
//2 -yellow

int timeToRest = 10;

//GyverTM1637 yellow_tm1637(YELLOW_LED_CLK, YELLOW_LED_DIO);
//GyverTM1637 green_tm1637(GREEN_LED_CLK, GREEN_LED_DIO);
LiquidCrystal_I2C lcd(0x27,16,2);


void setup() {
  pinMode(YELLOW_BUTTON, INPUT);
  pinMode(GREEN_BUTTON, INPUT);
  pinMode(SERVICE_BUTTON, INPUT);
  loadPoints();
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Hello,");
  lcd.setCursor(0,1);
  lcd.print("Airsoft player");
  if (yellowCount == 0 && greenCount == 0) {
    delay(1500);
    lcd.setCursor(0,0);
    lcd.print("Made by: zBritva");
    lcd.setCursor(0,1);
    lcd.print("github.com/zBritva");
    delay(1500);
    lcd.scrollDisplayLeft();
    delay(700);
    lcd.scrollDisplayLeft();
    delay(1500);
  }
  lcd.clear();
}

void loop() {
//показываем текущий счетчик, если только не идет захват
if (captureCountDown == captureTime) { 
  lcd.setCursor(0,0);
  lcd.print("YELLOW: " + String(yellowCount) + "      ");
  lcd.setCursor(0,1);
  lcd.print("GREEN : " + String(greenCount) + "      ");
}

  yellowButtonState = LOW;
  greenButtonState = LOW;
  serviceButtonState = LOW;
  yellowButtonState = digitalRead(YELLOW_BUTTON);
  greenButtonState = digitalRead(GREEN_BUTTON);
  serviceButtonState = digitalRead(SERVICE_BUTTON);
  if (debugMode) {
    Serial.print("greenButtonState\n");
    Serial.print(greenButtonState);
    Serial.print("\n");
    
    Serial.print("yellowButtonState\n");
    Serial.print(yellowButtonState);
    Serial.print("\n");
    
    Serial.print("serviceButtonState\n");
    Serial.print(serviceButtonState);
    Serial.print("\n");
  }

  // проверяем состояние кнопок
  // если нажата кнопка желтых, но при этом не нажата кнопка зеленых
  if (yellowButtonState == HIGH && greenButtonState == LOW) {
    // если счетчик захвата 0, то точка захвачена
    if (captureCountDown <= 0) {
      selectedTeam = 2;
      captureCountDown = captureTime;
    } else if (selectedTeam != 2) { // проверяем что точка не занято желтыми (захват захваченной точки не имеет смысла)
      // чистим дисплей перед началом отчета
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("   CAPTURING    ");
      lcd.setCursor(0,1);
      lcd.print(captureCountDown);
      captureCountDown--; // уменьшаем счетчик захвата
      if (debugMode) {
        Serial.print("captureCountDown " + String(captureCountDown) + "\n");
      }
    }
    if (debugMode) {
      Serial.print("selectedTeam2 (green)\n");
    }
  }
  else
  // аналогично для желтых
  if (greenButtonState == HIGH && yellowButtonState == LOW ) {
    if (captureCountDown <= 0) {
      selectedTeam = 1;
      captureCountDown = captureTime;
    } else if (selectedTeam != 1) {
    // чистим дисплей перед началом отчета
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("   CAPTURING    ");
      lcd.setCursor(0,1);
      lcd.print(captureCountDown);
      captureCountDown--;
      if (debugMode) {
        Serial.print("captureCountDown " + String(captureCountDown) + "\n");
      }
    }
    
    if (debugMode) {
      Serial.print("selectedTeam1 (yellow)\n");
    }
  } else {
    // если не одна кнопка не нажата, то сбрасываем счетчик захвата
    captureCountDown = captureTime;
    if (debugMode) {
      Serial.print("Capturing was reset\n");
    }
  }

  // быстрый рестарт (нужно удерживать 2 кнопки одновременно более 15 секунд)
  if (yellowButtonState == HIGH && greenButtonState == HIGH) {
    timeToRest--;
    if (timeToRest == 0) {
      selectedTeam = 0;
      yellowCount = 0;
      greenCount = 0;
    }
  } else {
    timeToRest = 15;
  }

  // если не происходит захват точки, то прибавляем очки команде
  if (greenCount < pointLimits && yellowCount < pointLimits && captureCountDown == captureTime) {
    // если выбрана текущая команда 2 (желтые)
    if (selectedTeam == 2) {
      yellowCount++;
      if (debugMode) {
        Serial.print("yellowCount++\n");
        Serial.print(yellowCount);
        Serial.print("\n");
      }
      // сохраняем очки каждые 10 секунд
      if (yellowCount % 10 == 0) {
        savePoints(greenCount, yellowCount, selectedTeam);
      }
    }
    // если выбрана текущая команда 1 (зеленые)
    if (selectedTeam == 1) {
      greenCount++;
      if (debugMode) {
        Serial.print("greenCount++\n");
        Serial.print(greenCount);
        Serial.print("\n");
      }
      // сохраняем очки каждые 10 секунд
      if (greenCount % 10 == 0) {
        savePoints(greenCount, yellowCount, selectedTeam);
      }
    }
  }
  
  // если команда набрала необходимое количество очков, то останавливаем счетчик
  if (greenCount >= pointLimits || yellowCount >= pointLimits) {
    // смотрим у кого больше очков и выводим на экран победителя
    if (greenCount > yellowCount) {
      lcd.setCursor(0,0);
      lcd.print("GREEN WON: " + String(greenCount));
      lcd.setCursor(0,1);
      lcd.print("YELLOW   : " + String(yellowCount));
    } else {
      lcd.setCursor(0,0);
      lcd.print("YELLOW WON: " + String(yellowCount));
      lcd.setCursor(0,1);
      lcd.print("GREEN     : " + String(greenCount));
    }
    // Сохраняем очки в постоянной памяти (после сохранения сбой в питании не влияет на результат)
    savePoints(greenCount, yellowCount, selectedTeam);
  }

  Serial.flush();
  delay(1000);

  // если нажата сервисная кнопка на плате, то выводим описание настроек в серийный порт (вывод данных на PC или телефон через bluetooth)
  if (serviceButtonState == HIGH) {
    help();
  }

  // чтение данных из серийного порта, можно подавать команды через PC или из телефона при пожключении по bluetooth.
  if (Serial.available()) {
    String command = Serial.readString();
    if (debugMode) {
      Serial.print("Received:" + command);
    }
    readConfigurationFromSerialPort(command);
  }
}

void help() {
  Serial.print("Задать время захвата: -c N\n");
  Serial.print("Задать лимит очков  : -l N\n");
  Serial.print("Перезапустить счет  : -r\n");
  Serial.print("Отладка             : -d 0/1\n");
}

void currentSettings() {
  Serial.print("c=" + String(captureTime) + "\n");
  Serial.print("l=" + String(pointLimits) + "\n");
  Serial.print("d=" + String(debugMode) + "\n");
}

String getProgress(int countDown, int captureTime) {
  countDown = captureTime - countDown;
  int segments = round(16 / 100 * (round(countDown / captureTime * 100))) + 0;
  Serial.print("segments" + String(segments) + "\n");
  String result;
  for (int i = 0; i < segments; i++) {
    result += "#";
  }
  Serial.print("PROGRESS BAR\n");
  Serial.print(result + "\n");
  return result;
}
