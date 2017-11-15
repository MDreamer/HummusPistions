#include <Arduino.h>
#include <Servo.h>
#include <FastLED.h>
#include "config.hpp"

Servo srvLeft;
Servo srvRight;

#define PISTON_NUM_LEDS 2
#define PISTON_LED_PIN_L 8
#define PISTON_LED_PIN_R 7

#define HOSE_NUM_LEDS 30
#define CHOKER_NUM_LEDS 10 //connected to the left hose
#define HOSE_LED_PIN_L 5
#define HOSE_LED_PIN_R 6

int max_piston_move = 177;
int min_piston_move = 3;
int pos = 0;    // variable to store the servo position mirror symetric location of the pistons over 3-177 degrees
int pinHeart = 1;
int dir = 1;	//servo/pistons direction. 1 = + (up), -1 = - (down)

bool heartDetected = true;

//LED pistons - 2 led, top & bottom per piston - left & right
CRGB piston_leds_L[PISTON_NUM_LEDS];
CRGB piston_leds_R[PISTON_NUM_LEDS];

//hoses from the backpack to the chocker
CRGB hose_leds_L[HOSE_NUM_LEDS+CHOKER_NUM_LEDS];
CRGB hose_leds_R[HOSE_NUM_LEDS];

//aux vars for no delay one loop handling
unsigned long lastServoMoveTime = 0;  // the last time the output pin was toggled
unsigned long currServoMoveTime = 0;  // the last time the output pin was toggled
unsigned long volatile servoSpeed = 5;  //the smaller the faster. - go between 2ms to 15ms

unsigned long lastHeartTime = 0;  // the last time the output pin was toggled
unsigned long currHeartTime = 0;  // the last time the output pin was toggled
unsigned long HeartCheck = 20;	//check it every 5 ms

//  Variables
int pulsePin = 7;                 // Pulse Sensor purple wire connected to analog pin 0
int blinkPin = 13;                // pin to blink led at each beat
int fadePin = 5;                  // pin to do fancy classy fading blink at each beat
int fadeRate = 0;                 // used to fade LED on with PWM on fadePin

// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded!
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat".
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

void setup() {
	Serial.begin(115200);
	srvLeft.attach(9);  // attaches the servo on pin 9 to the servo object
	srvRight.attach(10);  // attaches the servo on pin 9 to the servo object
	pinMode(pinHeart,INPUT);

	pinMode(blinkPin,OUTPUT);         // pin that will blink to your heartbeat!
	pinMode(fadePin,OUTPUT);          // pin that will fade to your heartbeat!

	//interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS


	FastLED.addLeds<NEOPIXEL, PISTON_LED_PIN_L>(piston_leds_L, PISTON_NUM_LEDS);
	FastLED.addLeds<NEOPIXEL, PISTON_LED_PIN_R>(piston_leds_R, PISTON_NUM_LEDS);
	FastLED.addLeds<NEOPIXEL, HOSE_LED_PIN_L>(hose_leds_L, HOSE_NUM_LEDS + CHOKER_NUM_LEDS);
	FastLED.addLeds<NEOPIXEL, HOSE_LED_PIN_R>(hose_leds_R, HOSE_NUM_LEDS);
}

void loop() {

	currServoMoveTime = millis();
	if (abs(currServoMoveTime - lastServoMoveTime) > servoSpeed)
	{
		/*
		int readHeart = analogRead(pinHeart);

		if (readHeart > 550)
		{
			servoSpeed = map(readHeart,400,800,12,1);
			if (servoSpeed > 12)
				servoSpeed = 12;
			if (servoSpeed < 0)
				servoSpeed = 0;
		}
		else
			servoSpeed = 12;*/

		int ledpos_up = map(pos,min_piston_move,max_piston_move,50,255);
		int ledpos_down = map(pos,min_piston_move,max_piston_move,255,50);
		if (pos < 177 && dir == 1) { // goes from 0 degrees to 180 degrees
			srvRight.write(pos);              // tell servo to go to position in variable 'pos'
			srvLeft.write(map(pos,min_piston_move,max_piston_move,max_piston_move,min_piston_move));

			piston_leds_R[0].setRGB(ledpos_up,0,(ledpos_up/14));
			piston_leds_R[1].setRGB(0,ledpos_down,ledpos_down);
			piston_leds_L[0].setRGB(ledpos_down,0,ledpos_down/14);
			piston_leds_L[1].setRGB(0,ledpos_up,ledpos_up);
			pos++;
		}
		else if (pos > 3 && dir == -1) { // goes from 0 degrees to 180 degrees
			srvRight.write(pos);              // tell servo to go to position in variable 'pos'
			srvLeft.write(map(pos,min_piston_move,max_piston_move,max_piston_move,min_piston_move));
			piston_leds_R[0].setRGB(ledpos_up,0,(ledpos_up/14));
			piston_leds_R[1].setRGB(0,ledpos_down,ledpos_down);
			piston_leds_L[0].setRGB(ledpos_down,0,(ledpos_down/14));
			piston_leds_L[1].setRGB(0,ledpos_up,ledpos_up);
			pos--;
		}else
			dir = dir * -1;

		//creates peak movement in the hoses
		int hose_peak = map(pos,min_piston_move,max_piston_move,0,HOSE_NUM_LEDS-1);
		int light_coefficient_g = 3;
		int light_coefficient_b = 4;
		int i_dec=HOSE_NUM_LEDS;
		for (int i=0; i < hose_peak; i++)
		{
			hose_leds_L[i].setRGB(0,i*light_coefficient_g,i*light_coefficient_b);
			hose_leds_R[i_dec].setRGB(0,i_dec*light_coefficient_g,i_dec*light_coefficient_b);
		}
		i_dec=hose_peak;
		for (int i=hose_peak; i < HOSE_NUM_LEDS; i++)
		{
			hose_leds_L[i_dec].setRGB(0,i_dec*light_coefficient_g,i_dec*light_coefficient_b);
			hose_leds_R[i].setRGB(0,i*light_coefficient_g,i*light_coefficient_b);
			i_dec--;
		}

		//choker animation
		int choker_peak = map(pos,min_piston_move,max_piston_move,0,CHOKER_NUM_LEDS-1);

		for (int i=0; i < choker_peak; i++)
		{
			hose_leds_L[HOSE_NUM_LEDS+i].setRGB(0,i*light_coefficient_g,i*light_coefficient_b);
		}
		i_dec=choker_peak;
		for (int i=choker_peak; i < CHOKER_NUM_LEDS; i++)
		{
			hose_leds_L[HOSE_NUM_LEDS+i].setRGB(0,i_dec*light_coefficient_g,i_dec*light_coefficient_b);
			i_dec--;
		}


		lastServoMoveTime = millis();
		FastLED.show();
	}

/*
	currHeartTime = millis();
	if (abs(currHeartTime - lastHeartTime) > HeartCheck)
	{
		lastHeartTime = millis();
		serialOutput() ;

		if (QS == true){     // A Heartbeat Was Found
						   // BPM and IBI have been Determined
						   // Quantified Self "QS" true when arduino finds a heartbeat
			fadeRate = 255;         // Makes the LED Fade Effect Happen
									// Set 'fadeRate' Variable to 255 to fade LED with pulse
			serialOutputWhenBeatHappens();   // A Beat Happened, Output that to serial.
			QS = false;                      // reset the Quantified Self flag for next time
			pos = map(Signal,400,600,3,177);
			if (pos > 177)
				pos = 177;
			if (pos < 3)
				pos = 3;
			srvRight.write(pos);              // tell servo to go to position in variable 'pos'
			srvLeft.write(map(pos,3,177,177,3));

		}
		//else
		//	servoSpeed = 12;


		ledFadeToBeat();                      // Makes the LED Fade Effect Happen
		//delay(20);                             //  take a break

	}

*/

}

void ledFadeToBeat(){
    fadeRate -= 15;                         //  set LED fade value
    fadeRate = constrain(fadeRate,0,255);   //  keep LED fade value from going into negative numbers!
    analogWrite(fadePin,fadeRate);          //  fade LED
  }
