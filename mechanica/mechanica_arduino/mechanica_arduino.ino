/*

 Name:		mechanica_arduino.ino
 Created:	3/1/2018 1:13:50 PM
 Author:	Brian Amin
*/

// the setup function runs once when you press reset or power the board
#include <MegaMotoHB.h>
#include <Encoder.h>


// Setup Pins
int chip_select = 4;
int pinA = 2;
int pinB = 3;
int loadcell_pin = 0;
int motor_enablepin = 8;
int motor_pwm_pin = 11;
int motor_pwm_pin2 = 5;
//MegaMotoHB motor(motor_pwm_pin, motor_pwm_pin2, motor_enablepin);


//Magnetic Sensor
Encoder mag_sensor(2, 3);


// Load cell
//float analog_val = 0;
float analog_int = 0;





bool control_bool = true;
bool retract_bool = false;

long time = 0;
int interval = 500; //ms
String incoming_string = "";
double desired_position = 00.00;
String command = "0";
boolean newData = false;
boolean newData_desired = false;
const byte numChars = 32;
const byte numChars_desired = 32;
char recievedChars[numChars];
char recievedDesiredChars[numChars_desired];

double update_position()
{
	long newPos;
	newPos = mag_sensor.read();
	double pos = (newPos / 1024.0) * (2.0);

	return pos;
}

float read_loadcell()
{
	float analog_val = analogRead(0);

	//analog_val = map(analog_val, analog_int, 2000, 0, 1000);

	return analog_val;
}


void zero_loadcell()
{
  analog_int = analogRead(0);
  delay(10);
  analog_int = analogRead(0);

}

void recieve_commands()
{
	static boolean recvInProgress = false;
	static byte ndx = 0;
	char rc;
	char endMarker = '>';
	char startMaker = '<';
	while (Serial.available() > 0 && newData == false)
	{
		rc = Serial.read();
		if (recvInProgress == true)
		{
			if (rc != endMarker)
			{
				recievedChars[ndx] = rc;
				ndx++;
				if (ndx >= numChars)
				{
					ndx = numChars - 1;
				}
			}
			else
			{
				recievedChars[ndx] = '\0'; //termiante the string
				recvInProgress = false;
				ndx = 0;
				newData = true;
			}
		}
		else if (rc == startMaker)
		{
			recvInProgress = true;
		}

	}
}


void parseCommands()
{
	command = recievedChars;
}


void setup() {
	Serial.begin(115200);
	pinMode(chip_select, OUTPUT);

	digitalWrite(chip_select, LOW);

	delay(50);
	digitalWrite(chip_select, HIGH);


	//retract();
	Serial.println("ready");
	//setTime(0);

}

// the loop function runs over and over again until power down or reset
void loop() {

	recieve_commands();
	if (newData == true)
	{
		parseCommands();
		newData = false;
	}

	if (command == "1")
	{
		retract();
		command = "0";
	}
	else if (command == "2")
	{
		double pos = update_position();

		// Will have to fix this at some point
		long pos_time = millis();

		float val = analogRead(0);
		long load_time = millis();


		if (control_bool == false)
		{
			if (desired_position > pos)
			{
				motor.Fwd(150);
			}
			else if (desired_position <= pos)
			{
				control_bool = true;
				newData_desired = false;
				desired_position = 00.00;
				motor.Stop();
				command = "0";
			}


			if (millis() > time + interval)
			{

        //val = map(val, analog_int, 2000, 0, 1000);

				time = millis();

				Serial.print(pos);
				Serial.print(",");
				Serial.print(pos_time);
				Serial.print(",");
				Serial.print(val);
				Serial.print(",");
				Serial.print(load_time);
				Serial.println();



			}
		}
		else if (control_bool == true)
		{

			Serial.println("give");
			//while (!Serial.available()) ;
			while (Serial.available() == 0)
			{
				desired_position = Serial.parseFloat();
				//Serial.println("lol");
				//Serial.println(desired_position);
				control_bool = false;
				newData_desired = true;

				//empty buffer
				//Serial.flush();
	

			}

		}

		


	}
	else if (command == "0")
	{
		motor.Stop();
	}



	




  
}


void retract()
{
	motor.Rev(255);
	delay(10000);

	motor.Stop();
	mag_sensor.write(0);
    zero_loadcell();
	Serial.println("begin");
}


