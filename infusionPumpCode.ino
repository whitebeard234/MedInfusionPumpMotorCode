#include <LiquidCrystal.h>
#include <Keypad.h>
#include <TimedAction.h>

const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns
byte data_count = 0;
char Data[16];
char volume[16];
char inputTime[16];
bool done = 0;
int states = 0;
int volumelength = 0;
int timelength = 0;
int i;
float t, v;
float stepCM = 0.0003125;
int y = 0;
float ccLength = 0;
//int cycles = 0;
float del = 0;
char syringeType;
int steps = 0;
int aState;
int aLastState;

int S = 60; // count seconds
int M = 0; // count seconds
int H = 0; // count seconds

//Stepper Motor Config
int stepPin = 22;
int dirPin = 24;
int enblPin = 26;

//Rotary Encoder
int outputA = 44;
int outputB = 46;

// Define the Keymap for Keypad
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'.', '0', '#', 'D'}
};
// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte rowPins[ROWS] = {42, 40, 38, 36};
// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte colPins[COLS] = {34, 32, 30, 28};

//  Create the Keypad
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
LiquidCrystal lcd(29, 33, 43, 45, 47, 49);//RS,EN,D4,D5,D6,D7

void setup() {

  pinMode (stepPin, OUTPUT);
  pinMode (dirPin, OUTPUT);
  pinMode (enblPin, OUTPUT);
  digitalWrite(stepPin, LOW);
  digitalWrite(dirPin, LOW);
  digitalWrite(enblPin, HIGH);

  pinMode (outputA,INPUT);
  pinMode (outputB,INPUT);

  Serial.begin (9600);
   // Reads the initial state of the outputA
   aLastState = digitalRead(outputA);

  lcd.begin(16, 2);
  lcd.print("     Welcome    ");
  lcd.setCursor(0, 1);
  lcd.print("InfusionPumpv0.1");
  delay(3000);
  lcd.clear();
}

void loop() {
  InputData();
}

void nextstate() {
  states++;
  lcd.clear();
}

void clearData()
{
  memset(Data, 0, sizeof(Data));
  //  data_count = 0;
}

void runStepper() {
  int interval = 1;
  TimedAction countdownThread = TimedAction(1000, countdown);
  TimedAction progressThread = TimedAction(0, progress);
  char customKey = kpd.getKey();

//Steps per 100ms
  del = (t * 600);
  y = ((v * ccLength) / (stepCM * del));

  for (int x = 0; x < del; x++){
    for (int x = 0; x < y; x++){
      digitalWrite(stepPin, HIGH);
      delay(interval);
      digitalWrite(stepPin, LOW);
      delay(interval);  
      progressThread.check();
    }
    delay(100-(2 * interval * y));    
    countdownThread.check();
  }

  digitalWrite(enblPin, LOW);

  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Resetting motor_");
  
  digitalWrite(enblPin, HIGH);
  digitalWrite(dirPin, HIGH);

  int z = y * del;

  for (int x = 0; x < z; x++) {
    digitalWrite(stepPin, HIGH);
    delay(interval);
    digitalWrite(stepPin, LOW);
    delay(interval);
  }
  digitalWrite(enblPin, LOW);
  lcd.setCursor(0, 1);
  lcd.print("   Done Reset   ");
  done = 1;
  exit(0);

  if (customKey == '#' && done == 1) {
    done = 0;
    clearData();
    data_count = 0;
    states = 0;
    t = 0;
    v = 0;
  }

}

void progress() {
  aState = digitalRead(outputA); // Reads the "current" state of the outputA
   // If the previous and the current state of the outputA are different, that means a Pulse has occured
  if (aState != aLastState){     
     // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
    if (digitalRead(outputB) != aState) { 
      steps --;
    } else {
      steps ++;
    }
    float sBase = (v * ccLength) / 0.003125;
    int prog = (steps / sBase) * 100;
    
    lcd.setCursor(11, 0);
    lcd.print(prog);
    lcd.print(" %");
  } 
  aLastState = aState; // Updates the previous state of the outputA with the current state
}

void countdown() {
  lcd.setCursor(0, 0);
  lcd.print("Progress: ");
  lcd.setCursor(6, 1);
  lcd.print(":");
  lcd.setCursor(9, 1);
  lcd.print(":");

  S--;

  if (S < 0)
  {
    M--;
    S = 59;
  }
  if (M < 0)
  {
    H--;
    M = 59;
  }
  if (H < 0)
  {
    H = 23;
    M = 59;
    S = 59;
  }
  if (M > 9)
  {
    lcd.setCursor(7, 1);
    lcd.print(M);
  }
  else
  {
    lcd.setCursor(7, 1);
    lcd.print("0");
    lcd.setCursor(8, 1);
    lcd.print(M);
    lcd.setCursor(9, 1);
    lcd.print(":");
  }

  if (S > 9)
  {
    lcd.setCursor(10, 1);
    lcd.print(S);
  }
  else
  {
    lcd.setCursor(10, 1);
    lcd.print("0");
    lcd.setCursor(11, 1);
    lcd.print(S);
    lcd.setCursor(12, 1);
    lcd.print(" ");
  }

  if (H > 9)
  {
    lcd.setCursor(4, 1);
    lcd.print (H);
  }
  else
  {
    lcd.setCursor(4, 1);
    lcd.print("0");
    lcd.setCursor(5, 1);
    lcd.print(H);
    lcd.setCursor(6, 1);
    lcd.print(":");
  }
}

void InputData() {
  char customKey = kpd.getKey();
  switch (states) {

    case 0:
      lcd.setCursor(0, 0);
      lcd.print("Injection Type:");
      lcd.setCursor(0, 1);
      lcd.print("A = 3cc  B = 5cc");

      if (customKey == 'A') {
        ccLength = 4.725 / 3;
        syringeType = 'A';
        nextstate();
      }

      if (customKey == 'B') {
        ccLength = 4.6 / 6;
        //ccLength = 3.8 / 5;
        syringeType = 'B';
        nextstate();
      }

      break;

    case 1:
      lcd.setCursor(0, 0);
      lcd.print("Input Vol(cc):");

      if (customKey != NO_KEY && customKey != '#'
          && customKey != 'A' && customKey != 'B' && customKey != 'C'
          && customKey != 'D') {
        Data[data_count] = customKey;
        lcd.setCursor(data_count, 1);
        lcd.print(Data[data_count]);
        data_count++;
      }

      if (customKey == '#') {
        volumelength = data_count;

        for (i = 0; i < volumelength; i++) {
          volume[i] = Data[i];
        }
        clearData();
        data_count = 0;
        v = (float)atof(volume);
        if (syringeType == 'A' && v > 3) {
            lcd.setCursor(0, 1);
            lcd.print(" !Error Input! ");
            delay(1000);
            lcd.clear();
            clearData();
            data_count = 0;
            states = 1;
        }
        else if (syringeType == 'B' && v > 6) {
            lcd.setCursor(0, 1);
            lcd.print(" !Error Input! ");
            delay(1000);
            lcd.clear();
            clearData();
            data_count = 0;
            states = 1;
        }
        else {
          nextstate();
        }
      }

      if (customKey == 'D') {
        lcd.clear();
        clearData();
        data_count = 0;

      }
      break;

    case 2:
      lcd.setCursor(0, 0);
      lcd.print("Input Time(min):");

      if (customKey != NO_KEY && customKey != '#' && customKey != 'A' && customKey != 'B' && customKey != 'C'
          && customKey != 'D') {
        Data[data_count] = customKey;
        lcd.setCursor(data_count, 1);
        lcd.print(Data[data_count]);
        data_count++;

      }

      if (customKey == '#') {
        timelength = data_count;

        for (i = 0; i < timelength; i++) {
          inputTime[i] = Data[i];
        }
        clearData();
        nextstate();
        data_count = 0;
        t = (float)atof(inputTime);

        int input = t;
        M = (input % 60) - 1;
        H = input / 60;
      }

      if (customKey == 'D') {
        lcd.clear();
        clearData();
        data_count = 0;
      }

      break;

    case 3:
      lcd.setCursor(0, 0);
      lcd.print("Volume: ");
      for (i = 0; i < volumelength; i++) {
        lcd.print(volume[i]);
      }
      lcd.print(" cc ?");
      
      lcd.setCursor(0, 1);
      lcd.print("Time: ");
      for (i = 0; i < timelength; i++) {
        lcd.print(inputTime[i]);
      }
      lcd.print(" mins ?");

    if (customKey == '#') {
      if (v != 0 && t != 0){
        delay(1000);
        nextstate();
      }
      if (v == 0 || t == 0) {
        lcd.clear();
        states = 1;
      }
    }
    if (customKey == 'D') {
        clearData();
        data_count = 0;
        states = 0;
        t = 0;
        v = 0;
      }
      break;

    case 4:
      runStepper();
      break;

  }

}
