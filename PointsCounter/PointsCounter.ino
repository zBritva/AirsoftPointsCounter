#include <EEPROM.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);
int ledPin = 10;

//const int greenPin = 2;
//const int redPin = 3;

const int adminButton = 5;
const int incButton = 11;
const int decButton = 10;
const int confButton = 9;

int redPoints = 0;
int greenPoints = 0;

int redPointsLoop = 0;
int greenPointsLoop = 0;

const int redButton = 7;
const int greenButton = 6;

const int redPointsAddr = 0;
const int greenPointsAddr = 10;

int redButtonState = 0;
int greenButtonState = 0;
int adminButtonState = 0;

bool adminMode = false;

char serialBuffer[10];

// the setup function runs once when you press reset or power the board
void setup() {
  lcd.init();
  lcd.backlight(); 
  lcd.setCursor(0,0);
  lcd.print("Hello, players!");
  delay(2000);
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
    //lcd.clear();
    buttonState();
  //  
    //return;
    
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
  else{
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
  admin();
  greenButtonState = 0;
  redButtonState = 0;
  adminButtonState = 0;
}

void admin(){
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
      adminMode = false;    
    }
    else{
      if (Serial.available()) {  
        serialBuffer = Serial.readString();

        if(serialBuffer == "help"){
          printHelp
        }
      }
    }
  }
}

void printHelp(){
  Serial.println("help - print this");  
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
