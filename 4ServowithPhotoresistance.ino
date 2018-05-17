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

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
  //detect input port A0
  pinMode(A0,INPUT);
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
  int val1 = 0;
  val1 = analogRead(A0);
  val1 = (val1>300)? 90 : 0 ;
  if(val1 == 90){
    lcd.setCursor(0, 0);
    lcd.print("four spare room");
  }
  else{
    lcd.setCursor(0, 0);
    lcd.print("Hello world     ");//use space to overwrite the previous words
  }
    
}
  
