// include the library code:
#include <LiquidCrystal.h>
#include <Servo.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 42, d5 = 40, d6 = 38, d7 = 36;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//for four servos
Servo myservo1; 
Servo myservo2;
Servo myservo3;
Servo myservo4;

int gateopen = 0;//开门角度
int gateclose = 90;//关门角度
int angle[]={gateclose,gateclose,gateclose,gateclose}; //舵机旋转角度

int Gatecar = 0;//读取A0的值，判断大门前是否有车，1为有0为无
int state[] = {0,0,0};//三个停车位的状态矩阵，1为有车，0为无车
int compare = 700;//A0A1等端口的输入值将于此值比较，建议为500，但根据光照强度不同该值可能需要改变
int leavecar = 0;//在Closegate123中被赋值，用于指示那辆车正在离开
int bookedspace;//用于储存蓝牙预定的是哪一个车位
int bluetoothPayment =0;//用于确认是否通过蓝牙支付
int bluetoothArrive = 1;//用于防止按下arrive时大门立刻关闭

int GatePin = A0;
int Room1Pin = A1;
int Room2Pin = A2;
int Room3Pin = A3;

int LED1 = 22;
int LED2 = 24;
int LED3 = 26;

int where = 0;  //检测程序在哪一状态
/* UART FSM States */
typedef enum
{
  IDLE = 0U,
  SelectSpace,
  OpenMainGate,
  OpenSelectGate,
  CloseSelectGate,
  OpenGate1,
  CloseGate1,
  OpenGate2,
  CloseGate2,
  OpenGate3,
  CloseGate3,
  PayBill,
  OpenMainGate2,
  CloseMainGate2,
  Full,
} UART_State_t;

/* FSM variables */
UART_State_t txState = IDLE; // State register
int StartTime[] = {0,0,0,0};            //计时用
int StopTime[] = {0,0,0,0};
int TotalTime = 0;

const byte interruptbuttonPin0 = 48;
const byte interruptbuttonPin1 = 50;
const byte interruptbuttonPin2 = 52;
const byte interruptbuttonPin3 =46;



/*for bluetooth*/
#define blueToothSerial   Serial2     //set serial2(USART1) as blueToothSerial
const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;
#define p (1<<21)

void setup() {
  // put your setup code here, to run once:
  
  
  Serial.begin(9600);
  pinMode(interruptbuttonPin1, INPUT_PULLUP);
  pinMode(interruptbuttonPin2, INPUT_PULLUP);
  pinMode(interruptbuttonPin3, INPUT_PULLUP);
  pinMode(interruptbuttonPin0, INPUT_PULLUP);
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);
  digitalWrite(LED1,HIGH);
  digitalWrite(LED2,HIGH);
  digitalWrite(LED3,HIGH);
  
  myservo1.attach(2); //将pin6作为servo1的PWM波输出端口
  myservo2.attach(3);
  myservo3.attach(4);
  myservo4.attach(9);
    lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
  carDetect();//改到这里，仅在最开始运行一次
  setupBlueToothConnection();

  //attachInterrupt(interruptbuttonPin1, button1, RISING);
  //attachInterrupt(interruptbuttonPin2, button2, RISING);
  //attachInterrupt(interruptbuttonPin3, button3, RISING);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(50);//这个delay很重要，让analog接口的值有足够时间改变
  if(where==0||where==2){ 
  if(digitalRead(interruptbuttonPin1)==1)
    txState = OpenGate1;
  if(digitalRead(interruptbuttonPin2)==1)
    txState = OpenGate2;
  if(digitalRead(interruptbuttonPin3)==1)
    txState = OpenGate3;
  }
  carpark_state_machine();
  servoRotate();
  LCDdebug();
  BluetoothRecvWithStartEndMarkers();
  showNewData();
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

int newcarspace = 0;

void Selectspace(){//有车来时为其安排停车位
  newcarspace = 0;
  for(int i=0;i<=2;i++){//安排停车位
    if (state[i]==0){
      newcarspace = i+1;//安排到1，2，3号停车位中的一个
      Serial.print("Selectspace您的停车位是");
      Serial.print(newcarspace);
      Serial.println("号停车位hh");
      state[i]=1;//将该停车位标记为已占用
      break;
      }
  else {
    Serial.print(i+1);
    Serial.println("号停车位已被占用");
        }
    }
    if (newcarspace == 0){
    Serial.println("停车位已满");
    }
}


  int timelimit1 = 0; //用于限制各状态循环次数
  int timelimit2 = 0; //用于限制各状态循环次数
  int timelimit3 = 0;
  int timelimit4 = 0;
  int timelimit5 = 0;
  int timelimit6 = 0;
  int timelimit7 = 0;
void carpark_state_machine()
{
  switch(txState)
  {
    case IDLE:
    if(where==0)
    //Serial.print("正常进入IDLE");//debug
      timelimit1 =0 ;
      timelimit2 =0 ;
      timelimit3 =0 ;
      timelimit4 =0 ;
      timelimit5 =0 ;
      timelimit6 =0 ;
      timelimit7 =0 ;//空闲时将所有timelimit置零，为其他程序做准备
      txState = analogRead(GatePin) >=compare ? SelectSpace : IDLE;//判断是否有车到大门处，若有则开始工作。
      break;
    case SelectSpace:
      Selectspace();//安排停车位,其值写入“newcarspace”。
      /*txState = newcarspace == 0 ? Full : OpenMainGate;*/
      if(newcarspace ==0){
        Serial.println("我在SelectSpace状态里，newcarspace是零，我现在要去Full状态");
      txState=Full;
      }
      else{
      txState=OpenMainGate;
      }
      break;
    case OpenMainGate:
      where = 1;//indicate it's working in car-going-in circle
      if (timelimit1 == 0) {
      Serial.println("大门已打开");
      switch (newcarspace) {
    case 1:
            digitalWrite(LED1,LOW);
      break;
    case 2:
            digitalWrite(LED2,LOW);
      break;
          case 3:
            digitalWrite(LED3,LOW);
      break;
    default:
      // if nothing else matches, do the default
      // default is optional
      break;
  }
      
      angle[0] = gateopen;
      Serial.print("OpenMainGate您的车位是");
      Serial.print(newcarspace);
      Serial.println("号车位");
      Serial.println("请跟随LED灯指引");
      timelimit1 = 1;//限制这部分循环的次数为一
      }
      if(analogRead(GatePin) <=compare && bluetoothArrive == 1){
      txState=OpenSelectGate;
      }
      else{
      txState=OpenMainGate;
      }
      break;
    case OpenSelectGate:
      if (timelimit2 == 0) {
      Serial.println("检测到车已进入停车场，大门已关闭");
      angle[0] = gateclose;
      Serial.println("对应的门已打开");
      angle[newcarspace] = gateopen;
      timelimit2 =1;
      }
      txState = analogRead(newcarspace) >=compare ? CloseSelectGate : OpenSelectGate;
      delay(1000);
      break;
    case CloseSelectGate:
    if (timelimit3 ==0) {
      Serial.println("对应的门已关闭");
      switch (newcarspace) {
    case 1:
            digitalWrite(LED1,HIGH);
      break;
    case 2:
            digitalWrite(LED2,HIGH);
      break;
          case 3:
            digitalWrite(LED3,HIGH);
      break;
    default:
      // if nothing else matches, do the default
      // default is optional
      break;
  }

      angle[newcarspace] = gateclose;
      /*if(newcarspace == 3){
  for (int pos = 90; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo4.write(pos);              // tell servo to go to position in variable 'pos'
        Serial.println("我在反向跳舞");
    delay(15);                       // waits 15ms for the servo to reach the position
  }
      }*/
      StartTime[newcarspace] = millis()/1000;
      Serial.print("开始计时，起始时间为");
      Serial.println(StartTime[newcarspace]);
      timelimit3 =1;
      }
      txState = IDLE;
      where = 0;
      break;

    case OpenGate1:
    if (timelimit4 ==0) {
      Serial.println("检测到1号停车位车主按下了离开按钮，1号门已开启，其他车主请等待");
      angle[1] = gateopen;
      StopTime[1] = millis()/1000;
      Serial.print("停止计时，停止时间为");
      Serial.println(StopTime[1]);
      TotalTime = StopTime[1]-StartTime[1];
      timelimit4 =1;
    }
      if (analogRead(1)<compare)
      txState = CloseGate1;
      else
      txState = OpenGate1;
      break;
    case CloseGate1:
      Serial.println("检测到1号车主已离开停车位，对应的门已关闭");
      leavecar = 1;
      angle[1] = gateclose;
      if (analogRead(1)<compare)
        txState = PayBill;
      else 
        txState = CloseGate1;
      break;

     case OpenGate2:
    if (timelimit4 ==0) {
      Serial.println("检测到2号停车位车主按下了离开按钮，2号门已开启，其他车主请等待");
      angle[2] = gateopen;
      StopTime[2] = millis()/1000;
      Serial.print("停止计时，停止时间为");
      Serial.println(StopTime[2]);
      TotalTime = StopTime[2]-StartTime[2];
      timelimit4 =1;
    }
      if (analogRead(2)<compare)
      txState = CloseGate2;
      else
      txState = OpenGate2;
      break;
    case CloseGate2:
      Serial.println("检测到2号车主已离开停车位，对应的门已关闭");
      leavecar = 2;
      angle[2] = gateclose;
      if (analogRead(2)<compare)
        txState = PayBill;
      else 
        txState = CloseGate2;
      break;

    case OpenGate3:
    if (timelimit4 ==0) {
      Serial.println("检测到3号停车位车主按下了离开按钮，3号门已开启，其他车主请等待");
      angle[3] = gateopen;
      StopTime[3] = millis()/1000;
      Serial.print("停止计时，停止时间为");
      Serial.println(StopTime[3]);
      TotalTime = StopTime[3]-StartTime[3];
      timelimit4 =1;
    }
      if (analogRead(3)<compare)
      txState = CloseGate3;
      else
      txState = OpenGate3;
      break;
    case CloseGate3:
      Serial.println("检测到3号车主已离开停车位，对应的门已关闭");
      leavecar = 3;
      angle[3] = gateclose;
      if (analogRead(3)<compare)
        txState = PayBill;
      else 
        txState = CloseGate3;
      break;
      
    /*case OpenSelectGate2:
    if (timelimit4 ==0) {
      Serial.println("检测到车主按下了离开按钮，对应的门已开启");
      angle[newcarspace] = gateopen;
      StopTime = millis()/1000;
      Serial.print("停止计时，停止时间为");
      Serial.println(StopTime);
      TotalTime = StopTime-StartTime;
      timelimit4 =1;
    }
      txState = analogRead(newcarspace) <=compare ? CloseSelectGate2 : OpenSelectGate2;
      break;
    case CloseSelectGate2:
      Serial.println("检测到车主已离开停车位，对应的门已关闭");
      angle[newcarspace] = gateclose;
        txState = PayBill;
      break;*/
    case PayBill:
    if (timelimit5 ==0) {
    Serial.print("请付费,停车总时长为");
    Serial.println(TotalTime);
      timelimit5 =1;
    }
        if (digitalRead(interruptbuttonPin0) == 1||bluetoothPayment ==1){
          txState = OpenMainGate2;
          Serial.println("检测到车主已付费");
          bluetoothPayment = 0;
        }
        else{
          txState = PayBill;
        }       
        break;
    case OpenMainGate2:
    if (timelimit6 ==0) {
      Serial.println("大门已开启");
      angle[0] = gateopen;
      timelimit6 =1;
    }  
        txState = analogRead(GatePin) >= compare? CloseMainGate2 : OpenMainGate2;
      break;
    case CloseMainGate2:
    if (timelimit7 ==0) {
      Serial.println("检测到车主已离开，大门已关闭");
      angle[0] = gateclose;
      timelimit7 =1;
    state[leavecar-1]=0;//车位已空闲，改变state状态为0
    }  
        txState = analogRead(GatePin) <=compare ? IDLE:CloseMainGate2;
        where = 0;
      break;    
   case Full:
        where = 2;
    Serial.println("停车位已满,请等待车主离开");//停车位满时转到这个状态，在有空位之前大门只出不进。
        txState = Full;
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
    lcd.print(state[0]);
    //lcd.print(angle[0]);
    lcd.setCursor(4, 1);
    lcd.print(state[1]);
    //lcd.print(angle[1]);
    lcd.setCursor(8, 1);
    lcd.print(state[2]);
    //lcd.print(angle[2]);
    lcd.setCursor(12, 1);
    lcd.print(newcarspace);
    //lcd.print(angle[3]);
  }
}

/*void button1(){
  txState = OpenGate1;
}
void button2(){
  txState = OpenGate2;
}
void button3(){
  txState = OpenGate3;
}*/

void BluetoothRecvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
 // if (Serial.available() > 0) {
    while (blueToothSerial.available() > 0 && newData == false) {
        rc = blueToothSerial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }

}

void showNewData() {
    if (newData == true) {
        Serial.print("You have sent ");
        blueToothSerial.print("You have sent ");
        Serial.println(receivedChars);
        blueToothSerial.println(receivedChars);
        if(strcmp(receivedChars, "state") == 0){
          int remainingSpace = 0;
          remainingSpace = 3- (state[0]+state[1]+state[2]);
          Serial.print("now there are ");
          Serial.print(remainingSpace);
          Serial.println(" space remaining");
          blueToothSerial.print("now there are");
          blueToothSerial.print(remainingSpace);
          blueToothSerial.println("space remaining");
        }
        else if (strcmp(receivedChars, "reserve") == 0){
          Selectspace();
          bookedspace = newcarspace;
          bluetoothArrive = 0;
          Serial.print("Received your order, your booked place is place ");
          Serial.print(bookedspace);
          Serial.println(".");
          blueToothSerial.print("Received your order, your booked place is place ");
          blueToothSerial.print(bookedspace);
          blueToothSerial.println(".");
        }
        else if (strcmp(receivedChars, "arrive") == 0){
          txState = OpenMainGate;
          newcarspace = bookedspace;
          bluetoothArrive = 1;
          Serial.print("Welcome, please go to space ");
          Serial.print(bookedspace);
          Serial.println(".");
          blueToothSerial.print("Welcome, please go to space ");
          blueToothSerial.print(bookedspace);
          blueToothSerial.println(".");
        }
        else if (strcmp(receivedChars, "leave") == 0){
          switch (bookedspace) {
  case 1:
    // statements
    txState = OpenGate1;
    break;
  case 2:
    // statements
    txState = OpenGate2;
    break;
  case 3:
    // statements
    txState = OpenGate3;
    break;
  default:
    // statements
    break;
}
          Serial.println("Received your order, The door has been opened.");
          blueToothSerial.println("Received your order, The door has been opened.");
        }
         else if (strcmp(receivedChars, "pay") == 0){
          bluetoothPayment =1;
          Serial.println("Your payment has been confirmed, thanks for your using");
          blueToothSerial.println("Your payment has been confirmed, thanks for your using");
        }
          else{
          Serial.println("unknown order");
          blueToothSerial.println("unknown order");
        }
        newData = false;
    }
}


void setupBlueToothConnection()
{
    blueToothSerial.begin(38400);                           // Set BluetoothBee BaudRate to default baud rate 38400
    blueToothSerial.print("\r\n+STWMOD=0\r\n");             // set the bluetooth work in slave mode
    blueToothSerial.print("\r\n+STNA=Test\r\n");    // set the bluetooth name as "SeeedBTSlave"
    blueToothSerial.print("\r\n+STOAUT=1\r\n");             // Permit Paired device to connect me
    blueToothSerial.print("\r\n+STAUTO=0\r\n");             // Auto-connection should be forbidden here
    blueToothSerial.print("\r\n+STPIN=1234\r\n");
    delay(2000);                                            // This delay is required.
    blueToothSerial.print("\r\n+INQ=1\r\n");                // make the slave bluetooth inquirable
    Serial.println("The slave bluetooth is inquirable!");
    delay(2000);                                            // This delay is required.
    blueToothSerial.flush();
}
