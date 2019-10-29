//#include <GyverTM1637.h>
#include <EEPROM.h>
#include <Wire.h> 
/*
Возможно нужно пофиксить либу LiquidCrystal_I2C функция должна возвращать 1 а не 0
inline size_t LiquidCrystal_I2C::write(uint8_t value) {
 send(value, Rs);
 return 0;
}
*/
#include <LiquidCrystal_I2C.h>
#include "const.h"

short yellowCount = 0;
short greenCount = 0;

byte yellowButtonState = LOW;
byte greenButtonState = LOW;
byte serviceButtonState = LOW;
byte selectedTeam = 0; // текущая выбранная команда
//1 -green
//2 -yellow
byte captureTime = DEFAULT_CAPTURE_TIME; // время захвата точки
short timeForPoint = DEFAULT_TIME_FOR_POINT; // сколько секунд нужно удерживать точку для получения очков
byte pointsStep = 1; // количество очков за еденицу времени (timeToPoint)
byte captureCountDown = captureTime;
short pointLimits = MAX_POINTS;
bool debugMode = false;
short currentTimeForPoint = timeForPoint;

// какую информацию нужно показывать
// кривая реализация карусели
const byte infoDisplayCOUNT = 2;
byte infoDisplayTime[] = { 5, 15, 15 };
byte infoDisplayTimeCounter[] = { 5, 15, 15 };
byte currentInfo = 0;
// 0 -текущие очки 
// 1 -время до получения 1 единцы очков 
// 2 -резерв

byte timeToRest = 10;

//GyverTM1637 yellow_tm1637(YELLOW_LED_CLK, YELLOW_LED_DIO);
//GyverTM1637 green_tm1637(GREEN_LED_CLK, GREEN_LED_DIO);
LiquidCrystal_I2C lcd(0x27,16,2);


void setup() {
  // erase();
  pinMode(YELLOW_BUTTON, INPUT);
  pinMode(GREEN_BUTTON, INPUT);
  pinMode(SERVICE_BUTTON, INPUT);
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
  loadPoints();
  Serial.print("Setup done\n");
}

void loop() {
  // Serial.print("-1currentTimeForPoint" + String(currentTimeForPoint) + "\n");
  //показываем текущий счетчик, если только не идет захват
  if (captureCountDown == captureTime) {
    // какой счет показывать
    if (currentInfo == 0  || timeForPoint <= 3) {
      lcd.setCursor(0,0);
      lcd.print("YELLOW: " + String(yellowCount) + "      ");
      lcd.setCursor(0,1);
      lcd.print("GREEN : " + String(greenCount) + "      ");
    }
    // вермя до захвата показываем, если только точка захвачена
    if (currentInfo == 1 && selectedTeam != 0 && timeForPoint > 3) {
      lcd.setCursor(0,0);
      lcd.print("TIME TO POINT   ");
      lcd.setCursor(0,1);
      lcd.print("" + String(currentTimeForPoint) + "               ");
    }
    infoDisplayTimeCounter[currentInfo]--;
    if (infoDisplayTimeCounter[currentInfo] <= 0) {
      infoDisplayTimeCounter[currentInfo] = infoDisplayTime[currentInfo];
      currentInfo++;
      infoDisplayTimeCounter[currentInfo] = infoDisplayTime[currentInfo];
    }
    if (currentInfo == infoDisplayCOUNT) {
      currentInfo = 0;
    }
  }

  yellowButtonState = LOW;
  greenButtonState = LOW;
  serviceButtonState = LOW;
  yellowButtonState = digitalRead(YELLOW_BUTTON);
  greenButtonState = digitalRead(GREEN_BUTTON);
  serviceButtonState = digitalRead(SERVICE_BUTTON);

  // проверяем состояние кнопок
  // если нажата кнопка желтых, но при этом не нажата кнопка зеленых
  if (yellowButtonState == HIGH && greenButtonState == LOW) {
    // если счетчик захвата 0, то точка захвачена
    if (captureCountDown <= 0) {
      // сбросит текущий счетчик времени для получения очков, если другая команда захватила точку
      if (selectedTeam == 1) {
        currentTimeForPoint = timeForPoint;
      }
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
    }
  }
  else
  // аналогично для желтых
  if (greenButtonState == HIGH && yellowButtonState == LOW ) {
    if (captureCountDown <= 0) {
      // сбросит текущий счетчик времени для получения очков, если другая команда захватила точку
      if (selectedTeam == 2) {
        currentTimeForPoint = timeForPoint;
      }
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
    }
  } else {
    // если не одна кнопка не нажата, то сбрасываем счетчик захвата
    captureCountDown = captureTime;
  }

  // сброс захвата (нужно удерживать 2 кнопки одновременно более 10 секунд)
  if (yellowButtonState == HIGH && greenButtonState == HIGH) {
    timeToRest--;
    lcd.setCursor(0,0);
    lcd.print("RESETING        ");
    lcd.setCursor(0,1);
    lcd.print("" + String(timeToRest) + "                 ");
    if (timeToRest <= 0) {
      selectedTeam = 0;
    }
  } else {
    timeToRest = TIME_TO_RESET;
  }

  // если не происходит захват точки, то прибавляем очки команде
  if (greenCount < pointLimits && yellowCount < pointLimits && captureCountDown == captureTime && timeToRest == TIME_TO_RESET) {
    // если выбрана текущая команда 2 (желтые)
    if (selectedTeam == 2) {
      if (currentTimeForPoint <= 0) {
        yellowCount += pointsStep;
        // сохраняем очки каждые 10 секунд
        if (yellowCount % 10 == 0 || timeForPoint > 10) {
          savePoints(greenCount, yellowCount, selectedTeam);
        }
        currentTimeForPoint = timeForPoint;
      } else {
        currentTimeForPoint--;
      }
    }
    // если выбрана текущая команда 1 (зеленые)
    if (selectedTeam == 1) {
      if (currentTimeForPoint <= 0) {
        greenCount += pointsStep;
        // сохраняем очки каждые 10 секунд
        if (greenCount % 10 == 0 || timeForPoint > 10) {
          savePoints(greenCount, yellowCount, selectedTeam);
        }
        currentTimeForPoint = timeForPoint;
      } else {
        currentTimeForPoint--;
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

  // TODO написать нормальный программный таймер
  delay(1000);

  // если нажата сервисная кнопка на плате, то выводим описание настроек в серийный порт (вывод данных на PC или телефон через bluetooth)
  if (serviceButtonState == HIGH) {
    help();
  }

  // чтение данных из серийного порта, можно подавать команды через PC или из телефона при пожключении по bluetooth.
  if (Serial.available()) {
    String command = Serial.readString();
    readConfigurationFromSerialPort(command);
  }
}
