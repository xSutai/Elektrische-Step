#include <Servo.h>
#include <LiquidCrystal.h>

Servo servo;

const int rs = 12, en = 11, d4 = 7, d5 = 6, d6 = 5, d7 = 4, beeperpin = 8, batterypin = 0, buttonpin = 3, potpin = 7, escpin = 2;
//hallpin = 9;
const double inputVoltage = 5.09;
int potpinval, batpinval, batlevel = 100;
bool batcheck = true, running = true;
//halldetect = false;
//long time = micros(), prevtime = micros(), timediv;
//double rps = 0;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int voltageValues[500];
int batteryValues[300];

void setup() {
  // the setup of the code
  //Serial.begin(9600);
  servo.attach(escpin, 1000, 2000);
  servo.write(90);

  lcd.begin(16, 2);

  lcd.print("Speed:");

  lcd.setCursor(0, 1);
  lcd.print("Battery:");

  // if (digitalRead(hallpin)) {
  //   halldetect = true;
  // }
}

void loop() {
  //constant loop through this code
  arrayshift();
  readInputs();
  controlSpeed();
  manageBattery();
  //checkHallSensor();
  printLCD();
}

void arrayshift() {
  for (int i = 0; i < (sizeof(voltageValues) / sizeof(int)); i += 1) {
    int j = ((sizeof(voltageValues) / sizeof(int)) - 1) - i;
    if (j == 0) {
      voltageValues[j] = analogRead(batpinval);
    } else {
      voltageValues[j] = voltageValues[j - 1];
    }
  }

  for (int i = 0; i < (sizeof(batteryValues) / sizeof(int)); i += 1) {
    int j = ((sizeof(batteryValues) / sizeof(int)) - 1) - i;
    if (j == 0) {
      batteryValues[j] = batlevel;
    } else {
      batteryValues[j] = batteryValues[j - 1];
    }
  }
}

double calculateMean(int array[], int size) {
  long sum = 0;
  for (int i=0; i<size; i++) {
    sum += array[i];
  }
  return (double)floor(sum / size);
}

int calculateCharge(double num) {
  double voltage = (num / 1024) * inputVoltage;
  int charge;
  bool check = true;

  for (int num : voltageValues) {
    if (num == 0) {
      check = false;
    }
  }
  if (check) {
    if (voltage > 3.7) {  //6S battery pack is connected (24V)
      charge = (int)floor((-169.57 * (voltage * voltage)) + (1746.5 * voltage) - 4397.8);
    } else {  //3S battery pack is connected (11V)
      charge = (int)floor((-678.73 * (voltage * voltage)) + (3495.4 * voltage) - 4400.9);
    }

    if (charge > 0) {
      batlevel = charge;
    }
    return batlevel;
  } else {
    return batlevel;
  }
}

void beep(long time, int amount) {
  //beeps to let the user know it's battery is almost dead.
  batcheck = !batcheck;
  for (int i = 0; i < amount; i++) {
    digitalWrite(beeperpin, HIGH);
    delay(time);
    digitalWrite(beeperpin, LOW);
    delay(time);
  }
}

// void detect() {
//   //calculate the rps of the motor. (in the future it will calculate the speed in kph)
//   halldetect = !halldetect;
//   time = micros();

//   timediv = (time - prevtime) * 14;
//   prevtime = time;

//   rps = 1000000 / timediv;
// }

void readInputs() {
  //read all necessary inputs
  potpinval = analogRead(potpin);      //reading value of the potentiometer for speed control of bldc motor
  batpinval = analogRead(batterypin);  //reading value of the battery for charge estimation
  //hallpinval = analogRead(hallpin);    //reading value of the hall sensor in bldc motor for speed estimation
}

void controlSpeed() {
  //only when a button is pressed and when running is true, may the user control the speed of the bldc motor. else it's always 0 speed
  if (digitalRead(buttonpin) && running) {
    servo.write(map(potpinval, 0, 1023, 180, 0));
  } else {
    servo.write(90);
  }
}

void manageBattery() {
  //a switch case for the battery level to let the user know he's battery is dying.
  switch (batlevel) {
    case 10:
      if (!batcheck) {
        beep(500, 3);
        running = false;
      }
      break;
    case 15:
      if (batcheck) {
        beep(200, 2);
      }
      break;
    default:
      if (batlevel > 20) {
        batcheck = true;
        running = true;
      }
      break;
  }
}

// void checkHallSensor() {
//   //when the hallsensor detects a difference in value, the speed of the motor will be estimated
//   if ((!halldetect && digitalRead(hallpin)) || (halldetect && (!digitalRead(hallpin)))) {
//     detect();
//   }
// }

void printLCD() {
  //show necessary info on the lcd screen for the user
  lcd.setCursor(6, 0);
  lcd.print("   ");
  lcd.setCursor(6, 0);
  lcd.print(map(potpinval, 0, 1023, 10, -10));
  // lcd.setCursor(9, 0);
  // lcd.print("|       ");
  // lcd.setCursor(10, 0);
  // lcd.print((int)floor(rps));

  lcd.setCursor(8, 1);
  lcd.print("      ");
  lcd.setCursor(8, 1);
  calculateCharge(calculateMean(voltageValues, sizeof(voltageValues) / sizeof(int)));
  lcd.print((int)floor(calculateMean(batteryValues, sizeof(batteryValues) / sizeof(int))));
}