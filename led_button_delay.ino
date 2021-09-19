#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial, midiOut);
const int buttonsPin [4] = {8, 9, 10, 11 };
const int ledsPin [4]= {4, 5, 6, 7 };
int buttonsVal [4] = {-1} ;
int attempt =0;

void setup(){
  Serial.begin(9600);
  Serial.println("why");
  for (int i = 8; i<12; i++){
    pinMode(buttonsPin[i],INPUT_PULLUP);
  }
  for (int i = 4; i<8; i++){
    pinMode (ledsPin,OUTPUT);
  }
}
  
void loop(){
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
  delay(1000);
}
