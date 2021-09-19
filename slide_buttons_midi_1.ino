#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial, midiOut);
int MIDImessage1, MIDImessage2, MIDImessage3, MIDImessage4, MIDImessage5;
//buttons
const int buttonsPin [4] = {8, 9, 10, 11 };
const int ledsPin [4]= {4, 5, 6, 7 };
int buttonsVal [4] = {-1} ;
int attempt =0;
// slides //

const int slide1 = A0;  // slide pins
const int slide2 = A1;
const int slide3 = A2;

int slideVal1 = 0;
int slideVal2 = 0;
int slideVal3 = 0;

int lastslideVal1 = 0;
int lastslideVal2 = 0;
int lastslideVal3 = 0;


// JOYSTICK //

const int joyX = A5;  // joystick pins
const int joyY = A4;

const int Xswitch = 52;  // axis switche pins
const int Yswitch = 50;

int joyXval = 0;
int joyYval = 0;
int lastJoyXval = 0;
int lastJoyYval = 0;

void readslide();
void readJoystick();
void buttons();
 
void setup() {

  Serial.begin(9600); // enable serial communication
  pinMode(Xswitch, INPUT_PULLUP);
  pinMode(Yswitch, INPUT_PULLUP);

  for (int i = 8; i<12; i++){
    pinMode(buttonsPin[i],INPUT_PULLUP);
  }
  for (int i = 4; i<8; i++){
    pinMode (ledsPin,OUTPUT);
  }

}

void loop() {
  readslide();         // read slide
  readJoystick();     // read joystick + on/off switches
  buttons();          // read buttons statuse
}

void buttonbs(){
  Serial.println("woo");
  
  for (int i = 4; i<8; i++){
    digitalWrite(ledsPin,LOW);
  }
  for (int i = 0; i<4; i++){
    buttonsVal[i] = ! digitalRead(buttonsPin[i + 8]);
  }

  for (int i = 0; i<4; i++){
    if (buttonsVal[i] == HIGH){
      midiOut.sendControlChange(i+1, 127,1 ); // send a MIDI CC num = note, 127 = velocity, 1 = channel
      digitalWrite(ledsPin[i],HIGH);
      Serial.println("waa " );
      Serial.println(i);
    }
    else {
      digitalWrite(ledsPin[i] , LOW);
      Serial.println("wee " );
      Serial.println(i);
    }
  }
}
  
void readslide() {

  int diff = 4; // difference amount

  // READ slide //
  
  slideVal1 = analogRead(slide1);
  slideVal2 = analogRead(slide2);
  slideVal3 = analogRead(slide3);

  // CALCULATE DIFFERENCE BETWEEN NEW VALUE AND LAST RECORDED VALUE //
  
  int slideVal1diff = slideVal1 - lastslideVal1;
  int slideVal2diff = slideVal2 - lastslideVal2;
  int slideVal3diff = slideVal3 - lastslideVal3;

  // SEND MIDI MESSAGE //
  
  if (abs(slideVal1diff) > diff) // execute only if new and old values differ enough
  {
    MIDImessage1 = map(slideVal1, 0, 1023, 0, 127);  // map sensor range to MIDI range
    midiOut.sendControlChange(1, MIDImessage1,1 );
    lastslideVal1 = slideVal1; // reset old value with new reading
  }

  if (abs(slideVal2diff) > diff)
  {
    MIDImessage2 = map(slideVal2, 0, 1023, 0, 127);  // map sensor range to MIDI range
    midiOut.sendControlChange(2, MIDImessage2,1 );
    lastslideVal2 = slideVal2;
  }

  if (abs(slideVal3diff) > diff)
  {
    MIDImessage3 = map(slideVal3, 0, 1023, 0, 127);
    midiOut.sendControlChange(3, MIDImessage3,1 );
    lastslideVal3 = slideVal3;
  }

  delay(2); // small delay further stabilizes sensor readings
}


void readJoystick() {

  int diff = 2;

  // READ LEFT-RIGHT //
  if (digitalRead(Xswitch) == 0) { // execute only if X-switch is turned on

    joyXval = analogRead(joyX);
    int joyXvalDiff = joyXval - lastJoyXval;

    if (abs(joyXvalDiff) > diff) // send MIDI message only if new and old values differ enough
    {
      MIDImessage4 = map(joyXval, 379, 637, 127, 0); // map sensor range to MIDI range
      midiOut.sendControlChange(4, MIDImessage4,1 );
      lastJoyXval = joyXval; // reset old value with new reading
    }
    delay(2); // for stability
  }


  // READ UP-DOWN FOR MODULATION //
  if (digitalRead(Yswitch) == 0) {

    joyYval = analogRead(joyY);
    int joyYvalDiff = joyYval - lastJoyYval;

    if (abs(joyYvalDiff) > diff)
    {
      // construct 14 bit modulation value and send over MIDI
      int modulation = map(joyYval, 361, 635, -8000, 8000);
      unsigned int change = 0x2000 + modulation;  //  0x2000 == no Change
      unsigned char low = change & 0x7F;  // low 7 bits
      unsigned char high = (change >> 7) & 0x7F;  // high 7 bits

      MIDImessage5 = 224;
      midiOut.sendControlChange(5, MIDImessage5,1 );

      lastJoyYval = joyYval;
    }
    delay(2);
  }

}
