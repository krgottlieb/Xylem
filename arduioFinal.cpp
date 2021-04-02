/*
   Rebecca Gottlieb
   System Analysis and Design
   Mr.Miller
   04/2020

   The purpose of this project is to create a plant robot, named Xylem.
   He has two main components: A temperature and humidity module and a soil
   moisture module. This will allow you to get a reading of the rooms temperature
   and humidity level, and the soils moisture level. If he is too dry or the room
   is too hot, his LED eyes will turn red. If he is too wet or the room is too cold,
   his eyes will turn blue. He also has many smaller components implemented. He has
   RGB LED eyes and an LED dot matrix for a mouth. Given any situation he will 
   "speak to you." If he's happy, he smiles and gives you the current temperature of 
   the room. If he's uncomfortable, he will complain! He also has a speaker that acts as
	a nose. Given some situations his "nose" will play a funny sound effect to go along with
   his temperment.  He contains a PIR sensor, so he can send you Xs and Os to show
   you how much he cares for you when you walk past. Xylem also is equiped with a
   photo resistor and will play a light show as a night-light when the room is too dark.
   To top off his unique structure, he also contains twinkle lights for hair.

*/

//Sound module
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
SoftwareSerial softwareSerial(10, 11);
DFRobotDFPlayerMini player;


//Humidity and Temp module
#include <Adafruit_Sensor.h>
#include <DHT.h>
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht = DHT(DHTPIN, DHTTYPE);
int tempCtr = 0;

//LED Matrix
#include <MaxMatrix.h>
#include <avr/pgmspace.h>
#include "mouthInfo.h"
#define maxDisplays 2
MaxMatrix m(data, load, clock, maxDisplays);

//LED eyes
const int BLED = 7;
const int GLED = 8;
const int RLED = 9;
int colorNum;

//Photo resistor
const int LIGHT = 1;

//PIR Sensor
const int pirPin = 2;
int pirState = LOW;

//Moisture Sensor
int moistureVal = 0; //value for storing moisture value
int soilPin = A0;
int soilPower = 7; //Variable for soil moisture power
int moistureCtr = 0;

/*
  If it's too dark, a light show will help you to see.
  It's a do while loop for two reasons.
  First: You can see all night.
  Second: It won't continue back to the main loop.
  This means you won't hear noises, or continually be updated all night long.
*/
void NightLight() 
{
	int brightness = analogRead(LIGHT);
	Serial.print("light: ");
	Serial.println(brightness);

	while (brightness <= 450) 
	{
		digitalWrite(BLED, HIGH);
		digitalWrite(GLED, HIGH);
		digitalWrite(RLED, HIGH);
		delay(500);
		digitalWrite(BLED, HIGH);
		digitalWrite(GLED, LOW);
		digitalWrite(RLED, LOW);
		delay(500);
		digitalWrite(BLED, LOW);
		digitalWrite(GLED, HIGH);
		digitalWrite(RLED, LOW);
		delay(500);
		digitalWrite(BLED, LOW);
		digitalWrite(GLED, LOW);
		digitalWrite(RLED, HIGH);
		delay(500);
		analogWrite(BLED, 127);
		analogWrite(GLED, 0);
		analogWrite(RLED, 127);
		delay(500);
		analogWrite(BLED, 127);
		analogWrite(GLED, 127);
		analogWrite(RLED, 0);
		delay(500);
		analogWrite(BLED, 0);
		analogWrite(GLED, 127);
		analogWrite(RLED, 127);
		delay(500);
		brightness = analogRead(LIGHT);//Check to see if the loop should continue
	}
}

//Check for motion...sounds and lights can be distracting, so a nice message will do!
void PIRStatus() 
{
	pirState = digitalRead(pirPin);
	if (pirState == HIGH) 
	{
		Serial.println("I see you!!! PIR Module is working!");
		char string[] = "XOXOXO    ";
		m.shiftLeft(false, true);
		printStringWithShift(string, 100);
	} 
	else 
	{
		Serial.println ("Where are you?");
	}
}

/*
  Xylem will tell you if he's too hot, too cold, or the current temp if he's happy
  It takes a lot of space on the arduino to change an int into a string, and then convert
  it to a character array...so I used it for only one condition.  Make sure to give extra
  space to your array, because it scrolls. It's also important to empliment a counter or 
  he'll continue to play sounds and scroll text.
*/
void TempAndHumidity() 
{
	float h = dht.readHumidity();
	float t = dht.readTemperature();
	float f = dht.readTemperature(true);
	//Compute heat index in Fahrenheit (default)
	float hif = dht.computeHeatIndex(f, h);

	Serial.print("Humidity: ");
	Serial.print(h);
	Serial.print(" % ");
	Serial.print("Temperature: ");
	Serial.print(f);
	Serial.print(" \xC2\xB0");
	Serial.println("F ");
	if (tempCtr % 60 == 0 ) 
	{
		if (f > 75) 
		{
			colorNum = 2;
			CurrentColor();
			delay(500);
			player.play (1);//"Dragon breath"
			char string[] = "It's hot!!!    ";
			m.shiftLeft(false, true);
			printStringWithShift(string, 100);
		} 
		else if (f < 68) 
		{
			colorNum = 0;
			CurrentColor();
			delay(500);
			player.play (2);//"Teeth chattering"
			char string[] = "BRRRRR!    ";
			m.shiftLeft(false, true);
			printStringWithShift(string, 100);
		} 
		else 
		{
			colorNum = 1;
			CurrentColor();
			delay(500);
			String temp = "Temp: ";
			temp += String(f);
			int strLng = temp.length() + 4;
			char charTemp[strLng];
			temp.toCharArray(charTemp, strLng);
			Serial.println(charTemp);
			char string[] = {charTemp};
			m.shiftLeft(false, true);
			printStringWithShift(string, 200);
			delay(500);
		}
	}
	tempCtr++;
	ctrReset(tempCtr);
}

//Check to make sure the soil isn't too moise or too dry.
//Turning the power to low helps keep the sensor working longer
//Don't forget a counter or he'll continue to play sounds and "speak"
void ReadSoil() 
{
	Serial.print("Soil Moisture = ");
	Serial.println(moistureVal);
	digitalWrite(soilPower, HIGH);
	delay(50);
	moistureVal = analogRead(soilPin);
	digitalWrite(soilPower, LOW);
	if (moistureCtr % 60 == 0) 
	{
		if (moistureVal > 550)
		{
			colorNum = 0;
			CurrentColor();
			delay(500);
			player.play (4);//"Drowning"
			char string[] = "  OoOoOoOo";
			m.shiftLeft(false, true);
			printStringWithShift(string, 100);
		} 
		else if (moistureVal < 300) 
		{
			colorNum = 2;
			CurrentColor();
			delay(500);
			player.play (5);//"Feed Me"
			char string[] = "Water!!!!    ";
			m.shiftLeft(false, true);
			printStringWithShift(string, 100);
		} 
		else 
		{
			colorNum = 1;
			CurrentColor();
			delay(500);
			char string[] = " \\_/";
			m.shiftLeft(false, true);
			printStringWithShift(string, 0);
		}
	}
	moistureCtr++;
	ctrReset(moistureCtr);
}

/*
  Two functions for the Dot Matrix Mouth
  printCharWithShift takes the character string array, and uses the
  printStringWithShift to determine the scrolling speed (higher is slower).
  The mouthInfo header file contains the rest of the programming for
  the mouth. This site had the best information on how to link dot matrix's:
  https://www.best-microcontroller-projects.com/max7219.html#Library_MaxMatrix_
*/
void printCharWithShift(char c, int shift_speed)
{
	if (c < 32) return;
	c -= 32;
	memcpy_P(Buf7219, CH + 7 * c, 7);
	m.writeSprite(maxDisplays * 8, 0, Buf7219);
	m.setColumn(maxDisplays * 8 + Buf7219[0], 0);

	for (int i = 0; i <= Buf7219[0]; i++) 
	{
		delay(shift_speed);
		m.shiftLeft(false, false);
	}
}

void printStringWithShift(char* s, int shift_speed) 
{
	while (*s != 0) 
	{
		printCharWithShift(*s, shift_speed);
		s++;
	}
}

//Reset the counters back to zero
void ctrReset(int ctr) 
{
	if (ctr == 60) {ctr = 0;}
}

void CurrentColor() 
{
	switch (colorNum) 
	{
		case 0:
			digitalWrite(BLED, HIGH);
			digitalWrite(GLED, LOW);
			digitalWrite(RLED, LOW);
			break;
		case 1:
			digitalWrite(BLED, LOW);
			digitalWrite(GLED, HIGH);
			digitalWrite(RLED, LOW);
			break;
		case 2:
			digitalWrite(BLED, LOW);
			digitalWrite(GLED, LOW);
			digitalWrite(RLED, HIGH);
			break;
	}
}

void setup () 
{
	Serial.begin (9600);
	
	//Sound module and speaker
	softwareSerial.begin(9600);
	if (player.begin(softwareSerial)) {Serial.println("OK");} 
	else {Serial.println("Fail");}
	player.volume(30);
	
	//PIR sensor
	pinMode(pirPin, INPUT);

	//RGB LED's
	pinMode(BLED, OUTPUT);
	pinMode(GLED, OUTPUT);
	pinMode(RLED, OUTPUT);
	
	//Temp and humidity
	dht.begin();
	
	//Soil Moisture
	pinMode(soilPower, OUTPUT);
	digitalWrite(soilPower, LOW);
	
	//Dot matrix
	m.init();
	m.setIntensity(3);
}

void loop () 
{
	//Set the current color for the begining of the loop
	CurrentColor();
	delay(500);
	//Check the photo resistor
	NightLight();
	delay(500);
	//Check for motion
	PIRStatus();
	delay(500);
	//Check the temp and humidity
	TempAndHumidity();
	delay(500);
	//Check the soil moisture
	ReadSoil();
	delay(500);
}
