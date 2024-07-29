#include <Servo.h>
#include <LiquidCrystal.h>

Servo servo;

const int rs = 12, en = 11, d4 = 7, d5 = 6, d6 = 5, d7 = 4, beeperpin = 8, batterypin = 0, buttonpin = 3, potpin = 6, escpin = 2;
const double inputVoltage = 5.1;
int potpinval, batpinval, batlevel = 100;
bool batcheck = true, running = true;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int voltageValues[500];
int batteryValues[300];

void setup()
{
	// the setup of the code
	servo.attach(escpin, 1000, 2000);
	servo.write(90);

	lcd.begin(16, 2);

	lcd.print("Speed:");

	lcd.setCursor(0, 1);
	lcd.print("Battery:");

}

void loop()
{
	// constant loop through this code
	arrayshift();
	readInputs();
	controlSpeed();
	manageBattery();
	printLCD();
}

void shiftArray(int *array, int size, int newValue)
{
	for (int i = size - 1; i > 0; i--)
	{
		array[i] = array[i - 1];
	}
	array[0] = newValue;
}

void arrayshift()
{
	int voltageValuesSize = sizeof(voltageValues) / sizeof(int);
	int batteryValuesSize = sizeof(batteryValues) / sizeof(int);

	shiftArray(voltageValues, voltageValuesSize, analogRead(batpinval));
	shiftArray(batteryValues, batteryValuesSize, batlevel);
}

double calculateMean(int array[], int size)
{
	long sum = 0;
	for (int i = 0; i < size; i++)
	{
		sum += array[i];
	}
	return (double)floor(sum / size);
}

bool checkCharge(double num)
{
	for (int num : voltageValues)
	{
		if (num == 0)
		{
			return false;
		}
	}
	return true;
}

void calculateCharge(double num)
{

	double voltage = (num / 1024) * inputVoltage;
	int charge;

	if (voltage > 3.7)
	{ // 6S battery pack is connected (24V)
		charge = formulaCharge(voltage, -172.2, 1760, 4397.8);  //values are calculated in "battery formulas.xlsx"
	}
	else
	{ // 3S battery pack is connected (11V)
		charge = formulaCharge(voltage, -689.24, 3522.4, 4400.9); //values are calculated in "battery formulas.xlsx"
	}

	if (charge > 0)
	{
		batlevel = charge;
	}
}

int formulaCharge(double voltage, double a, double b, double c)
{
	return (int)floor((a * (voltage * voltage)) + (b * voltage) - c);
}

void beep(long time, int amount)
{
	// beeps to let the user know it's battery is almost dead.
	batcheck = !batcheck;
	for (int i = 0; i < amount; i++)
	{
		digitalWrite(beeperpin, HIGH);
		delay(time);
		digitalWrite(beeperpin, LOW);
		delay(time);
	}
}

void readInputs()
{
	// read all necessary inputs
	potpinval = analogRead(potpin);		// reading value of the potentiometer for speed control of bldc motor
	batpinval = analogRead(batterypin); // reading value of the battery for charge estimation
}

void controlSpeed()
{
	// only when a button is pressed and when running is true, may the user control the speed of the bldc motor. else it's always 0 speed
	if (digitalRead(buttonpin) && running)
	{
		servo.write(map(potpinval, 10, 1000, 180, 0));
	}
	else
	{
		servo.write(90);
	}
}

void manageBattery()
{
	// a switch case for the battery level to let the user know he's battery is dying.
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

void printLCD()
{
	// show necessary info on the lcd screen for the user
	lcd.setCursor(6, 0);
	lcd.print("   ");
	lcd.setCursor(6, 0);
	lcd.print(map(potpinval, 10, 1000, -10, 10));

	lcd.setCursor(8, 1);
	lcd.print("      ");
	lcd.setCursor(8, 1);
  float mean = calculateMean(voltageValues, sizeof(voltageValues) / sizeof(int));
	if (checkCharge(mean))
	{
		calculateCharge(mean);
	}
	lcd.print((int)floor(calculateMean(batteryValues, sizeof(batteryValues) / sizeof(int))));
}