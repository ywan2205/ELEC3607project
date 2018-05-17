/*
  LiquidCrystal Library - Hello World
 This sketch prints "Hello World!" to the LCD
 and shows the time.
*/

// include the library code:
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int potpin1 = 0; //将A0作为servo1的控制信号输入端口
int potpin2 = 1;
int potpin3 = 2;
int potpin4 = 3;
int val1; //控制信号输入
int val2; 
int val3; 
int val4; 

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
  //detect input port A0
  pinMode(potpin1,INPUT);
  Serial.begin(9600);
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  LCDhowmuchspace();
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);
}

void LCDhowmuchspace(){
  int spareroom = 0;
  carDetect();
  if(val1 == 90){
    spareroom = 0;
    if(val2 == 90)
    spareroom++;
    if(val3 == 90)
    spareroom++;
    if(val4 == 90)
    spareroom++;
    lcd.setCursor(0, 0);
    if(spareroom ==3)
    lcd.print("Three spare room");
    else if(spareroom ==2)
    lcd.print("Two spare room  ");
    else if((spareroom ==1))
    lcd.print("One spare room  ");
    else
    lcd.print("No spare room   ");
  }
  else{
    lcd.setCursor(0, 0);
    lcd.print("Hello world     ");//use space to overwrite the previous words
  }
}

void carDetect(){
  int compare = 800;//光敏电阻接负极，1K接正极，白天未遮光val约等于700，遮光约等于1000
  val1 = analogRead(potpin1); //测量A0输入
  Serial.println(val1);
  val1 = (val1>compare)? 90 : 0 ;//若val1大于compare，说明有车遮挡电阻，旋转开启大门至九十度，反之保持在零度angle: the value to write to the servo, from 0 to 180
  val2 = analogRead(potpin2); 
  val2 = (val2>compare)? 90 : 0 ;
  val3 = analogRead(potpin3); 
  val3 = (val3>compare)? 90 : 0 ;
  val4 = analogRead(potpin4); 
  val4 = (val4>compare)? 90 : 0 ;
}
