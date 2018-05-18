const byte ledPin = 13;
const byte interruptbuttonPin1 = 48;
const byte interruptbuttonPin2 = 50;
const byte interruptbuttonPin3 = 52;
volatile byte state = LOW;
void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(interruptbuttonPin1, INPUT_PULLUP);
  pinMode(interruptbuttonPin2, INPUT_PULLUP);
  pinMode(interruptbuttonPin3, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptbuttonPin1), blink, CHANGE);
  attachInterrupt(digitalPinToInterrupt(interruptbuttonPin2), blink, CHANGE);
  attachInterrupt(digitalPinToInterrupt(interruptbuttonPin3), blink, CHANGE);
  Serial.begin(9600);
}
void loop() {
  digitalWrite(ledPin, state);
  byte test = digitalRead(interruptbuttonPin1);
  Serial.println(test);
}
void blink() {
  state = !state;
}

