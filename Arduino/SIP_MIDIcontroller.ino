// THE MIDINATOR //

// Author: Daniel Jansson // Switch & Lever // switchandlever.com

// This code is a companion to the YouTube video on how to build a MIDI controller (called
// The MIDInator) linked below:

// https://youtu.be/JZ5yPdoPooU

// The MIDInator uses an Arduino MEGA 2560, connected to a keypad, a few potentiometers, rotary
// encoders, switches and a joystick to send MIDI messages over USB serial to a Serial to
// MIDI bridge software running on the computer.

// COMPONENT LIST

// 1x   Arduino MEGA 2560
// 3x   10kOhm (B10K) linear rotary potentiometer
// 1x   10kOhm (B10K) linear slide potentiometer
// 1x   ALPS 6 position rotary switch (SRBM160700)
// 2x   ALPHA 12 step rotary encoder (RE130F-41-175F-12P)
// 1x   phone keypad from old Ericsson Diavox
// 2x   generic slide toggle switches
// 1x   generic (ON)-OFF-(ON) toggle switch
// 1x   remote control joystick (2 potentiometers)

// The libraries used in this sketch can be found at:

// Keypad.h :: https://github.com/Chris--A/Keypad
// Encoder.h :: https://github.com/PaulStoffregen/Encoder

// Please refer to the documentation for the individual libraries on their respective
// operation and function.

// This code is licensed under a GNU General Public License and may be freely modified
// and redistributed under the same license terms.
// https://www.gnu.org/licenses/gpl-3.0.en.html


// KEYPAD //

#include <Keypad.h>

const byte ROWS = 4; // four rows
const byte COLS = 4; // four columns
char keys[ROWS][COLS] = {  // keypad keys, 1-9, 0, S for star (asterisk) and P for pound (square)
  {'0', '7', '2','A'},
  {'9', '4', ';','B'},
  {'6', '1', '8','C'},
  {'3', ':', '5','D'}
};

byte rowPins[ROWS] = {4,5,6,7}; // keypad row pinouts
byte colPins[COLS] = {8,9,10,11}; // keypad column pinouts
const int buttonPin = 2;  // the number of the pushbutton pin
volatile byte buttonState = LOW; // variable for reading the pushbutton status

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

int midC = 60; // MIDI note value for middle C on a standard keyboard

// const int transposePin1 = 22; // pins for the switch controlling transposing
// const int transposePin2 = 23;
int transpose = 0;  // if = 0 no transposing
// int transposeLeft = 0;
// int transposeRight = 0;
// int oldTransposeLeft = 0;
// int oldTransposeRight = 0;
// unsigned long transposeTimer = 0;  // for debouncing


// // ROTARY ENCODER //

// #define ENCODER_DO_NOT_USE_INTERRUPTS
// #include <Encoder.h>

// Encoder myEnc1(26, 27);
// Encoder myEnc2(24, 25);
// long position1  = -999;
// long position2  = -999;
// int encVals[12] = {64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64}; // set initial value of encoder to mid range of 0-127


// ROTARY SWITCH //

// const int rotSwitch1 = 30;  // rotary switch pins
// const int rotSwitch2 = 32;
// const int rotSwitch3 = 34;
// const int rotSwitch4 = 36;
// const int rotSwitch5 = 38;
// const int rotSwitch6 = 40;
// int cVal = 1;


// POTENTIOMETERS //

// const int pot1 = A0;  // potentiometer pins
// const int pot2 = A1;
// const int pot3 = A2;
// const int slidePot = A3;

// int potVal1 = 0;
// int potVal2 = 0;
// int potVal3 = 0;
// int slidePotVal = 0;

// int lastPotVal1 = 0;
// int lastPotVal2 = 0;
// int lastPotVal3 = 0;
// int lastSlidePotVal = 0;


// // JOYSTICK //

// const int joyX = A5;  // joystick pins
// const int joyY = A4;

// const int Xswitch = 52;  // axis switche pins
// const int Yswitch = 50;

// int joyXval = 0;
// int joyYval = 0;
// int lastJoyXval = 0;
// int lastJoyYval = 0;
/*
  The keypad runs on the keypad.h library, which you also can use
  to run other similar keypads with similar pinouts, or even use
  regular micro switches in an array to build your own keypad.

  With the use of a bidirectional toggle switch the keypad
  mapping can also be transposed, or moved, up and down the
  scale. The keypad starts with middle C (note number 60) on the
  Nr 1 key. Using the transpose switch moves the keypad one entire
  octave up or down, within the full range (0 - 127) of available
  notes.
*/

int kpc = 144; // the function of the keypad, 144 = Channel 1 Note on

int progChg = 192;
/*
  The MIDI message consists of three bytes, first setting the status
  of the message, and the following bytes denoting the action, which
  note number, velocity, pressure etc. The data bytes are different
  depending on the type of the message set by the status byte.

  Refer to the following chart for which bytes do what:

  https://www.midi.org/specifications-old/item/table-2-expanded-messages-list-status-bytes
*/

void MIDImessage(byte status, byte data1, byte data2)
{
  Serial.write(status);
  Serial.write(data1);
  Serial.write(data2);
}

void MIDIProg(byte status, byte data ) {
  Serial.write(status);
  Serial.write(data);
}

void readKeyPad() {

  // // TRANSPOSE SWITCH //

  // transposeRight = digitalRead(transposePin1);

  // if (oldTransposeRight != transposeRight && transposeRight == 0) { // detect switch change, and only do this once regardless how long it's held down
  //   if (millis() - transposeTimer > 1000) { // debounce so you can only do this once a second
  //     if (transpose < 48) transpose = transpose + 12; // only change transpose value if it's smaller than 48
  //     transposeTimer = millis();// reset debounce timer
  //   }
  // }
  // oldTransposeRight = transposeRight;

  // transposeLeft = digitalRead(transposePin2);

  // if (oldTransposeLeft != transposeLeft && transposeLeft == 0) { // same as above but to decrease the transpose value
  //   if (millis() - transposeTimer > 1000) {
  //     if (transpose > -60) transpose = transpose - 12; // only change transpose value if it's bigger than -60
  //     transposeTimer = millis();
  //   }
  // }
  // oldTransposeLeft = transposeLeft;


  // KEYPAD //

  if (kpd.getKeys())  // supports up to ten simultaneous key presses
  {
    for (int i = 0; i < LIST_MAX; i++) // scan the whole key list
    {
      if ( kpd.key[i].stateChanged)   // find keys that have changed state
      {
          const int key = kpd.key[i].kchar - '0';
        if (kpd.key[i].kstate == PRESSED) // sends MIDI on message when keys are pressed
        {
          if(key>=0 && key<=11) {
            MIDImessage(kpc, midC + (transpose *12) + key, 127);
          }
        }

        if (kpd.key[i].kstate == RELEASED)  // sends MIDI off message when keys are released
        {
          if(key>=0 && key<=11) {
            MIDImessage(kpc -16, midC + (transpose *12) + key, 127);
          }else if(key == 17){
            transpose++;
            MIDIProg(progChg,transpose);

          }else if(key == 18){
            transpose--;
            MIDIProg(progChg,transpose);

          }
        }
      }
    }
  }
}



void setup() {

  Serial.begin(31250); // enable serial communication

  // pinMode(transposePin1, INPUT_PULLUP);  // activate the input pullup resistor on all buttons/switches
  // pinMode(transposePin2, INPUT_PULLUP);  // means you won't need external resistors to read the buttons
  // pinMode(rotSwitch1, INPUT_PULLUP);
  // pinMode(rotSwitch2, INPUT_PULLUP);
  // pinMode(rotSwitch3, INPUT_PULLUP);
  // pinMode(rotSwitch4, INPUT_PULLUP);
  // pinMode(rotSwitch5, INPUT_PULLUP);
  // pinMode(rotSwitch6, INPUT_PULLUP);
  // pinMode(Xswitch, INPUT_PULLUP);
  // pinMode(Yswitch, INPUT_PULLUP);      
   // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButton, FALLING );
  // attachInterrupt(digitalPinToInterrupt(buttonPin), buttonOnRelease, RISING );

} 


void loop() {
// read the state of the pushbutton value:
  // readRotSwitch();    // read roatary switch
  // readEncoder();      // read roraty encoder
  readKeyPad();       // read keypad + transpose switch
  // readPots();         // read potentiometers
  // readJoystick();     // read joystick + on/off switches
 

}

void handleButton(){
  Serial.println("Change");

    // buttonState = !buttonState;
    // if(buttonState){
    //  Serial.println("press");
    // }else{
    //  Serial.println("release");
    // }
    // MIDImessage(kpc, midC + (transpose *12), 127);
}

void buttonOnRelease(){
      Serial.println("release"),
    MIDImessage(kpc -16, midC + (transpose *12), 127);
}
