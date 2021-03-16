const int relayPin = 2;
const int uPin = 3;
const int standByBtn = 4;
const int standByLed = 8;
const int standByCountPin = 7;
const int iLDR = A3;
const int iTMP = A2;
const int oLDR = A0;
const int oTMP = A1;

// motors
const int coolingPWM = 10;
const int coolingin3 = 12;
const int coolingin4 = 13;

const int processPWM = 11;
const int processin3 = 0;
const int processin4 = 1;

//PWM LED PINS 
const int LEDSET1 = 5;
const int LEDSET2 = 6;
const int LEDSET3 = 9;

const unsigned long scriptStartTime = 0;

bool humanPresence = false;
bool standByState = false;
int standByHours = 1;

int oldStandByState = 0;
int newStandByState;

void setup() {
  pinMode(relayPin, OUTPUT);
  pinMode(standByBtn, INPUT);
  pinMode(standByLed, OUTPUT);
  pinMode(standByCountPin, INPUT);
  pinMode(iLDR, INPUT);
  pinMode(iTMP, INPUT);
  pinMode(oLDR, INPUT);
  pinMode(oTMP, INPUT);

  pinMode(LEDSET1, OUTPUT);
  pinMode(LEDSET2, OUTPUT);
  pinMode(LEDSET3, OUTPUT);

  pinMode(coolingin3, OUTPUT);
  pinMode(coolingin4, OUTPUT);
  pinMode(coolingPWM, OUTPUT);

  pinMode(processin3, OUTPUT);
  pinMode(processin4, OUTPUT);
  pinMode(processPWM, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  float uPulseIn = readUltrasonicPing();
  float UDistance = pulseToDistance(uPulseIn);

  int presenceDistance = 20;

  if (UDistance <= presenceDistance) {
    if (humanPresence == true) {
      humanPresence = false;
      Serial.println("Human has exited");
      delay(6000);
    } else {
      Serial.println("Human has entered");
      humanPresence = true;
      delay(6000);
    }
  }

  if (humanPresence == true) {
    digitalWrite(relayPin, LOW); //ON
    checkStandByMode();
    adjustLighting();
    adjustCooling();
    runProccessingUnit();

    if ((millis() - scriptStartTime) % 100 == 0) {
      outputWeatherInfo();
    }
  } else {
    analogWrite(LEDSET1, 0);
    analogWrite(LEDSET2, 0);
    analogWrite(LEDSET3, 0);
    if (standByState == true) {
      digitalWrite(relayPin, LOW); //ON
      // set processing unit on for specified time
      Serial.println("Running standby");
      for (int i = 0; i <= standByHours; i++) {
        runProccessingUnit();
        delay(1000);
      }
      Serial.println("StandBY period Over");
      digitalWrite(standByLed, LOW);
      digitalWrite(relayPin, HIGH); //OFF
      standByState = false;
      standByHours = 1;
    } else {
      digitalWrite(relayPin, HIGH); //OFF
    }
  }
}

float readUltrasonicPing() {
  pinMode(uPin, OUTPUT); // Clear the trigger
  digitalWrite(uPin, LOW);
  delayMicroseconds(2);
  digitalWrite(uPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(uPin, LOW);
  pinMode(uPin, INPUT);
  return pulseIn(uPin, HIGH);
}

float pulseToDistance(float pulse) {
  return 0.01723 * pulse;
}

void checkStandByMode() {
  newStandByState = digitalRead(standByBtn);
  if (oldStandByState == 0 && newStandByState == 1) {
    if (standByState == false) {
      digitalWrite(standByLed, HIGH);
      standByState = true;
      Serial.println("Switched ON StandBY Mode.");
    } else {
      digitalWrite(standByLed, LOW);
      standByState = false;
      Serial.println("Switched OFF StandBY Mode.");
    }
  }
  oldStandByState = newStandByState;

  int countState = digitalRead(standByCountPin);
  if (countState == HIGH) {
    standByHours++;
    if (standByHours >= 11) {
      standByHours = 1;
    }
    Serial.print("Set STANDBY for: ");
    Serial.print(standByHours);
    Serial.print(" hour(s)\n");
    delay(300);
  }
}

void outputWeatherInfo() {
  // LIGHT
  int lightIntensity = analogRead(oLDR);
  if (lightIntensity >= 800) {
    Serial.println("it's Bright Outside. Allow Sunlight to minimise energy usage");

  } else if (lightIntensity >= 500) {
    Serial.println("Outdoor Light is fading. Maximise efficiency by using sunlight 🌞 before it gets dark");
  } else {
    Serial.println("Outdoor Lighting has faded away. Maximise sunlight usage during the day");
  }
  Serial.print("Intensity: ");
  Serial.print(lightIntensity);
  Serial.print("\nLux: ");
  Serial.print(int(LightIntensity(lightIntensity)));
  Serial.print("\n\n");

  //TEMPERATURE
  int reading = analogRead(oTMP);
  int temp = temperatureC(reading);
  if (temp > 18 && temp < 28) {
    Serial.println("Optimal Temperature Outside. Allow Airflow");
  }
  Serial.print("Temperature Outside: ");
  Serial.print(temp);
  Serial.println("C \n\n");
}

double LightIntensity(int RawADC0) {
  double Vout = RawADC0 * 0.0048828125;
  int lux = (2500 / Vout - 500) / 10;
  return lux;
}

int temperatureC(int reading) {
  int value = (reading - 20) * 3.04;
  int celsius = map(value, 0, 1023, -40, 125);
  return celsius;
}

void adjustLighting() {
  int lightIntensity = analogRead(iLDR);
  int mapped = map(lightIntensity, 54, 974, 0, 255);
  mapped = 255 - mapped;
  analogWrite(LEDSET1, mapped);
  analogWrite(LEDSET2, mapped);
  analogWrite(LEDSET3, mapped);
}

void adjustCooling() {
  int reading = analogRead(iTMP);
  int temp = temperatureC(reading);
  int motorVal = map(temp, -40, 125, 0, 255);
  digitalWrite(coolingin3, LOW);
  digitalWrite(coolingin4, HIGH);

  analogWrite(coolingPWM, motorVal);
}
void runProccessingUnit() {
  digitalWrite(processin3, LOW);
  digitalWrite(processin4, HIGH);

  analogWrite(processPWM, 150);
  delay(100);
}
