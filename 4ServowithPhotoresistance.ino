#include <Servo.h>

Servo myservo1; 
Servo myservo2;
Servo myservo3;
Servo myservo4;

int potpin1 = 0; //将A0作为servo1的控制信号输入端口
int potpin2 = 1;
int potpin3 = 2;
int potpin4 = 3;
int val1; //控制信号输入
int val2; 
int val3; 
int val4; 

void setup()
{
myservo1.attach(6); //将pin6作为servo1的PWM波输出端口
myservo2.attach(9);
myservo3.attach(10);
myservo4.attach(11);
Serial.begin(9600);
}
void loop() {
{ 

carDetect();
servoRotate();
delay(5); 
} 
}

void carDetect(){
  int compare = 300;//光敏电阻接负极，10K接正极，白天未遮光val约等于60，遮光约等于300-500
  val1 = analogRead(potpin1); //测量A0输入
  val1 = (val1>compare)? 90 : 0 ;//若val1大于compare，说明有车遮挡电阻，旋转开启大门至九十度，反之保持在零度angle: the value to write to the servo, from 0 to 180
  val2 = analogRead(potpin2); 
  val2 = (val2>compare)? 90 : 0 ;
  val3 = analogRead(potpin3); 
  val3 = (val3>compare)? 90 : 0 ;
  val4 = analogRead(potpin4); 
  val4 = (val4>compare)? 90 : 0 ;
}

void servoRotate(){
  myservo1.write(val1); 
  Serial.println(val1);
  
  myservo2.write(val2); 
  
  myservo3.write(val3); 
  
  myservo4.write(val4); 

}

