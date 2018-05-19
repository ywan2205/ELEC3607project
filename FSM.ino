int newcar = 0;//判断大门前是否有车，1为有0为无
int newcar2 = 0;//判断A1前是否有车，1为有0为无
int state[] = {1,0,0};


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
} UART_State_t;

/* FSM variables */
UART_State_t txState = IDLE; // State register
int selectedSpace = 0;       // 选择的停车位
int StartTime = 0;            //计时用
int StopTime = 0;
int TotalTime = 0;

unsigned char txData;        // Current data register
unsigned char txSym = 1;     // Bit to transmit
unsigned char txStart = 0;   // 1 if UART is sending data, 0 if idle

const byte interruptbuttonPin1 = 48;
const byte interruptbuttonPin2 = 50;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(interruptbuttonPin1, INPUT_PULLUP);
  pinMode(interruptbuttonPin2, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  carDetect();
  carpark_state_machine();
}
void carDetect(){
  newcar = analogRead(A0);
  newcar = newcar>500? 1:0;//1：大门有车   0：大门无车

  newcar2 = analogRead(A1);
  newcar2 = newcar2>300? 1:0;//对应的门处有车
}

int Selectspace(){//有车来时为其安排停车位
  int newcarspace = 0;
  for(int i=0;i<=2;i++){//安排停车位
    if (state[i]==0){
      newcarspace = i+1;//安排到1，2，3号停车位中的一个
      Serial.print("您的停车位是");
      Serial.print(newcarspace);
      Serial.println("号停车位");
      state[i]=1;//将该停车位标记为已占用
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
      txState = newcar == 1 ? SelectSpace : IDLE;//判断是否有车到大门处，若有则开始工作。
      break;
    case SelectSpace:
      selectedSpace = Selectspace();//安排停车位
      //txState = selectedSpace == 0 ? Full : OpenMainGate;过会要把full部分补上
      txState = OpenMainGate;
      break;
    case OpenMainGate:
      if (timelimit1 == 0) {
      Serial.println("大门已打开");
      Serial.print("您的车位是");
      Serial.print(selectedSpace);
      Serial.println("号车位");
      Serial.println("请跟随LED灯指引");
      timelimit1 = 1;//限制这部分循环的次数为一
      }
      txState = newcar2 == 1 ? OpenSelectGate : OpenMainGate;
      break;
    case OpenSelectGate:
      if (timelimit2 == 0) {
      Serial.println("检测到车已进入停车场，大门已关闭");
      Serial.println("对应的门已打开");
      timelimit2 =1;
      }
      txState = newcar2 == 0 ? CloseSelectGate : OpenSelectGate;
      break;
    case CloseSelectGate:
    if (timelimit3 ==0) {
      Serial.println("对应的门已关闭");
      StartTime = millis();
      Serial.print("开始计时，起始时间为");
      Serial.println(StartTime);
      timelimit3 =1;
      }
      txState = digitalRead(interruptbuttonPin1) == 1 ? OpenSelectGate2 : CloseSelectGate;
      break;
    case OpenSelectGate2:
    if (timelimit4 ==0) {
      Serial.println("检测到车主按下了离开按钮，对应的门已开启");
      StopTime = millis();
      Serial.print("停止计时，停止时间为");
      Serial.println(StopTime);
      TotalTime = StopTime-StartTime;
      timelimit4 =1;
    }
      txState = newcar2 == 1 ? CloseSelectGate2 : OpenSelectGate2;
      break;
    case CloseSelectGate2:
      Serial.println("检测到车主已离开停车位，对应的门已关闭");
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
      timelimit6 =1;
    }  
        txState = newcar == 1? CloseMainGate2 : OpenMainGate2;
      break;
    case CloseMainGate2:
    if (timelimit7 ==0) {
      Serial.println("检测到车主已付费，大门已开启");
      timelimit7 =1;
    }  
        txState = IDLE;
      break;    
    /*case Full:
    Serial.println("停车位已满");//停车位满时转到这个状态，在有空位之前大门只出不进。
    
      break;*/
    default:
        break;
  }
}

