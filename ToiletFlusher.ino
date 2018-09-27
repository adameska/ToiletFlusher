/* Ping))) Sensor

This sketch reads a PING))) ultrasonic rangefinder and returns the
distance to the closest object in range. To do this, it sends a pulse
to the sensor to initiate a reading, then listens for a pulse
to return.  The length of the returning pulse is proportional to
the distance of the object from the sensor.

The circuit:
* +V connection of the PING))) attached to +5V
* GND connection of the PING))) attached to ground
* SIG connection of the PING))) attached to digital pin 7

http://www.arduino.cc/en/Tutorial/Ping

created 3 Nov 2008
by David A. Mellis
modified 30 Aug 2011
by Tom Igoe

This example code is in the public domain.

*/

// this constant won't change.  It's the pin number
// of the sensor's output:
const int pingPin = 2;
const int readPin = 4;

int sleepTime = 10000; //check for someone every 10s, if we find someone then we will change this until they leave

//variables for the motor pins
int motorPin1 = 10;    // Blue   - 28BYJ48 pin 1
int motorPin2 = 11;    // Pink   - 28BYJ48 pin 2
int motorPin3 = 12;    // Yellow - 28BYJ48 pin 3
int motorPin4 = 13;    // Orange - 28BYJ48 pin 4
// Red    - 28BYJ48 pin 5 (VCC)

int motorRetractSpeed = 700;  //variable to set stepper speed
int motorReleaseSpeed = 1200;  //variable to set stepper speed
int lookup[8] = { B01000, B01100, B00100, B00110, B00010, B00011, B00001, B01001 };

const int greenLed = 7;      // the number of the LED pin
const int yellowLed = 8;

bool flushRequired = false;
int countsTilWeEnableFlush = 0;
int countsTilWeFlush = 0;
int countsToResetSleepTimer = 0;

void setup() {
	pinMode(motorPin1, OUTPUT);
	pinMode(motorPin2, OUTPUT);
	pinMode(motorPin3, OUTPUT);
	pinMode(motorPin4, OUTPUT);

	pinMode(greenLed, OUTPUT);
	pinMode(yellowLed, OUTPUT);

	pinMode(pingPin, OUTPUT);
	pinMode(readPin, INPUT);

	// initialize serial communication:
	Serial.begin(9600);
}

void loop()
{
	// establish variables for duration of the ping,
	// and the distance result in inches and centimeters:
	long duration, inches, cm;

	// The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
	// Give a short LOW pulse beforehand to ensure a clean HIGH pulse:

	digitalWrite(pingPin, LOW);
	delayMicroseconds(2);
	digitalWrite(pingPin, HIGH);
	delayMicroseconds(5);
	digitalWrite(pingPin, LOW);

	// The same pin is used to read the signal from the PING))): a HIGH
	// pulse whose duration is the time (in microseconds) from the sending
	// of the ping to the reception of its echo off of an object.
	duration = pulseIn(readPin, HIGH);

	// convert the time into a distance
	inches = microsecondsToInches(duration);
	cm = microsecondsToCentimeters(duration);

	Serial.print(inches);
	Serial.print("in, ");
	Serial.print(cm);
	Serial.print("cm");
	Serial.println();

	if (countsTilWeEnableFlush >= 10){ //assume it takes at least 5 seconds to go
		setFlush();
	}
	else if (flushRequired){
		if (inches > 26){
			countsTilWeFlush++;
			if (countsTilWeFlush > 3){
				flushToilet();
			}
		}
	}
	else if (inches <= 26){
		sleepTime = 500; //check every .5 second now
		countsTilWeEnableFlush++;
	}
	else{
		countsTilWeEnableFlush = 0;
		countsToResetSleepTimer++;
		if (countsToResetSleepTimer > 3)
		{
			sleepTime = 10000;
			countsToResetSleepTimer = 0;
		}
	}

	delay(sleepTime);
}

void setFlush(){
	flushRequired = true;
	digitalWrite(yellowLed, HIGH);
	sleepTime = 3000; //check every 5s for them to leave
	countsTilWeEnableFlush = 0;
	Serial.print("Flush set, we will flush when user leaves.");
	Serial.println();
}

void flushToilet(){
	Serial.println("Initiating Flush Toilet.");
	digitalWrite(yellowLed, LOW);
	digitalWrite(greenLed, HIGH);

	int countsRetracting = 0;
	int countsToStartReleasing = 1024;
	int countsTilWeAreDone = countsToStartReleasing * 2;

	//pull toilet flush
	Serial.println("Retracting.");
	for (countsRetracting; countsRetracting < countsToStartReleasing; countsRetracting++)
	{
		tightenStringWithMotor();
	}

	//release flusher
	Serial.println("Releasing.");
	for (countsToStartReleasing; countsToStartReleasing < countsTilWeAreDone; countsToStartReleasing++)
	{
		loosenStringWithMotor();
	}

	//make sure motor is off
	digitalWrite(motorPin1, LOW);
	digitalWrite(motorPin2, LOW);
	digitalWrite(motorPin3, LOW);
	digitalWrite(motorPin4, LOW);

	Serial.println("Done flushing Toilet.");
	flushRequired = false;
	countsTilWeFlush = 0;
	sleepTime = 10000; //back to 10s
	digitalWrite(greenLed, LOW);
}

void tightenStringWithMotor(){
	for (int i = 7; i >= 0; i--)
	{
		setOutput(i);
		delayMicroseconds(motorRetractSpeed);
	}
}

void loosenStringWithMotor(){
	for (int i = 0; i < 8; i++)
	{
		setOutput(i);
		delayMicroseconds(motorReleaseSpeed);
	}
}

void setOutput(int out)
{
	digitalWrite(motorPin1, bitRead(lookup[out], 0));
	digitalWrite(motorPin2, bitRead(lookup[out], 1));
	digitalWrite(motorPin3, bitRead(lookup[out], 2));
	digitalWrite(motorPin4, bitRead(lookup[out], 3));
}

long microsecondsToInches(long microseconds)
{
	// According to Parallax's datasheet for the PING))), there are
	// 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
	// second).  This gives the distance travelled by the ping, outbound
	// and return, so we divide by 2 to get the distance of the obstacle.
	// See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
	return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds)
{
	// The speed of sound is 340 m/s or 29 microseconds per centimeter.
	// The ping travels out and back, so to find the distance of the
	// object we take half of the distance travelled.
	return microseconds / 29 / 2;
}

