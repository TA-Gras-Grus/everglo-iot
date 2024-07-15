int floatSensor1 = D4;
int floatSensor2 = D3;
int buttonState1 = 1;  //reads pushbutton status
int buttonState2 = 1;  //reads pushbutton status
int relayDosing1 = D2;
int relayDosing2 = D1;
int relayPump = D0;
int waterStatus = 1;

void setup() {
  Serial.begin(115200);
  pinMode(floatSensor1, INPUT_PULLUP);
  pinMode(floatSensor2, INPUT_PULLUP);
  pinMode(relayPump, OUTPUT);
  pinMode(relayDosing1, OUTPUT);
  pinMode(relayDosing2, OUTPUT);

  digitalWrite(relayPump, LOW);
  digitalWrite(relayDosing1, LOW);
  digitalWrite(relayDosing2, LOW);
}

void loop() {
  buttonState1 = digitalRead(floatSensor1);
  buttonState2 = digitalRead(floatSensor2);

  if (buttonState1 == LOW && buttonState2 == LOW) {
    waterStatus = 1;
    startPump();
  }

  if (buttonState1 == HIGH && buttonState2 == HIGH) {
    stopPump();
  }

  delay(500);
}

void startPump() {
  digitalWrite(relayPump, LOW);
}

void stopPump() {
  digitalWrite(relayPump, HIGH);
  if (waterStatus == 1) {
    startDosing();
    delay(5000);
    stopDosing();
    waterStatus = 0;
  }
}

void startDosing() {
  digitalWrite(relayDosing1, LOW);
  digitalWrite(relayDosing2, LOW);
}

void stopDosing() {
  digitalWrite(relayDosing1, HIGH);
  digitalWrite(relayDosing2, HIGH);
}