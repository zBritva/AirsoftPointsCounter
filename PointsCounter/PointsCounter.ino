#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);
int ledPin = 10;

int currentGameMode = 0;

//const int greenPin = 2;
//const int redPin = 3;

const int adminButton = 5;
const int incButton = 11;
const int decButton = 10;
const int confButton = 9;

//common variables
int redPoints = 0;
int greenPoints = 0;

int redPointsLoop = 0;
int greenPointsLoop = 0;

const int redButton = 7;
const int greenButton = 6;

const int redPointsAddr = 0; //need 2 bytes
const int greenPointsAddr = 2; //need 2 bytes
const int gameModeAddr = 4; //need 1 byte
const int captureTimeAddr = 5; //need 1 byte

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

void checkAdminMode();
void buttonState();

void loadPoints(){
  redPoints = EEPROM.read(redPointsAddr);
  redPointsLoop = EEPROM.read(redPointsAddr + 1);
  
  greenPoints = EEPROM.read(greenPointsAddr);
  greenPointsLoop = EEPROM.read(greenPointsAddr + 1);  
}

void savePoints(){
  EEPROM.write(redPointsAddr, redPoints);
  EEPROM.write(redPointsAddr + 1, redPointsLoop);
  
  EEPROM.write(greenPointsAddr, greenPoints);
  EEPROM.write(greenPointsAddr + 1, greenPointsLoop);
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

// the setup function runs once when you press reset or power the board
void setup() {
  loadSettings();
  lcd.init();
  lcd.backlight(); 
  lcd.setCursor(0,0);
  lcd.print("Hello, players!");
  delay(1000);
  // initialize digital pin 13 as an output.
  //pinMode(ledPin, OUTPUT);
  pinMode(redButton, INPUT);
  pinMode(greenButton, INPUT);
  lcd.clear();
  greenButtonState = 0;
  redButtonState = 0;
  greenPointsLoop = 0;
  redPointsLoop = 0;
  loadPoints();
  adminButtonState = digitalRead(adminButton);
}

void blink(){
  digitalWrite(ledPin, HIGH);
  delay(750);              
  digitalWrite(ledPin, LOW);
  delay(750);
}

void printClock(){
  lcd.setCursor(0,0);    
  lcd.print("TIME:");  
}

void printRedPoints(){  
  lcd.setCursor(9,1);   
  lcd.print("RED:");
  lcd.setCursor(13,1);    
  lcd.print("   ");
  lcd.setCursor(13,1);    
  lcd.print(redPoints + (255 * redPointsLoop));
}

void printGreenPoints(){
  lcd.setCursor(0,1);   
  lcd.print("GREEN:");
  lcd.setCursor(6,1);    
  lcd.print("   ");
  lcd.setCursor(6,1);    
  lcd.print(greenPoints + (255 * greenPointsLoop));
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

// the loop function runs over and over again forever
void loop() {
  delay(1000);
  redButtonState = digitalRead(redButton);
  greenButtonState = digitalRead(greenButton);
  if(adminButtonState == 0){    
    if(currentGameMode == 0)
      holdThePoint();
    else
    if(currentGameMode == 1)
      captureThePoint();
    else{
      lcd.setCursor(0,1);    
      lcd.print("G.MODE ISN'T SET");
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
  lcd.clear();
  if (greenButtonState == 1 && redButtonState == 1){
    lcd.setCursor(0,1);    
    lcd.print(" CONFLICT STATE");
    return;
  }
  //Capturing enabled only if another team captured this point
  if (redButtonState == 1 && teamFlag != 2){
    capturing++;
    
    if(capturing > captureTime){
      teamFlag = 2;
      capturing = 0;
    }
  }
  else{
    capturing = 0;    
  }
  //Capturing enabled only if another team captured this point
  if (greenButtonState == 1  && teamFlag != 1){
    capturing++;
        
    if(capturing > captureTime){
      teamFlag = 1;
      capturing = 0;
    }
  }
  else{
    capturing = 0;
  }
  
  if(capturing > 0){
    lcd.setCursor(0,1);    
    lcd.print("CAPTURING: ");
    lcd.print(capturing);
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
  lcd.clear();
  if (greenButtonState == 1 && redButtonState == 1){
    lcd.setCursor(0,1);    
    lcd.print(" CONFLICT STATE");
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
  lcd.setCursor(0,1);    
  lcd.print("PRES.BTNs TO RST"); 
  buttonState();
  redButtonState = digitalRead(redButton);
  greenButtonState = digitalRead(greenButton);  
  if (greenButtonState == 1 && redButtonState == 1){
    redPoints = 0;
    greenPoints = 0;
    redPointsLoop = 0;
    greenPointsLoop = 0;
    savePoints();
    lcd.setCursor(0,1);
    lcd.print("                "); 
    lcd.setCursor(0,1);
    lcd.print("RESET DONE"); 
    delay(1000);
  }
}


void checkAdminMode(){
  adminButtonState = digitalRead(adminButton);
  if (adminButtonState == 1){
    lcd.setCursor(0,0);    
    lcd.print("                "); 
    lcd.setCursor(0,0);    
    lcd.print("ADMIN MODE");

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
      }
    }
  }
  else{
    adminMode = false;
    Serial.println("Admin mode: OFF");  
    Serial.end();
  }
}

void printHelp(){
  Serial.println("help                - print this");
  Serial.println("set game <number>   - set game mode");
  Serial.println("list games          - print avialable game modes");
  Serial.println("game                - print current game mode");
  //Serial.println("settings            - print current settings of game");
  //Serial.println("set <param> <value> - set settings value");
  //Serial.println("reset               - erase memmory, reset to default settings");  
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
  lcd.setCursor(0,0);    
  lcd.print("                "); 
  lcd.setCursor(0,0);    
  lcd.print("DEBUG:");
  lcd.setCursor(7,0);
  lcd.print(redButtonState);
    
  lcd.setCursor(12,0);    
  lcd.print(greenButtonState);
}
