// Define your board, choose:
// "ATMEGA328" if using ATmega328 - Uno, Mega, Nano...
// "ATMEGA32U4" if using with ATmega32U4 - Micro, Pro Micro, Leonardo...
// "TEENSY" if using a Teensy board
// "DEBUG" if you just want to debug the code in the serial monitor
#define TEENSY 1 //* put here the uC you are using, like in the lines above followed by "1", like "ATMEGA328 1", "DEBUG 1", etc.

// LIBRARIES -- Defines the MIDI library -- //

// For ATmega328 - Uno, Mega, Nano...
#ifdef ATMEGA328
#include <MIDI.h> // by Francois Best
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, midiOut);

// For ATmega32U4 - Micro, Pro Micro, Leonardo...
#elif ATMEGA32U4
#include <MIDIUSB.h>

// For ATmega32U4 - Micro, Pro Micro, Leonardo...
#elif TEENSY
#include <Bounce.h>
#endif


// BUTTONS
const int N_BUTTONS = 7; //*  total numbers of buttons
const int BUTTON_ARDUINO_PIN[N_BUTTONS] = {6, 7, 8, 9, 10, 11, 12}; //* pins of each button connected straight to the controller
const int LED_ARDUINO_PIN[N_BUTTONS] = {0, 1, 2, 3, 4, 5, 21}; //* pins of each led connected straight to the controller

int buttonCState[N_BUTTONS] = {};        // stores the button current value
int buttonPState[N_BUTTONS] = {};        // stores the button previous value


// debounce
unsigned long lastDebounceTime[N_BUTTONS] = {0};  // the last time the output pin was toggled
unsigned long debounceDelay = 12;    //* the debounce time; increase if the output flickers

/////////////////////////////////////////////
// POTENTIOMETERS
const int N_POTS = 3; //* total numbers of pots, slide & rotary
const int POT_PIN[N_POTS] = {A0, A1, A2}; //* Pot's Arduino pins

int potCState[N_POTS] = {0}; // Current state of the pot
int potPState[N_POTS] = {0}; // Previous state of the pot
int potVar = 0; // Difference between the current and previous state of the pot

int midiPotCState[N_POTS] = {0}; // Current state of the midi value
int midiPotPState[N_POTS] = {0}; // Previous state of the midi value

const int PTIMEOUT = 300; //* Amount of time the potentiometer will be read after it exceeds the varThreshold
const int pvarThreshold = 10; //* Threshold for the potentiometer signal variation
boolean potMoving = true; // If the potentiometer is moving
unsigned long PTime[N_POTS] = {0}; // Previously stored time
unsigned long timer[N_POTS] = {0}; // Stores the time that has elapsed since the timer was reset

/////////////////////////////////////////////
// JOYSTICK
// Analog input of the horizontal joystick position
const int N_JOYS = 2; //* totall numbers of joysticks
const int JOY_X_PIN[N_JOYS] = {A3, A5}; //* Joy's X exis pins
const int JOY_Y_PIN[N_JOYS] = {A4, A6}; //* Joy's Y exis pins

int joyXState[N_JOYS] = {0}; // Current X exis state of the joy
int joyYState[N_JOYS] = {0}; // Current Y exis state of the joy
static int midiJoyState[N_JOYS] = {0}; 
int joyUpVar = 700; // Difference between the current and previous state of the X joy's exis
int joyDownVar = 300; // Difference between the current and previous state of the Y joy's exis

int midiJoyXState[N_JOYS] = {0}; // Current state of the midi value
int midiJoyYState[N_JOYS] = {0}; // Previous state of the midi value

const int JTIMEOUT = 300; //* Amount of time the joysticks will be read after it exceeds the varThreshold
boolean joyMovingUp = false; // If the joystic is moving up
boolean joyMovingDown = false; // If the joystic is moving down
unsigned long jTime[N_JOYS] = {0}; // Previously stored time
unsigned long jTimer[N_JOYS] = {0}; // Stores the time that has elapsed since the timer was reset


/////////////////////////////////////////////
// MIDI
byte midiCh = 1; //* MIDI channel to be used
byte note = 1; //* Lowest note to be used
byte cc = 15; //* Lowest MIDI CC to be used with the pot
byte ccJ = 20; //*Lowest Midi CC to be used with the joystick

/////////////////////////////////////////////
// SETUP
void setup() {
  // Baud Rate  if using with ATmega328 (uno, mega, nano...)
  // 31250 for MIDI class compliant | 115200 for Hairless MIDI
//  usbMIDI.begin();
  Serial.begin(9600);
  
  #ifdef DEBUG
  Serial.println("Debug mode");
  Serial.println();
  #endif
  
   // Buttons Initialize buttons with pull up resistors
    for (int i = 0; i < N_BUTTONS; i++) {
      pinMode(BUTTON_ARDUINO_PIN[i], INPUT_PULLUP);
    }
    for (int i = 0; i < N_BUTTONS; i++) {
      pinMode(LED_ARDUINO_PIN[i], OUTPUT);
      digitalWrite(LED_ARDUINO_PIN[i], LOW);
    }

  
   // Pots Initialize 10k
  for (int i = 0; i < N_POTS; i++) {
    pinMode(POT_PIN[i], INPUT);
  }


  // Thumb Joysticks
  for (int i = 0; i < N_JOYS; i++) {
    pinMode(JOY_X_PIN[i], INPUT);
  }
  for (int i = 0; i < N_JOYS; i++) {
    pinMode(JOY_Y_PIN[i], INPUT);
  }
  }

/////////////////////////////////////////////
// LOOP
void loop() {
  buttons();
  potentiometers();
  joysticks();
}

/////////////////////////////////////////////

// BUTTONS
void buttons() {

  for (int i = 0; i < N_BUTTONS; i++) {

    buttonCState[i] = digitalRead(BUTTON_ARDUINO_PIN[i]);  // read pins from arduino

//#ifdef pin13
//    if (i == pin13index) {
//      buttonCState[i] = !buttonCState[i]; // inverts the pin 13 because it has a pull down
//    }
//#endif

    if ((millis() - lastDebounceTime[i]) > debounceDelay) {

      if (buttonPState[i] != buttonCState[i]) {
        lastDebounceTime[i] = millis();

        if (buttonCState[i] == LOW) {
          digitalWrite(LED_ARDUINO_PIN[i] , LOW);
           // Sends the MIDI note ON accordingly to the chosen board
           #ifdef ATMEGA328
            // use if using with ATmega328 (uno, mega, nano...)
             midiOut.sendControlChange(note + i, 127, midiCh); // note, velocity, channel
            
            #elif ATMEGA32U4
            // use if using with ATmega32U4 (micro, pro micro, leonardo...)
            noteOn(midiCh, note + i, 127);  // channel, note, velocity
            MidiUSB.flush();
            
            #elif TEENSY
            //do usbMIDI.sendNoteOn if using with Teensy
            usbMIDI.sendNoteOn(note + i, 127, midiCh); // note, velocity, channel
            
            #elif DEBUG
            Serial.print(i);
            Serial.println(": button on");
            #endif
            }
          
        else {
          // Sends the MIDI note OFF accordingly to the chosen board
          digitalWrite(LED_ARDUINO_PIN[i] , HIGH  );
          
          // Sends the MIDI note OFF accordingly to the chosen board
            #ifdef ATMEGA328
            // use if using with ATmega328 (uno, mega, nano...)
             midiOut.sendControlChange(note + i, 0, midiCh); // note, velocity, channel
            
            #elif ATMEGA32U4
            // use if using with ATmega32U4 (micro, pro micro, leonardo...)
            noteOn(midiCh, note + i, 0);  // channel, note, velocity
            MidiUSB.flush();
            
            #elif TEENSY
            //do usbMIDI.sendNoteOn if using with Teensy
            usbMIDI.sendNoteOn(note + i, 0, midiCh); // note, velocity, channel
            
            #elif DEBUG
            Serial.print(i);
            Serial.println(": button off");
            #endif
        }
        buttonPState[i] = buttonCState[i];
      }
    }
  }
}

/////////////////////////////////////////////
// POTENTIOMETERS
void potentiometers() {


  for (int i = 0; i < N_POTS; i++) { // Loops through all the potentiometers
    potCState[i] = analogRead(POT_PIN[i]); // reads the pins from arduino
    midiPotCState[i] = map(potCState[i], 0, 1023, 0, 127); // Maps the reading of the potCState to a value usable in midi
    potVar = abs(potCState[i] - potPState[i]); // Calculates the absolute value between the difference between the current and previous state of the pot

    if (potVar > pvarThreshold) { // Opens the gate if the potentiometer variation is greater than the threshold
      PTime[i] = millis(); // Stores the previous time
    }
    timer[i] = millis() - PTime[i]; // Resets the timer 11000 - 11000 = 0ms

    if (timer[i] < PTIMEOUT) { // If the timer is less than the maximum allowed time it means that the potentiometer is still moving
      potMoving = true;
    }
    else {
      potMoving = false;
    }

    if (potMoving == true) { // If the potentiometer is still moving, send the change control
      if (midiPotPState[i] != midiPotCState[i]) {

        // Sends the MIDI CC accordingly to the chosen board
        // use if using with ATmega328 (uno, mega, nano...)
        #ifdef ATMEGA328
        midiOut.sendControlChange(cc + i, midiPotCState[i], midiCh); // cc number, cc value, midi channel
        
        //use if using with ATmega32U4 (micro, pro micro, leonardo...)
        #elif ATMEGA32U4
        controlChange(midiCh, cc + i, midiPotCState[i]); //  (channel, CC number,  CC value)
        MidiUSB.flush();
        
        //do usbMIDI.sendControlChange if using with Teensy
        #elif TEENSY
        usbMIDI.sendControlChange(cc + i, midiPotCState[i], midiCh); // cc number, cc value, midi channel
        
        #elif DEBUG
        Serial.print("Pot: ");
        Serial.print(i);
        Serial.print(" ");
        Serial.println(midiPotCState[i]);
        #endif

        potPState[i] = potCState[i]; // Stores the current reading of the potentiometer to compare with the next
        midiPotPState[i] = midiPotCState[i];
      }
    }
  }
}

/////////////////////////////////////////////
// THUMB JOYSTICKS
void joysticks() {

  for (int i = 0; i < N_JOYS; i++) { // Loops through all the joy's exis
    joyXState[i] = analogRead(JOY_X_PIN[i]); // reads the pins from the x exis
    joyYState[i] = analogRead(JOY_Y_PIN[i]); // reads the pins from the y exis
    midiJoyXState[i] = map(joyXState[i], 0, 1023, 0, 127); // Maps the reading of the joyXCState to a value usable in midi
    midiJoyYState[i] = map(joyYState[i], 0, 1023, 0, 127); // Maps the reading of the joyXCState to a value usable in midi

    jTimer[i] = millis() - jTime[i]; // Resets the timer 11000 - 11000 = 0ms
    if (jTimer[i] > JTIMEOUT) { // If the timer is less than the maximum allowed time it means that the joystick is still moving{
      if (joyXState[i] > joyUpVar || joyYState[i] > joyUpVar) { // Opens the gate if the potentiometer variation is greater than the threshold
        joyMovingUp = true;
        if (midiJoyState[i] < 123 ){
          midiJoyState[i] += 5 ; 
        }
      }

      else if (joyXState[i] < joyDownVar || joyYState[i] < joyDownVar) { // Opens the gate if the potentiometer variation is greater than the threshold
          joyMovingDown = true;
          if (midiJoyState[i] > 3 ){
            midiJoyState[i] -= 5 ; 
          }
        }

      else {
        midiJoyState[i] = 63 ;         
      }
      
      if (joyMovingDown == true) { // If the joystick is still moving, send the change control
          // Sends the MIDI CC accordingly to the chosen board
          // use if using with ATmega328 (uno, mega, nano...)
          #ifdef ATMEGA328
          midiOut.sendControlChange(ccJ + i, midiJoyState[i], midiCh); // cc number, cc value, midi channel
          
          //use if using with ATmega32U4 (micro, pro micro, leonardo...)
          #elif ATMEGA32U4
          controlChange(midiCh, ccJ + i, midiJoyState[i]); //  (channel, CC number,  CC value)
          MidiUSB.flush();
          
          //do usbMIDI.sendControlChange if using with Teensy
          #elif TEENSY
          usbMIDI.sendControlChange(ccJ + i, midiJoyState[i], midiCh); // cc number, cc value, midi channel
          
          #elif DEBUG
          Serial.print("Joy: ");
          Serial.print(i);
          Serial.print(" moving down ");
          Serial.println(midiJoyState[i]);
          #endif
      }
       
      if (joyMovingUp == true) { // If the joystick is still moving, send the change control
          // Sends the MIDI CC accordingly to the chosen board
          // use if using with ATmega328 (uno, mega, nano...)
          #ifdef ATMEGA328
          midiOut.sendControlChange(ccJ + i, midiJoyState[i], midiCh); // cc number, cc value, midi channel
          
          //use if using with ATmega32U4 (micro, pro micro, leonardo...)
          #elif ATMEGA32U4
          controlChange(midiCh, ccJ + i, midiJoyState[i]); //  (channel, CC number,  CC value)
          MidiUSB.flush();
          
          //do usbMIDI.sendControlChange if using with Teensy
          #elif TEENSY
          usbMIDI.sendControlChange(ccJ + i, midiJoyState[i], midiCh); // cc number, cc value, midi channel
          
          #elif DEBUG
          Serial.print("Joy: ");
          Serial.print(i);
          Serial.print(" moving up ");
          Serial.println(midiJoyState[i]);
          #endif
          
        }
    jTime[i] = millis(); // Stores the previous time
    joyMovingUp = false;
    joyMovingDown = false;
    }
  }
}

/////////////////////////////////////////////
// if using with ATmega32U4 (micro, pro micro, leonardo...)
#ifdef ATMEGA32U4

// Arduino (pro)micro midi functions MIDIUSB Library
void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}
#endif
