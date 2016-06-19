#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);
int const ledPin = 10;
int const adminButtonPin = 5;
int const redButtonPin = 7;
int const greenButtonPin = 6;

int redValue = 0;
int greenValue = 0;
int adminValue = 0;

// the setup function runs once when you press reset or power the board
void setup() {
  lcd.init();
  lcd.backlight(); 
  lcd.setCursor(0,0);
  lcd.print("Hello, players!");
  delay(2000);
  pinMode(redButtonPin, INPUT);
  pinMode(greenButtonPin, INPUT);
}


// the loop function runs over and over again forever
void loop() {
  redValue = digitalRead(redButtonPin);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(redValue);
  greenValue = digitalRead(greenButtonPin);
  lcd.setCursor(3,0);
  lcd.print(greenValue);
  adminValue = digitalRead(adminButtonPin);
  lcd.setCursor(6,0);
  lcd.print(adminValue);
  delay(1000);
}
