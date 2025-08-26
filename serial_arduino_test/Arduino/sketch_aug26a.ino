const int ledPin = 13;
const int sensorPin = A0;

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);

  randomSeed(analogRead(0)); // seed random generator
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.equalsIgnoreCase("GET_TEMP")) {
      float temp = random(200, 350) / 10.0;  // 20.0 to 34.9
      Serial.print("TEMP:");
      Serial.println(temp);
    }
    else if (command.equalsIgnoreCase("GET_LED_STS")) {
      int ledStatus = digitalRead(ledPin);
      Serial.print("LED:");
      Serial.println(ledStatus ? "ON" : "OFF"); //LED:OFF\r\n
    }
    else if (command.equalsIgnoreCase("GET_SENSOR_1")) {
      int sensorValue = random(400, 600);  // fake sensor value
      Serial.print("SENSOR1:");
      Serial.println(sensorValue);
    }
    else if (command.startsWith("SET_LED")) {
      if (command.endsWith("ON")) {
        digitalWrite(ledPin, HIGH);
        Serial.println("LED ON");
      } else {
        digitalWrite(ledPin, LOW);
        Serial.println("LED OFF");
      }
    }
    else {
      Serial.println("UNKNOWN_COMMAND");
    }
  }
}
