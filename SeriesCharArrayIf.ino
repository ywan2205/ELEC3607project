const byte numChars = 32;
char receivedChars[numChars];

boolean newData = false;

void setup() {
    Serial.begin(9600);
    Serial.println("<Arduino is ready>");
}

void loop() {
    recvWithStartEndMarkers();
    showNewData();
}

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
 // if (Serial.available() > 0) {
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

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
        Serial.println(receivedChars);
        if(strcmp(receivedChars, "returnState") == 0)
          Serial.println("now there are three space remaining");
        else if (strcmp(receivedChars, "bookPlace") == 0)
          Serial.println("Received your order, your booker place is place 4.");
         else if (strcmp(receivedChars, "payBill") == 0)
          Serial.println("Your payment has been confirmed, thanks for your using");
          else
          Serial.println("unknown order");
        newData = false;
    }
}

