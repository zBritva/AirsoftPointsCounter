#include <Time.h>
#include <TM1637.h>
#include <DS1307RTC.h>

#include <EEPROM.h>
#include <Wire.h>

//CLOCK
#define DS1307_I2C_ADDRESS 0x68  // This is the I2C address
byte _second, _minute, _hour, _dayOfWeek, _dayOfMonth, _month, _year;

int ledPin = 10;

int currentGameMode = 0;

//const int greenPin = 2;
//const int redPin = 3;

const int adminButton = 9;
const int incButton = 11;
const int decButton = 10;
const int confButton = 9;

//common variables
int redPoints = 0;
int greenPoints = 0;

int redPointsLoop = 0;
int greenPointsLoop = 0;

const int redButton = 7;
const int greenButton = 8;

const int redPointsAddr = 0; //need 2 bytes
const int greenPointsAddr = 2; //need 2 bytes
const int gameModeAddr = 4; //need 1 byte
const int captureTimeAddr = 5; //need 1 byte
const int teamFlagAddr = 6; //need 1 byte

int redButtonState = 0;
int greenButtonState = 0;
int adminButtonState = 0;

bool adminMode = false;
String serialBuffer;

//game mode variables
//capture the point
int captureTime = 10;
int capturing = 0;
//who is captures point
//0 - neutral
//1 - green
//2 - red
int teamFlag = 0; 

const int RED_LED_CLK = 3; 
const int RED_LED_DIO = 2; 

const int GREEN_LED_DIO = 6;
const int GREEN_LED_CLK = 5;

TM1637 red_tm1637(RED_LED_CLK,RED_LED_DIO);
TM1637 green_tm1637(GREEN_LED_CLK,GREEN_LED_DIO);

//DEFINE FUNCTIONS
void checkAdminMode();
void buttonState();
//byte bcdToDec(byte val);
//byte decToBcd(byte val);

void loadPoints(){
  redPoints = EEPROM.read(redPointsAddr);
  redPointsLoop = EEPROM.read(redPointsAddr + 1);
  
  greenPoints = EEPROM.read(greenPointsAddr);
  greenPointsLoop = EEPROM.read(greenPointsAddr + 1);  
  
  teamFlag = EEPROM.read(teamFlagAddr);  
}

void savePoints(){
  EEPROM.write(redPointsAddr, redPoints);
  EEPROM.write(redPointsAddr + 1, redPointsLoop);
  
  EEPROM.write(greenPointsAddr, greenPoints);
  EEPROM.write(greenPointsAddr + 1, greenPointsLoop);
  
  EEPROM.write(teamFlagAddr, teamFlag);
}

void loadSettings()
{
  currentGameMode = EEPROM.read(gameModeAddr); 
  captureTime = EEPROM.read(captureTimeAddr);  
}

void saveSettings()
{
  EEPROM.write(gameModeAddr, currentGameMode);
  EEPROM.write(captureTimeAddr, captureTime);
}

void set_default(){
  EEPROM.write(gameModeAddr, 0);
  EEPROM.write(redPointsAddr, 0);
  EEPROM.write(redPointsAddr + 1, 0);
  EEPROM.write(greenPointsAddr, 0);
  EEPROM.write(greenPointsAddr + 1, 0);
  EEPROM.write(captureTimeAddr, 10);
  EEPROM.write(teamFlagAddr, 0);
  Serial.println("Default settings was set");  
  Serial.println("Please restart device");  
}

void init_game_settings(){
  //Ordering is mandatory
  greenPointsLoop = 0;
  redPointsLoop = 0;
  greenPoints = 0;
  redPoints = 0;
  teamFlag = 0;
  loadPoints();
  capturing = 0;
}

// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);
  delay(1000);
  //loadSettings();
  //init_game_settings();
/*  lcd.init();
  lcd.backlight(); 
  lcd.setCursor(0,0);
  lcd.print("Hello, players!");
  lcd.setCursor(0,1);
  lcd.print("GAME ");
  lcd.print(currentGameMode); */
  
  red_tm1637.display(0,10);
  red_tm1637.display(1,11);
  red_tm1637.display(2,12);
  red_tm1637.display(3,13);
  green_tm1637.display(0,currentGameMode);
  delay(1000);
  // initialize digital pin 13 as an output.
  //pinMode(ledPin, OUTPUT);
  pinMode(redButton, INPUT);
  pinMode(greenButton, INPUT);
  //lcd.clear();
  greenButtonState = 0;
  redButtonState = 0;
  adminButtonState = digitalRead(adminButton);
  
  test_clock();
  Serial.begin(9600);
}

void blink(){
  digitalWrite(ledPin, HIGH);
  delay(750);              
  digitalWrite(ledPin, LOW);
  delay(750);
}

void soft_reset()
{
  redPoints = 0;
  greenPoints = 0;
  redPointsLoop = 0;
  greenPointsLoop = 0;
  teamFlag = 0;
  savePoints();
}

void printClock(){
  //lcd.setCursor(0,0);    
  //lcd.print("TIME:");  
}

void printRedPoints(){  
  red_tm1637.display(redPoints + (255 * redPointsLoop));
  /*lcd.setCursor(9,1);   
  lcd.print("RED:");
  lcd.setCursor(13,1);    
  lcd.print("   ");
  lcd.setCursor(13,1);    
  lcd.print(redPoints + (255 * redPointsLoop));*/
}

void printGreenPoints(){
  green_tm1637.display(redPoints + (255 * redPointsLoop));
  /*lcd.setCursor(0,1);   
  lcd.print("GREEN:");
  lcd.setCursor(6,1);    
  lcd.print("   ");
  lcd.setCursor(6,1);    
  lcd.print(greenPoints + (255 * greenPointsLoop));*/
}

void updatePointLoops(){
 if(greenPoints > 255){
   greenPointsLoop++;
   greenPoints = 0;
 } 
 if(redPoints > 255){
   redPointsLoop++;
   redPoints = 0;
 } 
}

void print2digits(int number) 
{
 if (number >= 0 && number < 10) 
 {
  Serial.write('0');
 }
 Serial.print(number);
}

// the loop function runs over and over again forever
void loop() {
  
  tmElements_t tm;
  if (RTC.read(tm)) 
  {
    Serial.print("Ok, Time = ");
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);
    Serial.write('/');
    Serial.print(tm.Month);
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.println();
  }
  else 
  {
    if (RTC.chipPresent()) 
    {
      Serial.println("The DS1307 is stopped. Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } 
    else 
    {
      Serial.println("DS1307 read error! Please check the circuitry.");
      Serial.println();
    }
  }
  
  delay(1000);
  redButtonState = digitalRead(redButton);
  greenButtonState = digitalRead(greenButton);
  adminButtonState = digitalRead(adminButton);
  if(adminButtonState == 0){    
    if(currentGameMode == 0)
      holdThePoint();
    else
    if(currentGameMode == 1)
      captureThePoint();
    else{
      //lcd.setCursor(0,1);    
      //lcd.print("G.MODE ISN'T SET");
    }
  }
  else{
    admin();
  }  
  checkAdminMode();
  greenButtonState = 0;
  redButtonState = 0;
  adminButtonState = 0;
}

void captureThePoint()
{
  buttonState();    
  //lcd.clear();
  if (greenButtonState == 1 && redButtonState == 1){
    //lcd.setCursor(0,1);    
    //lcd.print(" CONFLICT STATE");
    return;
  }
  
  redButtonState = digitalRead(redButton);
  greenButtonState = digitalRead(greenButton);
    
  //Capturing enabled only if another team captured this point
  if (redButtonState == 1 && teamFlag != 2){
    capturing++;
    
    if(capturing >= captureTime){
      teamFlag = 2;
      capturing = 0;
    }
  }
  else
  //Capturing enabled only if another team captured this point
  if (greenButtonState == 1  && teamFlag != 1){
    capturing++;
        
    if(capturing >= captureTime){
      teamFlag = 1;
      capturing = 0;
    }
  }
  else{
    capturing = 0;
  }
      
  if(capturing > 0){
    //lcd.setCursor(0,0);    
    //lcd.print("CAPTURING: ");
    //lcd.print(capturing);
    red_tm1637.display(0,12);
    green_tm1637.display(0,capturing);
  }
  else{
    //lcd.setCursor(0,0);   
    //if(teamFlag == 2)
    //lcd.print("RED");    
    //if(teamFlag == 1)
    //lcd.print("GREEN");   
    //if(teamFlag == 0)
    //lcd.print("NEUTRAL"); 
  }
  
  if(teamFlag == 2)
    redPoints++;
    
  if(teamFlag == 1)
    greenPoints++;
    
  printGreenPoints();
  printRedPoints();
  
  updatePointLoops();
  
  //save points if point is captured by team
  if(teamFlag != 0)
    savePoints();
}

void holdThePoint()
{
  buttonState();    
  //lcd.clear();
  if (greenButtonState == 1 && redButtonState == 1){
    //lcd.setCursor(0,1);    
    //lcd.print(" CONFLICT STATE");
    red_tm1637.display(0,12);
    green_tm1637.display(0,12);
    return;
  }
  if (redButtonState == 1){
    redPoints++;
    savePoints();
  }  
  printRedPoints();
  if (greenButtonState == 1){
    greenPoints++;
    savePoints();
  }
  printGreenPoints();
  updatePointLoops();
}

void admin()
{
  //lcd.setCursor(0,1);    
  //lcd.print("PRES.BTNs TO RST"); 
  buttonState();
  redButtonState = digitalRead(redButton);
  greenButtonState = digitalRead(greenButton);  
  if (greenButtonState == 1 && redButtonState == 1){
    soft_reset();
    /*lcd.setCursor(0,1);
    lcd.print("                "); 
    lcd.setCursor(0,1);
    lcd.print("RESET DONE"); */
    delay(1000);
  }
}


void checkAdminMode(){
  adminButtonState = digitalRead(adminButton);
  if (adminButtonState == 1){
    /*lcd.setCursor(0,0);    
    lcd.print("                "); 
    lcd.setCursor(0,0);    
    lcd.print("ADMIN MODE");*/
    red_tm1637.display(0,10);
    green_tm1637.display(0,10);

    //first loop after press admin button
    if(!adminMode){
      Serial.begin(9600);  
      Serial.println("Welcome to Airsoft Points Counter Admin console");
      Serial.println("Source code is avialable there: ");
      Serial.println("https://github.com/zBritva/AirsoftPointsCounter");
      Serial.println("License GPLv3");  
      Serial.println("");  
      Serial.println("print 'help' to configure device");  
      adminMode = true;    
    }
    else{
      if (Serial.available()) {  
        serialBuffer = Serial.readString();

        if(serialBuffer == "help"){
          printHelp();
        }
        if(serialBuffer == "list games"){
          list_games();
        }
        if(serialBuffer.substring(0,8) == "set game"){
          int game = 0;
          game = serialBuffer.substring(9).toInt();
          set_game(game);  
        }
        if(serialBuffer == "game"){
          Serial.print("GAME: ");
          Serial.println(currentGameMode);
        }
        if(serialBuffer == "reset"){
          set_default();  
        }
      }
    }
  }
  else{
    if(adminMode == true){
      adminMode = false;
      Serial.println("Admin mode: OFF");  
      Serial.end();
    }
  }
}

void printHelp(){
  Serial.println("help                - print this");
  Serial.println("set game <number>   - set game mode");
  Serial.println("list games          - print avialable game modes");
  Serial.println("game                - print current game mode");
  //Serial.println("settings            - print current settings of game");
  //Serial.println("set <param> <value> - set settings value");
  Serial.println("reset               - erase memmory, reset to default settings");  
  
  Serial.println("GAME MODE SETTINGS:");  
  if(currentGameMode == 0){
    Serial.println("HOLD THE POINT:");  
  }
  if(currentGameMode == 1){
    Serial.println("CAPTURE THE POINT:");  
  }
}

void list_games()
{
  Serial.println("GAMES:");
  Serial.println("0-hold the point");
  Serial.println("1-capture the point");
}

void set_game(int game)
{
  Serial.print("GAME: ");
  Serial.println(game);
  currentGameMode = game;
  saveSettings();
}

void buttonState(){
  return;
  /*lcd.setCursor(0,0);    
  lcd.print("                "); 
  lcd.setCursor(0,0);    
  lcd.print("DEBUG:");
  lcd.setCursor(7,0);
  lcd.print(redButtonState);
    
  lcd.setCursor(12,0);    
  lcd.print(greenButtonState);*/
}


// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );
}
 
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
}

void printDigits(int digits) {
  //выводим время через ":"
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

bool isTimeSet = false; 

void test_clock()
{  

}

