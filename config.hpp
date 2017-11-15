#ifndef CONFIG_HPP_
#define CONFIG_HPP_
#include <Arduino.h>
#include <Servo.h>

#define PROCESSING_VISUALIZER 1
#define SERIAL_PLOTTER  2

static int outputType = SERIAL_PLOTTER;

extern volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
extern volatile int Signal;                // holds the incoming raw data
extern volatile int IBI;
extern int pulsePin;                 // Pulse Sensor purple wire connected to analog pin 0
extern int blinkPin;                // pin to blink led at each beat
extern int fadePin;                  // pin to do fancy classy fading blink at each beat
extern int fadeRate;
extern volatile boolean Pulse;     // "True" when User's live heartbeat is detected. "False" when not a "live beat".
extern volatile boolean QS;        // becomes true when Arduoino finds a beat.

void sendDataToSerial(char symbol, int data );
void serialOutput();
void serialOutputWhenBeatHappens();
void ledFadeToBeat();
void interruptSetup();

#endif
