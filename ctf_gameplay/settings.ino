#include <EEPROM.h>
#include <Wire.h> 
#include <Arduino.h>

// функция сохраняет текучие очки в память
void savePoints(short greenPoints, short yellowPoints, byte teamFlag) { 
  byte greenPoints1 = (greenPoints - greenPoints % 100) / 100;
  byte greenPoints2 = greenPoints % 100;
  EEPROM.write(GREEN_POINTS_ADDR_1, greenPoints1);
  EEPROM.write(GREEN_POINTS_ADDR_2, greenPoints2);
  
  byte yellowPoints1 = (yellowPoints - yellowPoints % 100) / 100;
  byte yellowPoints2 = yellowPoints % 100;
  EEPROM.write(YELLOW_POINTS_ADDR_1, yellowPoints1);
  EEPROM.write(YELLOW_POINTS_ADDR_2, yellowPoints2);
  
  EEPROM.write(TEAM_FLAG_ADDR_2, teamFlag);
}

// функция загружает сохраненные очки из памяти а так же настройки точки
void loadPoints() {
// ====================================================
// yellowCount
// ====================================================
  byte yellowCount1 = EEPROM.read(YELLOW_POINTS_ADDR_1);
  byte yellowCount2 = EEPROM.read(YELLOW_POINTS_ADDR_2);

  if (yellowCount1 > 100) {
    yellowCount1 = 0;
  }
  
  if (yellowCount2 > 100) {
    yellowCount2 = 0;
  }
  yellowCount = short(yellowCount1) * 100 + short(yellowCount2);
  Serial.print("yellowCount\n");
  Serial.print(String(yellowCount));
  Serial.print("\n");
  
// ====================================================
// greenCount
// ====================================================
  byte greenCount1 = EEPROM.read(GREEN_POINTS_ADDR_1);
  byte greenCount2 = EEPROM.read(GREEN_POINTS_ADDR_2);

  if (greenCount1 > 100) {
    greenCount1 = 0;
  }
  
  if (greenCount2 > 100) {
    greenCount2 = 0;
  }
  greenCount = short(greenCount1) * 100 + short(greenCount2);
  Serial.print("greenCount\n");
  Serial.print(String(greenCount));     
  Serial.print("\n");
  
// ====================================================
// selectedTeam
// ====================================================
  selectedTeam = EEPROM.read(TEAM_FLAG_ADDR_2);
// ====================================================
// captureTime
// ====================================================
  captureTime = EEPROM.read(CAPTURE_TIME_ADDR);

// ====================================================
// pointLimits
// ====================================================
  byte pointLimits1 = EEPROM.read(POINT_LIMITS_ADDR_1);
  byte pointLimits2 = EEPROM.read(POINT_LIMITS_ADDR_2);

  if (pointLimits1 > 100) {
    pointLimits1 = 99;
  }
  
  if (pointLimits2 > 100) {
    pointLimits2 = 99;
  }
  pointLimits = pointLimits1 * 100 + pointLimits2;
  if (pointLimits == 0) {
    pointLimits = MAX_POINTS;
  }

// ====================================================
// debugMode
// ====================================================
  // debugMode = EEPROM.read(DEBUG_MODE_ADDR);

// ====================================================
// timeForPoint
// ====================================================
  byte timeForPoint1 = EEPROM.read(TIME_FOR_POINT_ADDR_1);
  byte timeForPoint2 = EEPROM.read(TIME_FOR_POINT_ADDR_2);

  if (timeForPoint1 > 100) {
    timeForPoint1 = 99;
  }
  
  if (timeForPoint2 > 100) {
    timeForPoint2 = 99;
  }
  timeForPoint = timeForPoint1 * 100 + timeForPoint2;
  currentTimeForPoint = timeForPoint;
  if (timeForPoint <= 0) {
    timeForPoint = 0;
  }
  if (currentTimeForPoint <= 0) {
    currentTimeForPoint = 0;
  }

// ====================================================
// pointsStep
// ====================================================
  byte pointsStep1 = EEPROM.read(POINT_PER_TIME_ADDR_1);
  byte pointsStep2 = EEPROM.read(POINT_PER_TIME_ADDR_2);

  if (pointsStep1 > 100) {
    pointsStep1 = 99;
  }
  
  if (pointsStep2 > 100) {
    pointsStep2 = 99;
  }
  pointsStep = pointsStep1 * 100 + pointsStep2;
  if (pointsStep <= 0) {
    pointsStep = 1;
  }

// ====================================================
// infoDisplayTime
// ====================================================

  infoDisplayTime[0] = EEPROM.read(DISPLAY_TIME_ADDR_1);
  infoDisplayTime[1] = EEPROM.read(DISPLAY_TIME_ADDR_2);
  if (infoDisplayTime[0] <= 0) {
    infoDisplayTime[0] = 5;
  }
  if (infoDisplayTime[1] <= 0) {
    infoDisplayTime[1] = 15;
  }
  Serial.print("Load settings done\n");
}

// функция для стирания памяти в используемых ардесах, вызывать для новый плат, перед заливанием окончательной прошики 
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

void resetGame() {
  yellowCount = 0;
  greenCount = 0;
  selectedTeam = 0;
  currentTimeForPoint = timeForPoint;
  currentInfo = 0;
  infoDisplayTimeCounter[0] = infoDisplayTime[0];
  infoDisplayTimeCounter[1] = infoDisplayTime[1];
  savePoints(greenCount, yellowCount, selectedTeam);
  Serial.print("Сброс счетчика\n");
  lcd.clear();
}

// функция считает команды с серийного порта и меняет настройки
void readConfigurationFromSerialPort(String command) {
  if (command.startsWith("-h")) {
      help();
  }
  if (command.startsWith("-s")) {
      settings();
  }
  if (command.startsWith("-c ")) {
    captureTime = DEFAULT_CAPTURE_TIME;
    command.trim();
    command.replace("-c ", "");
    captureTime = command.toInt();
    EEPROM.write(CAPTURE_TIME_ADDR, captureTime);
    Serial.print("Время захвата точки (сек.): " + String(captureTime) + "\n");
  }
  if (command.startsWith("-l ")) {
    pointLimits = MAX_POINTS;
    command.trim();
    command.replace("-l ", "");
    pointLimits = command.toInt();
    byte pointLimits1 = (pointLimits - pointLimits % 100) / 100;
    byte pointLimits2 = pointLimits % 100;
    EEPROM.write(POINT_LIMITS_ADDR_1, pointLimits1);
    EEPROM.write(POINT_LIMITS_ADDR_2, pointLimits2);
    Serial.print("Лимит очков: " + String(pointLimits) + "\n");
    lcd.clear();
  }
  if (command.startsWith("-r")) {
    resetGame();
  }
  // if (command.startsWith("-d ")) {
  //   debugMode = 0;
  //   command.trim();
  //   command.replace("-d ", "");
  //   debugMode = command.toInt();
  //   EEPROM.write(DEBUG_MODE_ADDR, debugMode);
  //   if (debugMode) {
  //     Serial.print("Логирование включено\n");
  //   } else {
  //     Serial.print("Логирование выключено\n");
  //   }
  // }
  if (command.startsWith("-t ")) {
    timeForPoint = DEFAULT_TIME_FOR_POINT;
    command.trim();
    command.replace("-t ", "");
    timeForPoint = command.toInt();
    byte timeForPoint1 = (timeForPoint - timeForPoint % 100) / 100;
    byte timeForPoint2 = timeForPoint % 100;
    EEPROM.write(TIME_FOR_POINT_ADDR_1, timeForPoint1);
    EEPROM.write(TIME_FOR_POINT_ADDR_2, timeForPoint2);
    Serial.print("Время удержания: " + String(timeForPoint) + "\n");
  }
  if (command.startsWith("-p ")) {
    pointsStep = DEFAULT_POINT_PER_TIME;
    command.trim();
    command.replace("-p ", "");
    pointsStep = command.toInt();
    byte pointsStep1 = (pointsStep - pointsStep % 100) / 100;
    byte pointsStep2 = pointsStep % 100;
    EEPROM.write(POINT_PER_TIME_ADDR_1, pointsStep1);
    EEPROM.write(POINT_PER_TIME_ADDR_2, pointsStep2);
    Serial.print("Очки за еденицу времени удержания: " + String(pointsStep) + "\n");
  }
  if (command.startsWith("-it0")) {
    infoDisplayTime[0] = 5;
    command.trim();
    command.replace("-it0", "");
    infoDisplayTime[0] = command.toInt();
    EEPROM.write(DISPLAY_TIME_ADDR_1, infoDisplayTime[0] );
    Serial.print("Время отображения очков: " + String(infoDisplayTime[0]) + "\n");
  }
  if (command.startsWith("-it1")) {
    infoDisplayTime[1] = 15;
    command.trim();
    command.replace("-it1", "");
    infoDisplayTime[1] = command.toInt();
    EEPROM.write(DISPLAY_TIME_ADDR_2, infoDisplayTime[1]);
    Serial.print("Время отображения таймера до получения очков: " + String(infoDisplayTime[1]) + "\n");
  }
}


void help() {
  Serial.print("Задать время удержания                       : -t N\n");
  Serial.print("Задать очки за еденицу времени удержания     : -p N\n");
  Serial.print("Задать время захвата                         : -c N\n");
  Serial.print("Задать лимит очков                           : -l N\n");
  Serial.print("Перезапустить счет                           : -r\n");
  // Serial.print("Отладка                                      : -d 0/1\n");
  Serial.print("Настройки                                    : -s\n");
  Serial.print("Время отображения очков                      : -it0 N\n");
  Serial.print("Время отображения таймера до получения очков : -it1 N\n");
}

void settings() {
  Serial.print("-t "+ String(timeForPoint) + "\n");
  Serial.print("-p "+ String(pointsStep) + "\n");
  Serial.print("-c "+ String(captureTime) + "\n");
  Serial.print("-l "+ String(pointLimits) + "\n");
  // Serial.print("-d "+ String(debugMode) + "\n");
  Serial.print("-it0 "+ String(infoDisplayTime[0]) + "\n");
  Serial.print("-it1 "+ String(infoDisplayTime[1]) + "\n");
}