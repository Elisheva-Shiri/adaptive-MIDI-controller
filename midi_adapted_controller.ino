// For ATmega328 - Uno, Mega, Nano...
#include <MIDI.h> // by Francois Best
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, midiOut);

// BUTTONS
const int N_BUTTONS = 6; //*  total numbers of buttons
const int BUTTON_ARDUINO_PIN[N_BUTTONS] = {2, 3, 4, 5, 6, 7}; //* pins of each button connected straight to the Arduino
const int LED_ARDUINO_PIN[N_BUTTONS] = {8, 9, 10, 11, 12, 13};

int buttonCState[N_BUTTONS] = {};        // stores the button current value
int buttonPState[N_BUTTONS] = {};        // stores the button previous value

// debounce
unsigned long lastDebounceTime[N_BUTTONS] = {0};  // the last time the output pin was toggled
unsigned long debounceDelay = 12;    //* the debounce time; increase if the output flickers

/////////////////////////////////////////////
// POTENTIOMETERS
const int N_POTS = 3; //* total numbers of pots, slide & rotary
const int POT_ARDUINO_PIN[N_POTS] = {A0, A1, A2}; //* Pot's Arduino pins

int potCState[N_POTS] = {0}; // Current state of the pot
int potPState[N_POTS] = {0}; // Previous state of the pot
int potVar = 0; // Difference between the current and previous state of the pot

int midiCState[N_POTS] = {0}; // Current state of the midi value
int midiPState[N_POTS] = {0}; // Previous state of the midi value

const int PTIMEOUT = 300; //* Amount of time the potentiometer will be read after it exceeds the varThreshold
const int pvarThreshold = 10; //* Threshold for the potentiometer signal variation
boolean potMoving = false; // If the potentiometer is moving
unsigned long PTime[N_POTS] = {0}; // Previously stored time
unsigned long ptimer[N_POTS] = {0}; // Stores the time that has elapsed since the timer was reset

/////////////////////////////////////////////
// JOYSTICK
// Analog input of the horizontal joystick position
const int JoystickX1 = A3;
const int JoystickX2 = A4;
int joyX1 = 0;
int joyX2 = 0;

/////////////////////////////////////////////
// MIDI
byte midiCh = 1; //* MIDI channel to be used
byte note = 1; //* Lowest note to be used
byte cc = 15; //* Lowest MIDI CC to be used
byte cd = 20; //*Lowest Midi note to be used
/////////////////////////////////////////////
// SETUP
void setup() {
  Serial.begin(115200); //*

  // Buttons Initialize buttons with pull up resistors
  for (int i = 0; i < N_BUTTONS; i++) {
    pinMode(BUTTON_ARDUINO_PIN[i], INPUT_PULLUP);
  }
  for (int i = 0; i < N_BUTTONS; i++) {
    pinMode(LED_ARDUINO_PIN[i], OUTPUT);
    digitalWrite(LED_ARDUINO_PIN[i], LOW);
  }
#ifdef pin13 // inicializa o pino 13 como uma entrada
  pinMode(BUTTON_ARDUINO_PIN[pin13index], INPUT);
#endif

  // Pots Initialize 10k
  for (int i = 0; i < N_POTS; i++) {
    pinMode(POT_ARDUINO_PIN[i], INPUT);
  }

  // Joystick
  pinMode(JoystickX1, INPUT);
  pinMode(JoystickX2, INPUT);
}

/////////////////////////////////////////////
// LOOP
void loop() {

    buttons();
    potentiometers();
//    joystick();


}

/////////////////////////////////////////////
// BUTTONS
void buttons() {

  for (int i = 0; i < N_BUTTONS; i++) {

    buttonCState[i] = digitalRead(BUTTON_ARDUINO_PIN[i]);  // read pins from arduino

#ifdef pin13
    if (i == pin13index) {
      buttonCState[i] = !buttonCState[i]; // inverts the pin 13 because it has a pull down
    }
#endif

    if ((millis() - lastDebounceTime[i]) > debounceDelay) {

      if (buttonPState[i] != buttonCState[i]) {
        lastDebounceTime[i] = millis();

        if (buttonCState[i] == LOW) {
          // Sends the MIDI note ON accordingly to the chosen board
          // use if using with ATmega328 (uno, mega, nano...)
          digitalWrite(LED_ARDUINO_PIN[i] , HIGH);
          midiOut.sendControlChange(note + i, 127, midiCh); // note, velocity, channel
        }

        else {
          // Sends the MIDI note OFF accordingly to the chosen board
          digitalWrite(LED_ARDUINO_PIN[i] , LOW);
          midiOut.sendControlChange(note + i, 0, midiCh); // note, velocity, channel
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
    potCState[i] = analogRead(POT_ARDUINO_PIN[i]); // reads the pins from arduino
    midiCState[i] = map(potCState[i], 0, 1023, 0, 127); // Maps the reading of the potCState to a value usable in midi
    potVar = abs(potCState[i] - potPState[i]); // Calculates the absolute value between the difference between the current and previous state of the pot

    if (potVar > pvarThreshold) { // Opens the gate if the potentiometer variation is greater than the threshold
      PTime[i] = millis(); // Stores the previous time
    }
    ptimer[i] = millis() - PTime[i]; // Resets the timer 11000 - 11000 = 0ms

    if (ptimer[i] < PTIMEOUT) { // If the timer is less than the maximum allowed time it means that the potentiometer is still moving
      potMoving = true;
    }
    else {
      potMoving = false;
    }

    if (potMoving == true) { // If the potentiometer is still moving, send the change control
      if (midiPState[i] - midiCState[i] > debounceDelay ) {

        // Sends the MIDI CC accordingly to the chosen board
        midiOut.sendControlChange(cc + i, midiCState[i], midiCh); // cc number, cc value, midi channel
        potPState[i] = potCState[i]; // Stores the current reading of the potentiometer to compare with the next
        midiPState[i] = midiCState[i];
      }
    }
  }
}

/////////////////////////////////////////////
// JOYSTICK
void joystick() {
  // Process horizontal joystick position
  int x = analogRead(JoystickX1);
  int y = analogRead(JoystickX2);

  joyX1 = map(x, 0, 1023, 0, 127); // Maps the reading of the potCState to a value usable in midi
  joyX2 = map(y, 0, 1023, 0, 127); // Maps the reading of the potCState to a value usable in midi

  if (x > 700)
    midiOut.sendControlChange(cd , joyX1, midiCh); // cc number, cc value, midi channel;

  else if (x < 300)
    midiOut.sendControlChange(cd , -(joyX1), midiCh); // cc number, cc value, midi channel;

  if (y > 700)
    midiOut.sendControlChange(cd ,joyX2, midiCh); // cc number, cc value, midi channel;

  else if (y < 300)
    midiOut.sendControlChange(cd , -(joyX2), midiCh); // cc number, cc value, midi chan
    

}
