#include <Servo.h>
#include <LiquidCrystal.h>

Servo servo;

int const servopin = 2;
int const potpin = 0;
int const buttonpin = 3;
int const batterypin = 1;
int const beeperpin = 8;
int const hallpin = 2;
int potpinval;
int batpinval;
int hallpinval;
int batlevel;
bool batcheck = true;
bool running = true;
bool halldetect = false;
long time = micros();
long prevtime = micros();
long timediv;
double rps = 0;

const int rs = 12, en = 11, d4 = 7, d5 = 6, d6 = 5, d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  // the setup of the code
  Serial.begin(9600);
  servo.attach(servopin);
  servo.write(90);

  lcd.begin(16, 2);

  lcd.print("Speed:");

  lcd.setCursor(0, 1);
  lcd.print("Battery:");

  if(analogRead(hallpin)>300){
    halldetect = true;
  }
}

void loop() {
  //constant loop through this code
  readInputs();
  controlSpeed();
  manageBattery();
  checkHallSensor();
  printLCD();
}

void beep(long time, int amount) {
  //beeps to let the user know it's battery is almost dead.
  batcheck = !batcheck;
  for (int i=0 ; i<amount ; i++)
  {
    digitalWrite(beeperpin, HIGH);
    delay(time);
    digitalWrite(beeperpin, LOW);
    delay(time);
  }
}

void detect() {
  //calculate the rps of the motor. (in the future it will calculate the speed in kph)
  halldetect = !halldetect;
  time = micros();

  timediv = (time - prevtime)*14;
  prevtime = time;

  rps = 1000000/timediv;

}

void readInputs() {
  //read all necessary inputs
  potpinval = analogRead(potpin);       //reading value of the potentiometer for speed control of bldc motor
  batpinval = analogRead(batterypin);   //reading value of the battery for charge estimation
  hallpinval = analogRead(hallpin);     //reading value of the hall sensor in bldc motor for speed estimation

  batlevel = map(batpinval,0,1023, 100, 0);   //mapping the battery pin level to a 0% to 100% scale
}

void controlSpeed() {
  //only when a button is pressed and when running is true, may the user control the speed of the bldc motor. else it's always 0 speed
    if (digitalRead(buttonpin) && running) {
        servo.write(map(potpinval, 0, 1023, 0, 180));
    } else {
        servo.write(90);
    }
}

void manageBattery() {
  //a switch case for the battery level to let the user know he's battery is dying.
	switch (batlevel)
	{
    case 10:
      if (!batcheck)
      {
        beep(500, 3);
        running = false;
      }
      break;
    case 15:
      if (batcheck)
      {
        beep(200, 2);
      }
      break;
    default:
      if (batlevel > 20)
      {
        batcheck = true;
        running = true;
      }
      break;
	}
}

void checkHallSensor() {
  //when the hallsensor detects a difference in value, the speed of the motor will be estimated
  if ((!halldetect && analogRead(hallpin) > 500) || (halldetect && analogRead(hallpin) < 100)) {
    detect();
  }
}

void printLCD() {
  //show necessary info on the lcd screen for the user
	lcd.setCursor(6, 0);
	lcd.print("   ");
	lcd.setCursor(6, 0);
	lcd.print(map(potpinval, 0, 1023, 10, -10));
	lcd.print("|       ");
	lcd.setCursor(9, 0);
	lcd.print((int)floor(rps));

	lcd.setCursor(8, 1);
	lcd.print("   ");
	lcd.setCursor(8, 1);
	lcd.print(batlevel);
}