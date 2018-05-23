/*for order compare*/
const byte numChars = 32;
char receivedChars[numChars];

boolean newData = false;

/*for bluetooth*/
#include <SoftwareSerial.h>   //Software Serial Port
#define RxD 7
#define TxD 6

#define DEBUG_ENABLED  1

SoftwareSerial blueToothSerial(RxD,TxD);
void setup() {
    Serial.begin(9600);
    pinMode(RxD, INPUT);
    pinMode(TxD, OUTPUT);
    setupBlueToothConnection();
}

void loop() {
    BluetoothRecvWithStartEndMarkers();
    showNewData();
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
        if(strcmp(receivedChars, "returnState") == 0){
          Serial.println("now there are three space remaining");
          blueToothSerial.println("now there are three space remaining");
        }
        else if (strcmp(receivedChars, "bookPlace") == 0){
          Serial.println("Received your order, your booker place is place 4.");
          blueToothSerial.println("Received your order, your booker place is place 4.");
        }
         else if (strcmp(receivedChars, "payBill") == 0){
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
    blueToothSerial.print("\r\n+STNA=yuy\r\n");    // set the bluetooth name as "SeeedBTSlave"
    blueToothSerial.print("\r\n+STOAUT=1\r\n");             // Permit Paired device to connect me
    blueToothSerial.print("\r\n+STAUTO=0\r\n");             // Auto-connection should be forbidden here
    blueToothSerial.print("\r\n+STPIN=1234\r\n");
    delay(2000);                                            // This delay is required.
    blueToothSerial.print("\r\n+INQ=1\r\n");                // make the slave bluetooth inquirable
    Serial.println("The slave bluetooth is inquirable!");
    delay(2000);                                            // This delay is required.
    blueToothSerial.flush();
}
