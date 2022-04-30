/*  Door Sensor of Secret Knocks

    Detects patterns of knocks and triggers a motor or something if the pattern is correct.

    By: FeoN_
    Version: 0.5
    Licensed: Creative Commons Attribution-Noncommercial-Share Alike 3.0
    http://creativecommons.org/licenses/by-nc-sa/3.0/us/

    Source: https://www.instructables.com/Secret-Knock-Detecting-Door-Lock/
    By Steve Hoefer http://grathio.com
*/



//===---   Global Vars   ---===//

//==--  Pins  --==//
#define KnockSensorPin A0     // Sensor of vibration or Piezo sensor
#define ProgramButtonPin 2    // Programing Button to change a knock pattern
#define LockMotorPin 5        // Gear motor or other
#define RLEDPin 3             // Red Status LED
#define GLEDPin 4             // Green Status LED

//==--  Const  --==//
#define Timer 50                      // default: 500
#define SensorThreshold 500           // default: 500
#define ProgramingButtonDelay 800     // default: 800
#define MinRejectDeviation 40         // default: 40
#define MaxRejectDeviation 40         // default: 40
#define LockUnlockingDelay 700        // default: 700

#define MaxKnocks 20                // default: 20
#define KnockCompletedDelay 2000    // default: 2000

//==--  Vars  --==//
uint16_t SecretCode[MaxKnocks];
uint16_t ReadingKnocks[MaxKnocks];
uint16_t ProgramingButtonLastTime = millis();
bool ProgramingModeFlag = false;

//===---   ---===---   ---===//



//===---   MAIN SETUP   ---===//

void setup() {
  //==--  INPUTs  --==//
  pinMode(ProgramButtonPin, INPUT_PULLUP);
  pinMode(KnockSensorPin, INPUT);
  
  //==--  OUTPUTs  --==//
  pinMode(LockMotorPin , OUTPUT);
  pinMode(RLEDPin, OUTPUT);
  pinMode(GLEDPin, OUTPUT);

  //==--  Begins  --==//
  Serial.begin(9600);

  //==--  Interrupts  --==//
  attachInterrupt(digitalPinToInterrupt(ProgramButtonPin), switch_programing_mode, CHANGE);

  //==--  Startup  --==//
  Serial.println("Program started!");
}

//===---   ---===---   ---===//



//===---   MAIN LOOP   ---===//

void loop() {
  if (millis() % Timer == 0) {
    if (analogRead(KnockSensorPin) >= SensorThreshold) { listen_knocks(); }
  }
}

//===---   ---===---   ---===//



//===---   Global Funcs   ---===//
void listen_knocks() {

  //==-- Local Setup  --==//
  noInterrupts();

  Serial.println("Listening started...");
  blink_LED(GLEDPin, 1, 1, 150, 0);

  for (uint8_t i = 0; i < MaxKnocks; i++){ ReadingKnocks[i] = 0; }

  //==-- Local Vars  --==//
  uint8_t current_knock = 0;
  uint16_t start_time=millis();
  uint16_t now;

  //==-- Content  --==//
  // Listening
  do {
    if (analogRead(KnockSensorPin) >= SensorThreshold) {
      now = millis();
      ReadingKnocks[current_knock++] = now - start_time;
      start_time=now;
    }

  } while ((millis() - start_time < KnockCompletedDelay) && (current_knock < MaxKnocks));

  Serial.println("Listening ended.");
  blink_LED(GLEDPin, 1, 1, 150, 0);

  // Setting new pattern | Validating
  if (ProgramingModeFlag) { set_new_knocks_pattern(); }
  else { validate_knocks(); }

  interrupts();
}


bool validate_knocks() {
  //==-- Local Vars  --==//

  //==-- Content  --==//
  Serial.println("Validating...");

  for (uint8_t i = 0; i < MaxKnocks; i++) {
    if (SecretCode[i] - MinRejectDeviation < ReadingKnocks[i] && ReadingKnocks[i] < SecretCode[i] - MaxRejectDeviation) {

      Serial.println("Validation failed!");

      blink_LED(GLEDPin, 4, 1, 100, 10);

      return false;
    }
  }

  Serial.println("Validation passed!");
  blink_LED(GLEDPin, 2, 1, 200, 100);

  unlock_door();
  return true;
}

void set_new_knocks_pattern() {
  //==-- Local Vars  --==//

  //==-- Content  --==//
  Serial.println("Setting new pattern...");
  for (uint8_t i = 0; i < MaxKnocks; i++) { SecretCode[i] = ReadingKnocks[i]; }

  Serial.println("New pattern setted!");
  blink_LED(RLEDPin, 2, 1, 500, 50);
}


void unlock_door() {
  //==-- Local Vars  --==//

  //==-- Content  --==//
  Serial.println("Opening...");
  digitalWrite(GLEDPin, HIGH);

  // Motor work
  digitalWrite(LockMotorPin, HIGH);
  delay(LockUnlockingDelay);
  digitalWrite(LockMotorPin, LOW);

  Serial.println("Opened!");
  digitalWrite(GLEDPin, LOW);
}


void switch_programing_mode() {
  //==-- Local Vars  --==//

  //==-- Content  --==//
  if (millis() > ProgramingButtonLastTime + ProgramingButtonDelay) {
    ProgramingModeFlag = !ProgramingModeFlag;
    digitalWrite(RLEDPin, ProgramingModeFlag);

    ProgramingButtonLastTime = millis();
    Serial.print("Programing Mode:"); Serial.println(ProgramingModeFlag);
  }
}


void blink_LED(uint8_t led_pin, uint8_t amount, uint8_t interval, uint16_t knock_delay, uint16_t ending_delay) {
  //==-- Local Vars  --==//

  //==-- Content  --==//
  for (uint8_t a = 0; a < amount; a+interval) {
    digitalWrite(led_pin, HIGH);

    delay(knock_delay);

    digitalWrite(led_pin, LOW);
    delay(ending_delay);
  }
}

//===---   ---===---   ---===//

//######//
