#include <GyverTM1637.h>
#include <EEPROM.h>
#include <Wire.h>
/*
Возможно нужно пофиксить либу LiquidCrystal_I2C функция должна возвращать 1 а не 0
inline size_t LiquidCrystal_I2C::write(uint8_t value) {
 send(value, Rs);
 return 0;
}
*/
// #include <LiquidCrystal_I2C.h>

/*
 * PORT configuration
 */
const byte YELLOW_LED_CLK = 9;
const byte YELLOW_LED_DIO = 8;

const byte BLUE_LED_CLK = 7;
const byte BLUE_LED_DIO = 6;

const byte BLUE_BUTTON = 3;
const byte YELLOW_BUTTON = 2;

const byte SERVICE_BUTTON = 5; 

const byte BLUE_TEAM_LED = 12;
const byte YELLOW_TEAM_LED = 10;

const byte TONE_PORT = 11;

/*
 * MEMORY configuration
 */

const byte BLUE_POINTS_ADDR_1 = 0;
const byte BLUE_POINTS_ADDR_2 = 1;

const byte YELLOW_POINTS_ADDR_1 = 2;
const byte YELLOW_POINTS_ADDR_2 = 3;

const byte TEAM_FLAG_ADDR_2 = 4;

const byte CAPTURE_TIME_ADDR = 5;
const byte POINT_LIMITS_ADDR_1 = 6;
const byte POINT_LIMITS_ADDR_2 = 7;
const byte DEBUG_MODE_ADDR = 8;

const byte TIME_FOR_POINT_ADDR_1 = 9;
const byte TIME_FOR_POINT_ADDR_2 = 10;

const byte POINT_PER_TIME_ADDR_1 = 11;
const byte POINT_PER_TIME_ADDR_2 = 12;

const byte DISPLAY_TIME_ADDR_1 = 13;
const byte DISPLAY_TIME_ADDR_2 = 14;

const byte CUR_TIME_FOR_POINT_ADDR_1 = 15;
const byte CUR_TIME_FOR_POINT_ADDR_2 = 16;

const byte TONE_SETTINGS_ADDR = 17;
#define NOTE 4978

const byte DEFAULT_TIME_FOR_POINT = 60;
const byte DEFAULT_POINT_PER_TIME = 1;
const byte DEFAULT_CAPTURE_TIME = 15;

const byte DEFAULT_DISPLAY_TIME = 5;

const short MAX_POINTS = 9999;
const byte TIME_TO_RESET = 60;

const byte TIME_FOR_POINT_MIN_LIMIT = 3;

/*
 * VARIABLES
 */

short yellowCount = 0;
short blueCount = 0;

byte yellowButtonState = LOW;
byte blueButtonState = LOW;
byte serviceButtonState = LOW;
byte selectedTeam = 0; // текущая выбранная команда
//1 -blue
//2 -yellow
byte captureTime = DEFAULT_CAPTURE_TIME;     // время захвата точки
short timeForPoint = DEFAULT_TIME_FOR_POINT; // сколько секунд нужно удерживать точку для получения очков
byte pointsStep = 1;                         // количество очков за еденицу времени (timeToPoint)
byte captureCountDown = captureTime;
short pointLimits = MAX_POINTS;
bool debugMode = false;
short currentTimeForPoint = timeForPoint;
bool serviceMode = false;

// какую информацию нужно показывать
// кривая реализация карусели
const byte infoDisplayCOUNT = 2;
byte infoDisplayTime[] = {5, 15, 15};
byte infoDisplayTimeCounter[] = {5, 15, 15};
byte currentInfo = 0;
// 0 -текущие очки
// 1 -время до получения 1 единцы очков
// 2 -резерв

byte timeToRest = 10;

GyverTM1637 yellow_tm1637(YELLOW_LED_CLK, YELLOW_LED_DIO);
GyverTM1637 blue_tm1637(BLUE_LED_CLK, BLUE_LED_DIO);
// LiquidCrystal_I2C lcd(0x27,16,2);
// yellow_tm1637.brightness(7);
// blue_tm1637.brightness(7);

void setup()
{
  // erase();
  pinMode(YELLOW_BUTTON, INPUT);
  pinMode(BLUE_BUTTON, INPUT);
  // pinMode(SERVICE_BUTTON, INPUT);
  pinMode(BLUE_TEAM_LED, OUTPUT);
  pinMode(YELLOW_TEAM_LED, OUTPUT);
  pinMode(TONE_PORT, OUTPUT);
  // digitalWrite(TONE_PORT, LOW);
  Serial.begin(9600);
  loadPoints();
  Serial.print("Setup done\n");
  Serial.print("yellowCount " + String(yellowCount) + "\n");
  Serial.print("blueCount " + String(yellowCount) + "\n");
  Serial.print("pointLimits " + String(pointLimits) + "\n");
  yellow_tm1637.brightness(7);
  blue_tm1637.brightness(7);
  yellow_tm1637.displayInt(0);
  blue_tm1637.displayInt(0);

  yellowButtonState = LOW;
  blueButtonState = LOW;
  serviceButtonState = LOW;

  yellowButtonState = digitalRead(YELLOW_BUTTON);
  blueButtonState = digitalRead(BLUE_BUTTON);
  // serviceButtonState = digitalRead(SERVICE_BUTTON);

  if (yellowButtonState == HIGH && blueButtonState == HIGH)
  {
    serviceMode = true;
    yellow_tm1637.displayInt(-999);
    blue_tm1637.displayInt(-999);
    delay(2000);
  }
  // settings();
  // tone();

  Serial.print("setup\n");
}

void displayInfo(int yellow, int blue)
{
  yellow_tm1637.displayInt(yellow);
  blue_tm1637.displayInt(blue);
}

bool isTone = false;
void tone_(bool forced = false) {
  if (!isTone && !forced) {
    return;
  }
  tone(TONE_PORT, NOTE, 500);
  // digitalWrite(TONE_PORT, HIGH);
  // delay(100);
  // digitalWrite(TONE_PORT, LOW);
}

int savedValue = 0;
int userValue = 0;
int userValueLimit = 9999;
byte selectedParam = 0;
byte paramToSelect = 1;
byte displayParam = -1;
unsigned long time = 0;
unsigned long holdTime = 0;
unsigned long theLastSavedTime = 0;
int stepDelay = 1000;
bool longStep = false;
bool checkHold = false;
bool isHold = false;
int holdCount = 0;
void serviceLoop()
{
  // Serial.print("Задать время удержания                       : -t N\n"); 1
  // Serial.print("Задать очки за еденицу времени удержания     : -p N\n"); 2
  // Serial.print("Задать время захвата                         : -c N\n"); 3
  // Serial.print("Задать лимит очков                           : -l N\n"); 4
  // Serial.print("Звук                           : -r\n"); 5
  // Serial.print("Перезапустить счет                           : -r\n"); 6
  yellowButtonState = digitalRead(YELLOW_BUTTON);
  blueButtonState = digitalRead(BLUE_BUTTON);
  if (selectedParam > 0)
  {
    if (yellowButtonState == LOW && blueButtonState == HIGH)
    {
      holdCount++;
      if (userValue < userValueLimit)
      {
        userValue++;
      }
      else
      {
        userValue = userValueLimit;
      }
      isHold = true;
    }
    if (yellowButtonState == HIGH && blueButtonState == LOW)
    {
      holdCount++;
      if (userValue > 1)
      {
        userValue--;
      }
      else
      {
        userValue = 1;
      }
      isHold = true;
    }
    if (isHold)
    {
      holdCount++;
    }
    else
    {
      holdCount = 0;
    }

    if (yellowButtonState == LOW && blueButtonState == LOW)
    {
      isHold = false;
    }
    if (isHold && holdCount < 5)
    {
      delay(1000);
    }
    if (holdCount >= 5 && holdCount < 10)
    {
      delay(500);
    }
    if (holdCount >= 10 && holdCount < 15)
    {
      delay(150);
    }
    if (holdCount >= 20)
    {
      delay(25);
      isHold = false;
    }

    if (userValue != savedValue && theLastSavedTime - millis() > 3000)
    {
      theLastSavedTime = millis();
      //TODO save the value
      savedValue = userValue;
      blue_tm1637.displayInt(savedValue);
      switch (selectedParam)
      {
      case 1:
        setTimeForPoint(userValue);
        savedValue = userValue;
        break;

      case 2:
        setPointsStep(userValue);
        savedValue = userValue;
        break;

      case 3:
        setCaptureTime(userValue);
        savedValue = userValue;
        break;

      case 4:
        setPointLimits(userValue);
        savedValue = userValue;
        break;

      case 5:
        setTone(userValue == 2 ? true : false);
        savedValue = userValue;
        break;
      case 6:
        resetGame();
        savedValue = -1;
        break;

      default:
        break;
      }
    }

    return;
  }

  if (displayParam != paramToSelect)
  {
    displayParam = paramToSelect;
    yellow_tm1637.displayInt(paramToSelect);

    switch (displayParam)
    {
    case 1:
      userValue = timeForPoint;
      savedValue = userValue;
      userValueLimit = 9999;
      break;

    case 2:
      userValue = pointsStep;
      savedValue = userValue;
      userValueLimit = 9999;
      break;

    case 3:
      userValue = captureTime;
      savedValue = userValue;
      userValueLimit = 255;
      break;

    case 4:
      userValue = pointLimits;
      savedValue = userValue;
      userValueLimit = 9999;
      break;
    case 5:
      userValue = isTone ? 2 : 1;
      savedValue = userValue;
      userValueLimit = 2;
      break;
    case 6:
      userValue = 0;
      savedValue = 0;
    default:
      break;
    }
    blue_tm1637.displayInt(savedValue);
  }

  if (yellowButtonState == HIGH && blueButtonState == LOW)
  {
    paramToSelect++;
    if (paramToSelect > 6)
    {
      paramToSelect = 1;
    }
    delay(1000);
  }

  if (yellowButtonState == LOW && blueButtonState == HIGH)
  {
    isHold = false;
    selectedParam = paramToSelect;

    delay(1000);
  }
}

void loop()
{
  if (serviceMode)
  {
    serviceLoop();
    return;
  }
  //показываем текущий счетчик, если только не идет захват
  if (captureCountDown == captureTime)
  {
    // какой счет показывать
    if (/*currentInfo == 0 || */timeForPoint <= TIME_FOR_POINT_MIN_LIMIT || currentTimeForPoint > timeForPoint - 3)
    {
      displayInfo(yellowCount, blueCount);
    }
    else
    // вермя до захвата показываем, если только точка захвачена
    if (/*currentInfo == 1 &&*/ selectedTeam != 0 && timeForPoint > TIME_FOR_POINT_MIN_LIMIT && currentTimeForPoint < timeForPoint - 3)
    {
      if (selectedTeam == 1) {
        displayInfo(yellowCount, currentTimeForPoint);
      }
      if (selectedTeam == 2) {
        displayInfo(currentTimeForPoint, blueCount);
      }
    }
    // infoDisplayTimeCounter[currentInfo]--;
    // if (infoDisplayTimeCounter[currentInfo] <= 0)
    // {
    //   infoDisplayTimeCounter[currentInfo] = infoDisplayTime[currentInfo];
    //   currentInfo++;
    //   infoDisplayTimeCounter[currentInfo] = infoDisplayTime[currentInfo];
    // }
    // if (currentInfo == infoDisplayCOUNT)
    // {
    //   currentInfo = 0;
    // }
  }

  yellowButtonState = LOW;
  blueButtonState = LOW;
  serviceButtonState = LOW;
  yellowButtonState = digitalRead(YELLOW_BUTTON);
  blueButtonState = digitalRead(BLUE_BUTTON);
  // Serial.print("BUTTONS Y " +  String(yellowButtonState) + " B " + String(blueButtonState) + " S " + String(serviceButtonState) + "  \n");

  // проверяем состояние кнопок
  // если нажата кнопка желтых, но при этом не нажата кнопка зеленых
  if (yellowButtonState == HIGH && blueButtonState == LOW)
  {
    // если счетчик захвата 0, то точка захвачена
    if (captureCountDown <= 0)
    {
      // сбросит текущий счетчик времени для получения очков, если другая команда захватила точку
      if (selectedTeam == 1)
      {
        currentTimeForPoint = timeForPoint;
      }
      Serial.print("selectedTeam 2\n");
      selectedTeam = 2;
      captureCountDown = captureTime;
      savePoints(blueCount, yellowCount, selectedTeam, currentTimeForPoint);
    }
    else if (selectedTeam != 2)
    { // проверяем что точка не занято желтыми (захват захваченной точки не имеет смысла)
      displayInfo(captureCountDown, captureCountDown);
      captureCountDown--; // уменьшаем счетчик захвата
      tone_();
    }
  }
  else
      // аналогично для желтых
      if (blueButtonState == HIGH && yellowButtonState == LOW)
  {
    if (captureCountDown <= 0)
    {
      // сбросит текущий счетчик времени для получения очков, если другая команда захватила точку
      if (selectedTeam == 2)
      {
        currentTimeForPoint = timeForPoint;
      }
      selectedTeam = 1;
      Serial.print("selectedTeam 1\n");
      captureCountDown = captureTime;
      savePoints(blueCount, yellowCount, selectedTeam, currentTimeForPoint);
    }
    else if (selectedTeam != 1)
    {
      // чистим дисплей перед началом отчета
      displayInfo(captureCountDown, captureCountDown);
      captureCountDown--;
      tone_();
    }
  }
  else
  {
    // если не одна кнопка не нажата, то сбрасываем счетчик захвата
    captureCountDown = captureTime;
  }

  // сброс захвата (нужно удерживать 2 кнопки одновременно более 10 секунд)
  if (false && yellowButtonState == HIGH && blueButtonState == HIGH) // DISABLED
  {
    timeToRest--;
    displayInfo(timeToRest, timeToRest);
    if (timeToRest <= 0)
    {
      resetGame();
    }
  }
  else
  {
    timeToRest = TIME_TO_RESET;
  }

  // // если не происходит захват точки, то прибавляем очки команде
  // Serial.print("blueCount < pointLimits " + String(blueCount < pointLimits) + "\n");
  // Serial.print("yellowCount < pointLimits " + String(yellowCount < pointLimits) + "\n");
  // Serial.print("captureCountDown == captureTime " + String(yellowCount < pointLimits) + "\n");
  // Serial.print("timeToRest == TIME_TO_RESET " + String(timeToRest == TIME_TO_RESET) + "\n");
  if (blueCount < pointLimits && yellowCount < pointLimits && captureCountDown == captureTime && timeToRest == TIME_TO_RESET)
  {
    // если выбрана текущая команда 2 (желтые)
    if (selectedTeam == 2)
    {
      if (currentTimeForPoint <= 0)
      {
        yellowCount += pointsStep;
        displayInfo(yellowCount, blueCount);
        if (timeForPoint > TIME_FOR_POINT_MIN_LIMIT) {
          tone_();
          delay(100);
          tone_();
        }
        // сохраняем очки каждые 10 секунд
        if (yellowCount % 10 == 0 || timeForPoint > 10)
        {
          savePoints(blueCount, yellowCount, selectedTeam, currentTimeForPoint);
        }
        currentTimeForPoint = timeForPoint;
      }
      else
      {
        currentTimeForPoint--;
        setCurrentTimeForPoint(currentTimeForPoint);
      }
    }
    // если выбрана текущая команда 1 (синие)
    if (selectedTeam == 1)
    {
      if (currentTimeForPoint <= 0)
      {
        blueCount += pointsStep;
        displayInfo(yellowCount, blueCount);
        if (timeForPoint > TIME_FOR_POINT_MIN_LIMIT) {
          tone_();
          delay(150);
          tone_();
        }
        // сохраняем очки каждые 10 секунд
        if (blueCount % 10 == 0 || timeForPoint > 10)
        {
          savePoints(blueCount, yellowCount, selectedTeam, currentTimeForPoint);
        }
        currentTimeForPoint = timeForPoint;
      }
      else
      {
        currentTimeForPoint--;
        setCurrentTimeForPoint(currentTimeForPoint);
      }
    }
  }

  // если команда набрала необходимое количество очков, то останавливаем счетчик
  if (blueCount >= pointLimits || yellowCount >= pointLimits)
  {
    // смотрим у кого больше очков и выводим на экран победителя
    if (blueCount > yellowCount)
    {
      displayInfo(-1, blueCount);
    }
    else
    {
      displayInfo(yellowCount, -1);
    }
    // Сохраняем очки в постоянной памяти (после сохранения сбой в питании не влияет на результат)
    savePoints(blueCount, yellowCount, selectedTeam, currentTimeForPoint);
  }

  // TODO написать нормальный программный таймер
  delay(1000);
  // Serial.print("delay\n");

  // если нажата сервисная кнопка на плате, то выводим описание настроек в серийный порт (вывод данных на PC или телефон через bluetooth)
  if (serviceButtonState == HIGH)
  {
    // help();
  }

  // чтение данных из серийного порта, можно подавать команды через PC или из телефона при пожключении по bluetooth.
  if (Serial.available())
  {
    String command = Serial.readString();
    readConfigurationFromSerialPort(command);
  }

  if (selectedTeam == 0) {
    digitalWrite(BLUE_TEAM_LED, 0);
    digitalWrite(YELLOW_TEAM_LED, 0);
  }
  if (selectedTeam == 1) {
    digitalWrite(BLUE_TEAM_LED, 1);
    digitalWrite(YELLOW_TEAM_LED, 0);
  }
  if (selectedTeam == 2) {
    digitalWrite(BLUE_TEAM_LED, 0);
    digitalWrite(YELLOW_TEAM_LED, 1);
  }
}

// функция сохраняет текучие очки в память
void savePoints(short bluePoints, short yellowPoints, byte teamFlag, int currentTimeForPoint)
{
  byte bluePoints1 = (bluePoints - bluePoints % 100) / 100;
  byte bluePoints2 = bluePoints % 100;
  EEPROM.write(BLUE_POINTS_ADDR_1, bluePoints1);
  EEPROM.write(BLUE_POINTS_ADDR_2, bluePoints2);

  byte yellowPoints1 = (yellowPoints - yellowPoints % 100) / 100;
  byte yellowPoints2 = yellowPoints % 100;
  EEPROM.write(YELLOW_POINTS_ADDR_1, yellowPoints1);
  EEPROM.write(YELLOW_POINTS_ADDR_2, yellowPoints2);

  EEPROM.write(TEAM_FLAG_ADDR_2, teamFlag);

  if (timeForPoint > 1) {
    setCurrentTimeForPoint(currentTimeForPoint);
  }
}

// функция загружает сохраненные очки из памяти а так же настройки точки
void loadPoints()
{
  // ====================================================
  // yellowCount
  // ====================================================
  byte yellowCount1 = EEPROM.read(YELLOW_POINTS_ADDR_1);
  byte yellowCount2 = EEPROM.read(YELLOW_POINTS_ADDR_2);

  if (yellowCount1 > 100)
  {
    yellowCount1 = 0;
  }

  if (yellowCount2 > 100)
  {
    yellowCount2 = 0;
  }
  yellowCount = short(yellowCount1) * 100 + short(yellowCount2);

  // ====================================================
  // blueCount
  // ====================================================
  byte blueCount1 = EEPROM.read(BLUE_POINTS_ADDR_1);
  byte blueCount2 = EEPROM.read(BLUE_POINTS_ADDR_2);

  if (blueCount1 > 100)
  {
    blueCount1 = 0;
  }

  if (blueCount2 > 100)
  {
    blueCount2 = 0;
  }
  blueCount = short(blueCount1) * 100 + short(blueCount2);

  // ====================================================
  // selectedTeam
  // ====================================================
  selectedTeam = EEPROM.read(TEAM_FLAG_ADDR_2);
  // ====================================================
  // captureTime
  // ====================================================
  captureTime = EEPROM.read(CAPTURE_TIME_ADDR);
  // ====================================================
  // Sound settings
  // ====================================================
  
  isTone = (bool) EEPROM.read(TONE_SETTINGS_ADDR);
  // ====================================================
  // pointLimits
  // ====================================================
  byte pointLimits1 = EEPROM.read(POINT_LIMITS_ADDR_1);
  byte pointLimits2 = EEPROM.read(POINT_LIMITS_ADDR_2);

  if (pointLimits1 > 100)
  {
    pointLimits1 = 99;
  }

  if (pointLimits2 > 100)
  {
    pointLimits2 = 99;
  }
  pointLimits = pointLimits1 * 100 + pointLimits2;
  if (pointLimits == 0)
  {
    pointLimits = MAX_POINTS;
  }

  // ====================================================
  // debugMode
  // ====================================================
  // debugMode = EEPROM.read(DEBUG_MODE_ADDR);

  // ====================================================
  // timeForPoint
  // ====================================================
  
  byte curTimeForPoint1 = EEPROM.read(CUR_TIME_FOR_POINT_ADDR_1);
  byte curTimeForPoint2 = EEPROM.read(CUR_TIME_FOR_POINT_ADDR_2);

  if (curTimeForPoint1 > 100)
  {
    curTimeForPoint1 = 99;
  }

  if (curTimeForPoint2 > 100)
  {
    curTimeForPoint2 = 99;
  }
  currentTimeForPoint = curTimeForPoint1 * 100 + curTimeForPoint2;
  if (currentTimeForPoint <= 0)
  {
    currentTimeForPoint = 0;
  }

  // ====================================================
  // timeForPoint
  // ====================================================
  byte timeForPoint1 = EEPROM.read(TIME_FOR_POINT_ADDR_1);
  byte timeForPoint2 = EEPROM.read(TIME_FOR_POINT_ADDR_2);

  if (timeForPoint1 > 100)
  {
    timeForPoint1 = 99;
  }

  if (timeForPoint2 > 100)
  {
    timeForPoint2 = 99;
  }
  timeForPoint = timeForPoint1 * 100 + timeForPoint2;
  if (timeForPoint <= 0)
  {
    timeForPoint = 0;
  }

  // ====================================================
  // pointsStep
  // ====================================================
  byte pointsStep1 = EEPROM.read(POINT_PER_TIME_ADDR_1);
  byte pointsStep2 = EEPROM.read(POINT_PER_TIME_ADDR_2);

  if (pointsStep1 > 100)
  {
    pointsStep1 = 99;
  }

  if (pointsStep2 > 100)
  {
    pointsStep2 = 99;
  }
  pointsStep = pointsStep1 * 100 + pointsStep2;
  if (pointsStep <= 0)
  {
    pointsStep = 1;
  }

  // ====================================================
  // infoDisplayTime
  // ====================================================

  infoDisplayTime[0] = EEPROM.read(DISPLAY_TIME_ADDR_1);
  infoDisplayTime[1] = EEPROM.read(DISPLAY_TIME_ADDR_2);
  if (infoDisplayTime[0] <= 0)
  {
    infoDisplayTime[0] = 5;
  }
  if (infoDisplayTime[1] <= 0)
  {
    infoDisplayTime[1] = 15;
  }
  Serial.print("Load settings done\n");
}

// функция для стирания памяти в используемых ардесах, вызывать для новый плат, перед заливанием окончательной прошики
void erase()
{
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

void resetGame()
{
  yellowCount = 0;
  blueCount = 0;
  selectedTeam = 0;
  currentTimeForPoint = timeForPoint;
  currentInfo = 0;
  infoDisplayTimeCounter[0] = infoDisplayTime[0];
  infoDisplayTimeCounter[1] = infoDisplayTime[1];
  savePoints(blueCount, yellowCount, selectedTeam, currentTimeForPoint);
  Serial.print("Сброс счетчика\n");
  serviceMode = false;
  displayInfo(-1,-1);
  delay(1000);
  // lcd.clear();
}

void setCaptureTime(byte captureTime)
{
  EEPROM.write(CAPTURE_TIME_ADDR, captureTime);
}

void setTone(bool isOn)
{
  if (isOn && isTone != isOn) {
    tone_();
  }
  isTone = isOn;
  EEPROM.write(TONE_SETTINGS_ADDR, isOn);
}

void setPointLimits(int pointLimits)
{
  byte pointLimits1 = (pointLimits - pointLimits % 100) / 100;
  byte pointLimits2 = pointLimits % 100;
  EEPROM.write(POINT_LIMITS_ADDR_1, pointLimits1);
  EEPROM.write(POINT_LIMITS_ADDR_2, pointLimits2);
}

void setTimeForPoint(int timeForPoint)
{
  byte timeForPoint1 = (timeForPoint - timeForPoint % 100) / 100;
  byte timeForPoint2 = timeForPoint % 100;
  EEPROM.write(TIME_FOR_POINT_ADDR_1, timeForPoint1);
  EEPROM.write(TIME_FOR_POINT_ADDR_2, timeForPoint2);
}

void setCurrentTimeForPoint(int timeForPoint)
{
  byte timeForPoint1 = (timeForPoint - timeForPoint % 100) / 100;
  byte timeForPoint2 = timeForPoint % 100;
  EEPROM.write(CUR_TIME_FOR_POINT_ADDR_1, timeForPoint1);
  EEPROM.write(CUR_TIME_FOR_POINT_ADDR_2, timeForPoint2);
}

void setPointsStep(int pointsStep)
{
  byte pointsStep1 = (pointsStep - pointsStep % 100) / 100;
  byte pointsStep2 = pointsStep % 100;
  EEPROM.write(POINT_PER_TIME_ADDR_1, pointsStep1);
  EEPROM.write(POINT_PER_TIME_ADDR_2, pointsStep2);
}

// функция считает команды с серийного порта и меняет настройки
void readConfigurationFromSerialPort(String command)
{
  if (command.startsWith("-h"))
  {
    help();
  }
  if (command.startsWith("-s"))
  {
    settings();
  }
  if (command.startsWith("-c "))
  {
    captureTime = DEFAULT_CAPTURE_TIME;
    command.trim();
    command.replace("-c ", "");
    captureTime = command.toInt();
    //TODO split to function
    setCaptureTime(captureTime);
    Serial.print("Время захвата точки (сек.): " + String(captureTime) + "\n");
  }
  if (command.startsWith("-l "))
  {
    pointLimits = MAX_POINTS;
    command.trim();
    command.replace("-l ", "");
    pointLimits = command.toInt();
    //TODO split to function
    setPointLimits(pointLimits);
    Serial.print("Лимит очков: " + String(pointLimits) + "\n");
    // lcd.clear();
  }
  if (command.startsWith("-r"))
  {
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
  if (command.startsWith("-t "))
  {
    timeForPoint = DEFAULT_TIME_FOR_POINT;
    command.trim();
    command.replace("-t ", "");
    timeForPoint = command.toInt();
    //TODO split to function
    setTimeForPoint(timeForPoint);
    Serial.print("Время удержания: " + String(timeForPoint) + "\n");
  }
  if (command.startsWith("-p "))
  {
    pointsStep = DEFAULT_POINT_PER_TIME;
    command.trim();
    command.replace("-p ", "");
    pointsStep = command.toInt();
    //TODO split to function
    setPointsStep(pointsStep);
    Serial.print("Очки за еденицу времени удержания: " + String(pointsStep) + "\n");
  }
  if (command.startsWith("-it0"))
  {
    infoDisplayTime[0] = 5;
    command.trim();
    command.replace("-it0", "");
    infoDisplayTime[0] = command.toInt();
    EEPROM.write(DISPLAY_TIME_ADDR_1, infoDisplayTime[0]);
    Serial.print("Время отображения очков: " + String(infoDisplayTime[0]) + "\n");
  }
  if (command.startsWith("-it1"))
  {
    infoDisplayTime[1] = 15;
    command.trim();
    command.replace("-it1", "");
    infoDisplayTime[1] = command.toInt();
    EEPROM.write(DISPLAY_TIME_ADDR_2, infoDisplayTime[1]);
    Serial.print("Время отображения таймера до получения очков: " + String(infoDisplayTime[1]) + "\n");
  }
  if (command.startsWith("-son"))
  {
    EEPROM.write(TONE_SETTINGS_ADDR, 1);
    Serial.print("Звук вкл.\n");
  }
  if (command.startsWith("-soff"))
  {
    EEPROM.write(TONE_SETTINGS_ADDR, 0);
    Serial.print("Звук выкл.\n");
  }
}

void help()
{
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

void settings()
{
  Serial.print("TEAM " + String(selectedTeam) + "\n");
  Serial.print("BLUE " + String(blueCount) + "\n");
  Serial.print("YELLOW " + String(yellowCount) + "\n");
  Serial.print("CURRENT TIME" + String(currentTimeForPoint) + "\n");
  Serial.print("-t " + String(timeForPoint) + "\n");
  Serial.print("-p " + String(pointsStep) + "\n");
  Serial.print("-c " + String(captureTime) + "\n");
  Serial.print("-l " + String(pointLimits) + "\n");
  // Serial.print("-d "+ String(debugMode) + "\n");
  Serial.print("-it0 " + String(infoDisplayTime[0]) + "\n");
  Serial.print("-it1 " + String(infoDisplayTime[1]) + "\n");
}