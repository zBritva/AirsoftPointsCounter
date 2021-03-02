#include <GyverTM1637.h>
#include <Wire.h> 

#define CLK 7
#define DIO 6

GyverTM1637 tm1637(CLK,DIO);
int current;

void setup()
{
  tm1637.brightness(5);
  Serial.begin(9600);
  
  Serial.print("Setup done\n");
}


void loop()
{
    tm1637.displayInt(current);
    delay(1000);
    current++;
    Serial.print(String(current));
}

