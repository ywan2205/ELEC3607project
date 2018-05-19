// include the library code:
#include <LiquidCrystal.h>
#include <Servo.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//for four servos
Servo myservo1; 
Servo myservo2;
Servo myservo3;
Servo myservo4;

int angle[]={0,0,0,0}; //舵机旋转角度
int gateopen = 90;//开门角度
int gateclose = 0;//关门角度

int Gatecar = 0;//读取A0的值，判断大门前是否有车，1为有0为无
int state[] = {1,0,0};//三个停车位的状态矩阵，1为有车，0为无车
int compare = 300;//A0A1等端口的输入值将于此值比较，建议为500，但根据光照强度不同该值可能需要改变

/* UART FSM States */
typedef enum
{
  IDLE = 0U,
  SelectSpace,
  OpenMainGate,
  OpenSelectGate,
  CloseSelectGate,
  OpenSelectGate2,
  CloseSelectGate2,
  PayBill,
  OpenMainGate2,
  CloseMainGate2,
  Full,
} UART_State_t;

/* FSM variables */
UART_State_t txState = IDLE; // State register
int selectedSpace = 0;       // 选择的停车位
int StartTime = 0;            //计时用
int StopTime = 0;
int TotalTime = 0;

const byte interruptbuttonPin1 = 48;
const byte interruptbuttonPin2 = 50;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(interruptbuttonPin1, INPUT_PULLUP);
  pinMode(interruptbuttonPin2, INPUT_PULLUP);
  myservo1.attach(6); //将pin6作为servo1的PWM波输出端口
  myservo2.attach(7);
  myservo3.attach(8);
  myservo4.attach(9);
    lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
}

void loop() {
  // put your main code here, to run repeatedly:
  carDetect();
  delay(50);//这个delay很重要
  carpark_state_machine();
  servoRotate();
  LCDdebug();
}
void carDetect(){
  Gatecar = analogRead(A0);
  //Serial.print("A0端口的值为");
  //Serial.println(Gatecar);
  Gatecar = Gatecar>compare? 1:0;//1：大门有车   0：大门无车
  int i=0;
  for(;i<=2;i++){
  state[i] = analogRead(i+1);
  //Serial.print(i+1);
  //Serial.print("端口的值为");
  //Serial.println(state[i]);//debug
  state[i] = state[i]>compare? 1:0;//Roomi+1对应的门处有车?
  }
}

int Selectspace(){//有车来时为其安排停车位
  int newcarspace = 0;
  for(int i=0;i<=2;i++){//安排停车位
    if (state[i]==0){
      newcarspace = i+1;//安排到1，2，3号停车位中的一个
      Serial.print("您的停车位是");
      Serial.print(newcarspace);
      Serial.println("号停车位");
      //state[i]=1;//将该停车位标记为已占用，但是会被carDetect覆盖掉，没什么好的解决方法，干脆不用,反正我们假设每次只来一辆车
      break;
      }
  else {
    Serial.print(i+1);
    Serial.println("号停车位已被占用");
        }
    }
    if (newcarspace == 0)
    Serial.println("停车位已满");
    return newcarspace;
}


  int timelimit1 = 0; //用于限制各状态循环次数
  int timelimit2 = 0; //用于限制各状态循环次数
  int timelimit3 = 0;
  int timelimit4 = 0;
  int timelimit5 = 0;
  int timelimit6 = 0;
  int timelimit7 = 0 ;
void carpark_state_machine()
{
  switch(txState)
  {
    case IDLE:
      timelimit1 =0 ;
      timelimit2 =0 ;
      timelimit3 =0 ;
      timelimit4 =0 ;
      timelimit5 =0 ;
      timelimit6 =0 ;
      timelimit7 =0 ;//空闲时将所有timelimit置零，为其他程序做准备
      txState = Gatecar == 1 ? SelectSpace : IDLE;//判断是否有车到大门处，若有则开始工作。
      break;
    case SelectSpace:
      selectedSpace = Selectspace();//安排停车位,其值写入“selectedSpace”。
      txState = selectedSpace == 0 ? Full : OpenMainGate;
      break;
    case OpenMainGate:
      if (timelimit1 == 0) {
      Serial.println("大门已打开");
      angle[0] = gateopen;
      Serial.print("您的车位是");
      Serial.print(selectedSpace);
      Serial.println("号车位");
      Serial.println("请跟随LED灯指引");
      timelimit1 = 1;//限制这部分循环的次数为一
      }
      txState = Gatecar == 0 ? OpenSelectGate : OpenMainGate;
      break;
    case OpenSelectGate:
      if (timelimit2 == 0) {
      Serial.println("检测到车已进入停车场，大门已关闭");
      angle[0] = gateclose;
      Serial.println("对应的门已打开");
      angle[selectedSpace] = gateopen;
      timelimit2 =1;
      }
      txState = state[selectedSpace-1] == 1 ? CloseSelectGate : OpenSelectGate;
      break;
    case CloseSelectGate:
    if (timelimit3 ==0) {
      Serial.println("对应的门已关闭");
      angle[selectedSpace] = gateclose;
      StartTime = millis()/1000;
      Serial.print("开始计时，起始时间为");
      Serial.println(StartTime);
      timelimit3 =1;
      }
      txState = digitalRead(interruptbuttonPin1) == 1 ? OpenSelectGate2 : CloseSelectGate;
      break;
    case OpenSelectGate2:
    if (timelimit4 ==0) {
      Serial.println("检测到车主按下了离开按钮，对应的门已开启");
      angle[selectedSpace] = gateopen;
      StopTime = millis()/1000;
      Serial.print("停止计时，停止时间为");
      Serial.println(StopTime);
      TotalTime = StopTime-StartTime;
      timelimit4 =1;
    }
      txState = state[selectedSpace-1] == 0 ? CloseSelectGate2 : OpenSelectGate2;
      break;
    case CloseSelectGate2:
      Serial.println("检测到车主已离开停车位，对应的门已关闭");
      angle[selectedSpace] = gateclose;
        txState = PayBill;
      break;
    case PayBill:
    if (timelimit5 ==0) {
    Serial.print("请付费,停车总时长为");
    Serial.println(TotalTime);
      timelimit5 =1;
    }
        txState = digitalRead(interruptbuttonPin2) == 1 ? OpenMainGate2:PayBill;
        break;
    case OpenMainGate2:
    if (timelimit6 ==0) {
      Serial.println("检测到车主已付费，大门已开启");
      angle[0] = gateopen;
      timelimit6 =1;
    }  
        txState = Gatecar == 1? CloseMainGate2 : OpenMainGate2;
      break;
    case CloseMainGate2:
    if (timelimit7 ==0) {
      Serial.println("检测到车主已离开，大门已关闭");
      angle[0] = gateclose;
      timelimit7 =1;
    }  
        txState = Gatecar == 0 ? IDLE:CloseMainGate2;
      break;    
   case Full:
    Serial.println("停车位已满");//停车位满时转到这个状态，在有空位之前大门只出不进。
    if (state[0]+state[1]+state[2]==3)
        txState = Full;
        else 
        txState = IDLE;
      break;
    default:
        break;
  }
}


void servoRotate(){
  myservo1.write(angle[0]); 
  myservo2.write(angle[1]); 
  myservo3.write(angle[2]); 
  myservo4.write(angle[3]); 
}

unsigned previousMillis = 0;

void LCDdebug(){
  unsigned long currentMillis = millis();  
  if (currentMillis - previousMillis > 1000) {
    previousMillis = currentMillis;
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(analogRead(A0));//use space to overwrite the previous words
    lcd.setCursor(4, 1);
    lcd.print(analogRead(A1));
    lcd.setCursor(8, 1);
    lcd.print(analogRead(A2));
    lcd.setCursor(12, 1);
    lcd.print(analogRead(A3));
  }
}
