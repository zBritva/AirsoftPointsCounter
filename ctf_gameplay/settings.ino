#include <EEPROM.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// функция сохраняет текучие очки в память
void savePoints(int greenPoints, int yellowPoints, int teamFlag) { 
  int greenPoints1 = (greenPoints - greenPoints % 100) / 100;
  int greenPoints2 = greenPoints % 100;
  EEPROM.write(GREEN_POINTS_ADDR_1, greenPoints1);
  EEPROM.write(GREEN_POINTS_ADDR_2, greenPoints2);
  
  int yellowPoints1 = (yellowPoints - yellowPoints % 100) / 100;
  int yellowPoints2 = yellowPoints % 100;
  EEPROM.write(YELLOW_POINTS_ADDR_1, yellowPoints1);
  EEPROM.write(YELLOW_POINTS_ADDR_2, yellowPoints2);
  
  EEPROM.write(TEAM_FLAG_ADDR_2, teamFlag);
}

// функция загружает сохраненные очки из памяти
void loadPoints() {
  int yellowCount1 = EEPROM.read(YELLOW_POINTS_ADDR_1);
  int yellowCount2 = EEPROM.read(YELLOW_POINTS_ADDR_2);

  if (yellowCount1 > 100) {
    yellowCount1 = 0;
  }
  
  if (yellowCount2 > 100) {
    yellowCount2 = 0;
  }
  
  int greenCount1 = EEPROM.read(GREEN_POINTS_ADDR_1);
  int greenCount2 = EEPROM.read(GREEN_POINTS_ADDR_2);

  if (greenCount1 > 100) {
    greenCount1 = 0;
  }
  
  if (greenCount2 > 100) {
    greenCount2 = 0;
  }
  yellowCount = yellowCount1 * 100 + yellowCount2;
  greenCount = greenCount1 * 100 + greenCount2;
  
  selectedTeam = EEPROM.read(TEAM_FLAG_ADDR_2);
  Serial.write("yellowCount\n");
  Serial.print(String(yellowCount));
  Serial.write("\n");
  Serial.write("greenCount\n");
  Serial.print(String(greenCount));     
  Serial.write("\n");

  captureTime = EEPROM.read(CAPTURE_TIME_ADDR);

  int pointLimits1 = EEPROM.read(POINT_LIMITS_ADDR_1);
  int pointLimits2 = EEPROM.read(POINT_LIMITS_ADDR_2);

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

  debugMode = EEPROM.read(DEBUG_MODE_ADDR);
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
}

// функция считает команды с серийного порта и меняет настройки
void readConfigurationFromSerialPort(String command) {
  if (command.startsWith("-h")) {
       help();
    }
    if (command.startsWith("-c ")) {
      captureTime = 0;
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
      int pointLimits1 = (pointLimits - pointLimits % 100) / 100;
      int pointLimits2 = pointLimits % 100;
      EEPROM.write(POINT_LIMITS_ADDR_1, pointLimits1);
      EEPROM.write(POINT_LIMITS_ADDR_2, pointLimits2);
      Serial.print("Лимит очков: " + String(pointLimits) + "\n");
      lcd.clear();
    }
    if (command.startsWith("-r")) {
      yellowCount = 0;
      greenCount = 0;
      selectedTeam = 0;
      savePoints(greenCount, yellowCount, selectedTeam);
      Serial.print("Сброс счетчика\n");
      lcd.clear();
    }
    if (command.startsWith("-d ")) {
      debugMode = 0;
      command.trim();
      command.replace("-d ", "");
      debugMode = command.toInt();
      EEPROM.write(DEBUG_MODE_ADDR, debugMode);
      if (debugMode) {
        Serial.print("Логирование включено\n");
      } else {
        Serial.print("Логирование выключено\n");
      }
    }
}