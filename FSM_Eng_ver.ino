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

int gateopen = 0;//open angle
int gateclose = 90;//close angle
int angle[]={gateclose,gateclose,gateclose,gateclose}; //rotate angle

int Gatecar = 0;
int state[] = {0,0,0};
int compare = 700;
int leavecar = 0;
int bookedspace;
int bluetoothPayment =0;
int bluetoothArrive = 1;

int GatePin = A0;
int Room1Pin = A1;
int Room2Pin = A2;
int Room3Pin = A3;

int LED1 = 22;
int LED2 = 24;
int LED3 = 26;

int where = 0;  
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
int StartTime[] = {0,0,0,0};            
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
  
  myservo1.attach(2); 
  myservo2.attach(3);
  myservo3.attach(4);
  myservo4.attach(9);
    lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
  carDetect();
  setupBlueToothConnection();

  //attachInterrupt(interruptbuttonPin1, button1, RISING);
  //attachInterrupt(interruptbuttonPin2, button2, RISING);
  //attachInterrupt(interruptbuttonPin3, button3, RISING);
}

void loop() {

  delay(50);
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

  Gatecar = Gatecar>compare? 1:0;
  int i=0;
  for(;i<=2;i++){
  state[i] = analogRead(i+1);

  state[i] = state[i]>compare? 1:0;
  }
}

int newcarspace = 0;

void Selectspace(){
  newcarspace = 0;
  for(int i=0;i<=2;i++){
    if (state[i]==0){
      newcarspace = i+1;
      Serial.print("Selectspace Your Space is");
      Serial.print(newcarspace);
      Serial.println(" ");
      state[i]=1;
      break;
      }
  else {
    Serial.print(i+1);
    Serial.println("was occupied");
        }
    }
    if (newcarspace == 0){
    Serial.println("no spare room");
    }
}


  int timelimit1 = 0; 
  int timelimit2 = 0; 
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
      timelimit1 =0 ;
      timelimit2 =0 ;
      timelimit3 =0 ;
      timelimit4 =0 ;
      timelimit5 =0 ;
      timelimit6 =0 ;
      timelimit7 =0 ;
      txState = analogRead(GatePin) >=compare ? SelectSpace : IDLE;
      break;
    case SelectSpace:
      Selectspace();
      if(newcarspace ==0){
      txState=Full;
      }
      else{
      txState=OpenMainGate;
      }
      break;
    case OpenMainGate:
      where = 1;//indicate it's working in car-going-in circle
      if (timelimit1 == 0) {
      Serial.println("gate opened");
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
      Serial.print("OpenMainGate Your space is");
      Serial.print(newcarspace);
      Serial.println(" ");
      Serial.println("please follow the instruction");
      timelimit1 = 1;
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
      Serial.println("Gate closed");
      angle[0] = gateclose;
      Serial.println("corresponding door open");
      angle[newcarspace] = gateopen;
      timelimit2 =1;
      }
      txState = analogRead(newcarspace) >=compare ? CloseSelectGate : OpenSelectGate;
      delay(1000);
      break;
    case CloseSelectGate:
    if (timelimit3 ==0) {
      Serial.println("corresponding gate closed");
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
      StartTime[newcarspace] = millis()/1000;
      Serial.print("start time is");
      Serial.println(StartTime[newcarspace]);
      timelimit3 =1;
      }
      txState = IDLE;
      where = 0;
      break;

    case OpenGate1:
    if (timelimit4 ==0) {
      Serial.println("first room user wants to leave");
      angle[1] = gateopen;
      StopTime[1] = millis()/1000;
      Serial.print("stop time is");
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
      Serial.println("corresponding door closed");
      leavecar = 1;
      angle[1] = gateclose;
      if (analogRead(1)<compare)
        txState = PayBill;
      else 
        txState = CloseGate1;
      break;

     case OpenGate2:
    if (timelimit4 ==0) {
      Serial.println("second room user wants to leave");
      angle[2] = gateopen;
      StopTime[2] = millis()/1000;
      Serial.print("stop time is");
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
      Serial.println("corresponding door closed");
      leavecar = 2;
      angle[2] = gateclose;
      if (analogRead(2)<compare)
        txState = PayBill;
      else 
        txState = CloseGate2;
      break;

    case OpenGate3:
    if (timelimit4 ==0) {
      Serial.println("third room user wants to leave");
      angle[3] = gateopen;
      StopTime[3] = millis()/1000;
      Serial.print("stop time is");
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
      Serial.println("corresponding gate closed");
      leavecar = 3;
      angle[3] = gateclose;
      if (analogRead(3)<compare)
        txState = PayBill;
      else 
        txState = CloseGate3;
      break;
    case PayBill:
    if (timelimit5 ==0) {
    Serial.print("total time is");
    Serial.println(TotalTime);
      timelimit5 =1;
    }
        if (digitalRead(interruptbuttonPin0) == 1||bluetoothPayment ==1){
          txState = OpenMainGate2;
          Serial.println("payment confirmed");
          bluetoothPayment = 0;
        }
        else{
          txState = PayBill;
        }       
        break;
    case OpenMainGate2:
    if (timelimit6 ==0) {
      Serial.println("gate opened");
      angle[0] = gateopen;
      timelimit6 =1;
    }  
        txState = analogRead(GatePin) >= compare? CloseMainGate2 : OpenMainGate2;
      break;
    case CloseMainGate2:
    if (timelimit7 ==0) {
      Serial.println("gate closed");
      angle[0] = gateclose;
      timelimit7 =1;
    state[leavecar-1]=0;
    }  
        txState = analogRead(GatePin) <=compare ? IDLE:CloseMainGate2;
        where = 0;
      break;    
   case Full:
        where = 2;
    Serial.println("Full");
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
