#include <Wire.h>
#include <LiquidCrystal_I2C.h>



/** 
* Original: @author : Alex Baizeau 
* 
* Changed: Serdyukov Anton, devdotnet.org
* for device Arduino IIC / I2C Serial 3.2" LCD 2004
* Module Display GY-IICLCD GY-LCD-V1 PCF8574 PCF8574T
* from http://dx.com/p/arduino-iic-i2c-serial-3-2-lcd-2004-module-display-138611
*/  

 
#define LCD_I2C_ADDR    0x20


LiquidCrystal_I2C       lcd(LCD_I2C_ADDR,16,2);
//LiquidCrystal_I2C lcd(0x20, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
//LiquidCrystal_I2C lcd(0x20);
 
void setup()
{
lcd.begin (16,2);
lcd.setBacklight(HIGH); // Backlight on
//lcd.clear(); // Clear the lcd
lcd.setCursor ( 0, 1 );        
lcd.print("222 olo Olo");
}
 
void loop()
{
lcd.setCursor ( 0, 0 );        
lcd.print("222 olo Olo");
lcd.clear(); // Clear the lcd
delay(10000);
}
