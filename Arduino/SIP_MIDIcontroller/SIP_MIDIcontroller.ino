#include <Keypad.h>
#include <uClock.h>
#include <MIDI.h>
#include "Wire.h"
#include <MPU6050_light.h>
// Create and bind the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();

/*-------------------KEYPAD-VARS-----------------*/

#define Rkey 17
#define RPkey 18
#define STAR 10
#define HASHMRK 11

const byte ROWS = 4; // four rows
const byte COLS = 4; // four columns
char keys[ROWS][COLS] = {  // keypad keys, 1-9, 0, S for star (asterisk) and P for pound (square)
  {'1', '2', '3','A'},
  {'4', '5', '6','B'},
  {'7', '8', '9','C'},
  {':', '0', ';','D'}
};

int notes[12] = {10,0,1,2,3,4,5,6,7,8,9,11};
byte rowPins[ROWS] = {4,5,6,7}; // keypad row pinouts
byte colPins[COLS] = {8,9,10,11}; // keypad column pinouts
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


/*-------------------FSM-VARS-------------------*/
enum {DEF,R,RP,RRP};
unsigned char state;
unsigned long stateChangeTimer;

/*-------------------SEQUENCER-VARS-------------*/

// Sequencer config
#define N_TRACKS 3
#define STEP_MAX_SIZE      16
#define NOTE_LENGTH        12 // min: 1 max: 23 DO NOT EDIT BEYOND!!! 12 = 50% on 96ppqn, same as original tb303. 62.5% for triplets time signature
#define NOTE_VELOCITY      90
#define ACCENT_VELOCITY    127

// MIDI config
#define MIDI_CHANNEL      0 // 0 = channel 1

// do not edit below!
#define NOTE_STACK_SIZE    3

// MIDI clock, start, stop, note on and note off byte definitions - based on MIDI 1.0 Standards.
#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP  0xFC
#define NOTE_ON    0x90
#define NOTE_OFF   0x80

#define MIDI_SYS_EXC 0xF0
#define MIDI_SYS_EXC_END 0xF7
#define MIDI_MANUFACTURER_ID 0x7D


#define BASE_NOTE 60 //middle-C MIDI

// Sequencer data
typedef struct
{
  uint8_t note;
  bool accent;
  bool glide;
  bool rest;
} SEQUENCER_STEP_DATA;

typedef struct
{
  uint8_t note;
  int8_t length;
  uint8_t channel;
} STACK_NOTE_DATA; 


typedef struct
{
  SEQUENCER_STEP_DATA _sequence[STEP_MAX_SIZE];
  bool mute;
}SEQUENCE_DATA;

STACK_NOTE_DATA _note_stack[NOTE_STACK_SIZE];

// main sequencer data
SEQUENCE_DATA _sequencer[N_TRACKS];
// STACK_NOTE_DATA _note_stack[N_TRACKS][NOTE_STACK_SIZE];
uint16_t _step_length = STEP_MAX_SIZE;

// make sure all above sequencer data are modified atomicly only
// eg. ATOMIC(_sequencer[0].accent = true); ATOMIC(_step_length = 7);
#define ATOMIC(X) noInterrupts(); X; interrupts();

// shared data to be used for user interface feedback
bool _playing = false;
uint16_t _step = 0;

int selectedSequence = 0;
int selectedStep = 1;
int digitedStep = 0;
int transpose = 0;

#define TRANSPOSE_OFFSET 64 // for messaging purposes, not to end a negative number the transpose is shifted by 64

byte selectedSeq[STEP_MAX_SIZE + 3];




/*------------------------GYRO----------------------------*/

MPU6050 mpu(Wire);
unsigned long timer = 0;

const float sensorMin = 0;         // Minimum sensor value (degrees)
const float sensorMax = 120;       // Maximum sensor value (degrees)

float sensorValue;                 // value read directly from sensor (processed by MPU6050 library)
int mappedValue;                   // value of MIDI content

float previousSensorValue;         // used to avoid sending useless MIDI messages if sensor value doesn't change

const byte resetCoordinatesBtn = 2; // button used for re-calibration of sensor coordinates
float baseValue = 0;               // coordinate offset
int resetBtnState =0;
int prevResetBtnState = 0;


/*--------------------------------------------*/




/*-------------------UTILS--------------------*/
int getNote(int key){
  return BASE_NOTE + notes[key] + 12*transpose;
}

void sendMidiMessage(uint8_t command, uint8_t byte1, uint8_t byte2)
{ 
  // send midi message
  command = command | (uint8_t)MIDI_CHANNEL;
 // Serial.println("\ncommand:\t");
  //Serial.println(command);
 // Serial.println("\nbyte1:\t");
  //Serial.println(byte1);
   //Serial.println("\nbyte2:\t");
  //Serial.println(byte2);

  Serial.write(command);
  Serial.write(byte1);
  Serial.write(byte2);
}

void sendSysExcMidiMessage(uint8_t byte1, uint8_t byte2){
  Serial.write(MIDI_SYS_EXC);
  Serial.write(MIDI_MANUFACTURER_ID);
  Serial.write(byte1);
  Serial.write(byte2);
  Serial.write(MIDI_SYS_EXC_END);
}

void printSequencer(){
  for(int t = 0; t<N_TRACKS;t++){
    Serial.print("\nSequence\t");
    Serial.println(t);
    for(int i=0;i<STEP_MAX_SIZE;i++){
      Serial.print(_sequencer[t]._sequence[i].note);
      Serial.print(",\t");
    }
  }
}

void sendSelectedSequenceVector(){
  for (int i = 0; i < STEP_MAX_SIZE; ++i) {
      selectedSeq[i + 2] = _sequencer[selectedSequence]._sequence[i].note;
  }
  // // Print the contents of selectedSeq
  // for (int i = 0; i < STEP_MAX_SIZE + 3; ++i) {
  //     Serial.print(selectedSeq[i], HEX);
  //     Serial.print(" ");
  // }
  // Serial.println();
 MIDI.sendSysEx(STEP_MAX_SIZE +3, selectedSeq,true);
}

void sendState(){
  byte toSendState[4] = {0xF0,4,state,0xF7};
  MIDI.sendSysEx(4, toSendState,true);
}

void sendTranspose(){
  byte toSendTranspose[4] = {0xF0, 5, (byte)(transpose + TRANSPOSE_OFFSET), 0xF7}; // Ensure transpose is cast to byte
  MIDI.sendSysEx(sizeof(toSendTranspose), toSendTranspose, true);
}
/*----------------STEP-SEQUENCER--------------*/

// The callback function called by uClock each Pulse of 16PPQN clock resolution. Each call represents exactly one step.
void onStepCallback(uint32_t tick) 
{
  uint16_t step;
  uint16_t length = NOTE_LENGTH;
  
  for(int t=0;t<N_TRACKS;t++ ){
    if(_sequencer[t].mute)continue;
  // get actual step.
    _step = tick % _step_length;
    // send note on only if this step are not in rest mode
    if ( _sequencer[t]._sequence[_step].rest == false ) {
      // check for glide event ahead of _step
      step = _step;
      for ( uint16_t i = 1; i < _step_length; i++  ) {
        ++step;
        step = step % _step_length;
        if ( _sequencer[t]._sequence[step].glide == true && _sequencer[t]._sequence[step].rest == false ) {
          length = NOTE_LENGTH + (i * 24);
          break;
        } else if ( _sequencer[t]._sequence[step].rest == false ) {
          break;
        }
      }
    }
      // find a free note stack to fit in
    for ( uint8_t i = 0; i < NOTE_STACK_SIZE; i++ ) {
      // Serial.print("sequence:\t");
      // Serial.println(t);
  
      if ( _note_stack[i].length == -1 ) {
        _note_stack[i].note = _sequencer[t]._sequence[_step].note;
        _note_stack[i].length = length;
        _note_stack[i].channel = t+1;
        //  Serial.print("\n_note_stack[i].note:\t");
        //  Serial.println(_note_stack[i].note);

        // Serial.print("_note_stack[i].length:\t");
        // Serial.println(_note_stack[i].length);

        // send note on
        if( _note_stack[i].note!=0){
          // Serial.println(i);
          // Serial.print("\nNOTEON:\t");
          // Serial.println(_note_stack[i].note);
          // Serial.println(_note_stack[i].length);
          // Serial.println(_note_stack[i].channel);
          sendMidiMessage(NOTE_ON+_note_stack[i].channel, _note_stack[i].note, _sequencer[t]._sequence[_step].accent ? ACCENT_VELOCITY : NOTE_VELOCITY);    
        }
        break; 
        // return;
      }
    }
  } 
}

// The callback function called by uClock each Pulse of 96PPQN clock resolution.
void onPPQNCallback(uint32_t tick) 
{
  // Send MIDI_CLOCK to external hardware
  Serial.write(MIDI_CLOCK);

  // handle note on stack
  for ( uint8_t i = 0; i < NOTE_STACK_SIZE; i++ ) {
    if ( _note_stack[i].length != -1 ) {
        // Serial.print("\n_sequencer[t]._note_stack[i].length:\t");
        // Serial.println(_sequencer[t]._note_stack[i].length);
      --_note_stack[i].length;
      if (_note_stack[i].length == 0) {
        if(_note_stack[i].note!=0){
          // Serial.print("\nNOTEOFF:\t");
          // Serial.println(_note_stack[i].note);
          // Serial.println(_note_stack[i].length);
          // Serial.println(_note_stack[i].channel);



          sendMidiMessage(NOTE_OFF+_note_stack[i].channel,_note_stack[i].note, 0);
        }
        _note_stack[i].length = -1;
      }
    }
  }
}

// The callback function called when clock starts by using Clock.start() method.
void onClockStart() 
{
  Serial.write(MIDI_START);
  _playing = true;
}

// The callback function called when clock stops by using Clock.stop() method.
void onClockStop() 
{
  Serial.write(MIDI_STOP);
  // send all note off on sequencer stop
  
  for ( uint8_t i = 0; i < NOTE_STACK_SIZE; i++ ) {
    sendMidiMessage(NOTE_OFF+_note_stack[i].channel,_note_stack[i].note, 0);
      _note_stack[i].length = -1;
  }

  _playing = false;
}



/*--------------------MANAGE SEQUENCES-------------------------*/
void keyNoteOn(int note){
  // //Serial.print("keynoteOn:\t");
  // //Serial.println(note);
  sendMidiMessage(NOTE_ON,note,NOTE_VELOCITY);    
}

void keyNoteOff(int note){
  // //Serial.print("keynoteOff:\t");
  // //Serial.println(note);
  sendMidiMessage(NOTE_OFF,note,0);    
}

void selectSequence(int seqNumber){

  if(seqNumber>N_TRACKS || seqNumber<=0 )return;
  selectedSequence = seqNumber-1;
  // Serial.println("selectSequence");
  // Serial.println(selectedSequence);


  byte selSeq[4] = {0xF0,1,selectedSequence,0xF7};
  MIDI.sendSysEx(4, selSeq,true);
}

void insertStep(int digit){
  digitedStep = digitedStep *10 + digit;
  //validateInsertedStep();
}

void validateInsertedStep(){
  if(digitedStep>0 && digitedStep<=STEP_MAX_SIZE){
    selectedStep = digitedStep-1;
    
    // //Serial.print("SELECTED STEP:\t");
    // //Serial.println(selectedStep);
  }else{
    selectedStep = 0;
    digitedStep = 0;
    // //Serial.println("DIGITED STEP NOT VALID");
  }
  byte selStep[4] = {0xF0,2,selectedStep,0xF7};
  MIDI.sendSysEx(4, selStep,true);
}

void selectNote(int key){
  if(_sequencer[selectedSequence]._sequence[selectedStep].note == getNote(key)){
    ATOMIC(_sequencer[selectedSequence]._sequence[selectedStep].note = 0);
  }else{
   ATOMIC(_sequencer[selectedSequence]._sequence[selectedStep].note = getNote(key));
  }
  sendSelectedSequenceVector();
  // printSequencer();
}

void muteSeq(){
  _sequencer[selectedSequence].mute = !_sequencer[selectedSequence].mute;
}

void playPause(){

  if ( _playing == false ) {
    Serial.println("PLAY");
      // Starts the clock, tick-tac-tick-tac...
    uClock.start();
  } else {
    Serial.println("STOP");
    // stop the clock
    uClock.stop();
  }
}

void clearSequence(){
  for(int i=0;i<STEP_MAX_SIZE;i++){
    _sequencer[selectedSequence]._sequence[i].note = 0;
  }
  sendSelectedSequenceVector();
}

void resetAll(){
  selectedStep = 1;
  selectedSequence = 1;
  digitedStep = 0;
  initSequencerData();
}

void initSequencerData(){
  
  // Set the clock BPM to 126 BPM
  uClock.setTempo(126);
  for(uint16_t t = 0; t < N_TRACKS;t++){
  // initing sequencer data
    for ( uint16_t i = 0; i < STEP_MAX_SIZE; i++ ) {
      _sequencer[t]._sequence[i].note = 0;
      _sequencer[t]._sequence[i].accent = false;
      _sequencer[t]._sequence[i].glide = false;
      _sequencer[t]._sequence[i].rest = false;
    }

    // initing note stack data
    for ( uint8_t i = 0; i < NOTE_STACK_SIZE; i++ ) {
      _note_stack[i].note = 0;
      _note_stack[i].length = -1;
      _note_stack[i].channel =0;
    }
    _sequencer[t].mute =false;
  }
}

void acidRandomize() 
{
  // ramdom it all
  for ( uint16_t i = 0; i < STEP_MAX_SIZE; i++ ) {
    ATOMIC(_sequencer[selectedSequence]._sequence[i].note = random(36, 70)); // octave 2 to 4. octave 3 to 5 (40 - 83)
    ATOMIC(_sequencer[selectedSequence]._sequence[i].accent = random(0, 2));
    ATOMIC(_sequencer[selectedSequence]._sequence[i].glide = random(0, 2));
    ATOMIC(_sequencer[selectedSequence]._sequence[i].rest = random(0, 1));
  }
  sendSelectedSequenceVector();
}

/*-----------------------------------------------*/

/*----------------STATE MACHINE------------------*/
void keyPressedStateTrans(int key){
  switch (key){
    case Rkey:
      switch(state){
        case DEF:
          state = DEF;
          break;
        case RP:
          state = RRP;
          break;
        default:
          return;
      }
      break;
    case RPkey:
      switch(state){
      case DEF:
        state = DEF;
        break;
      case R:
        state = RRP;
        break;
      default:
        return;
    }
    break;
    default:
      return;
  }
}

void keyHoldStateTrans(int key){
  switch (key){
    case Rkey:
      switch(state){
        case DEF:
          state = R;
          break;
        case RP:
          state = RRP;
          break;
        case RRP:
          state = RRP;
          break;
        default:
          return;
      }
      break;
    case RPkey:
       switch(state){
        case DEF:
          state = RP;
          break;
        case R:
          state = RRP;
          break;
        case RRP:
          state = RRP;
          break;
        default:
          return;
      }
      break;
    default:
      return;
  }
}

void keyReleaseStateTrans(int key){
  switch (key){
    case Rkey:
      switch(state){
        case DEF:
          state = DEF;
          transpose++;
          sendTranspose();
          break;
        case R:
          validateInsertedStep();
          state = DEF;
          digitedStep = 0;
          break;
        case RRP:
          state = RP;
          break;
        default:
          return;
      }
      break;
    case RPkey:
      switch(state){
        case DEF:
          state = DEF;
          transpose--;
          sendTranspose();
          break;
        case RP:
          state = DEF;
          break;
        case RRP:
          state = R;
          break;
        default:
          return;
      }
      break;
    default:
      return;
  }
}


void checkStateTransition(int key,KeyState kState){
  if(key != Rkey && key!=RPkey)return;
  //Serial.print("checkStateTransition:\t");
  //Serial.print("key:\t");
  //Serial.print(key);
  //Serial.print("\tkState:\t");
  //Serial.print(kState);
  //Serial.print("\tINITIAL STATE:\t");
  //Serial.println(state);
  switch(kState){
    case PRESSED:
      keyPressedStateTrans(key);
      break;
    case HOLD:
      keyHoldStateTrans(key);
      break;
    case RELEASED:
      keyReleaseStateTrans(key);
    break;
    default:
      return;    
  }


 
  //Serial.print("\tFINAL STATE:\t");
  //Serial.println(state);
}


void handleCurrentState(int key,KeyState kState){
  
  if(state == DEF){
    const int note = getNote(key);
    if(kState == PRESSED) keyNoteOn(note);
    else if(kState == RELEASED) keyNoteOff(note);
    return;
  }
  if(kState != PRESSED)return;
  switch(state){
    case R:
      if(key==STAR) clearSequence();
      else if(key==HASHMRK) muteSeq();
      else{
        insertStep(key);
      }
    break;
    case RP:
      selectNote(key);
    break;
    case RRP:
      if(key==STAR) resetAll();
      else if(key==HASHMRK) playPause();
      else if(key== 9)acidRandomize();
      else{
        selectSequence(key);
      }
    break;
    default:
      return;
  }
}

/*-----------------------------------------------*/
/*--------------------KEYPAD---------------------*/


void readKeyPad() {
  if (kpd.getKeys())  // supports up to ten simultaneous key presses
  {
    for (int i = 0; i < LIST_MAX; i++) // scan the whole key list
    {
      if ( kpd.key[i].stateChanged)   // find keys that have changed state
      {
        const int key = kpd.key[i].kchar -'0';
        const KeyState kState = kpd.key[i].kstate;
        byte keyPress[4] = {0xF0,0,key,0xF7};
        byte keyRel[4] = {0xF0,0,key + 50,0xF7};
        // Serial.print("key:\t");
        // Serial.println(key);
        // Serial.print("kState:\t");
        // Serial.println(kState);
         if(kState ==PRESSED){
            // Serial.println(key);
            MIDI.sendSysEx(4,keyPress,true);
          }else if(kState == RELEASED){
            MIDI.sendSysEx(4,keyRel,true);
          }
        if(key == Rkey || key == RPkey){
          checkStateTransition(key,kState);
          sendState();
        }else{
         
          handleCurrentState(key,kState);
          
        }
      }
    }
  }
}

/*------------------------------------------------*/

/*---------------------GYRO-----------------------*/
void initGyro(){
  Wire.begin();
  byte status = mpu.begin();
  // Serial.print(F("MPU6050 status: "));
  // Serial.println(status);
  // while(status!=0){ } // stop everything if could not connect to MPU6050
  // Serial.println(F("Calculating offsets, do not move MPU6050"));
  // delay(1000);
  // mpu.upsideDownMounting = true; // uncomment this line if the MPU6050 is mounted upside-down
  mpu.calcOffsets(); // gyro and accelero
  // baseValue = mpu.getAngleZ();
}
void sendGyroCC(){
  mpu.update();

  // COMMENT
  // Get sensor value
  // NOTE: X and Y angles return weird values, maybe try different library
  sensorValue = mpu.getAngleZ();
  

  // Clip sensor value if limits are exceeded
  if(sensorValue < sensorMin){
    sensorValue = sensorMin;
  }else if(sensorValue > sensorMax){
    sensorValue = sensorMax;
  }

  // Map value to MIDI range
  mappedValue = map(sensorValue, sensorMin, sensorMax, 0, 127);


  if(mappedValue != previousSensorValue && resetBtnState){
    // Serial.print("mappedValue:\t");
    // Serial.println(mappedValue);
    Serial.print("mappedValue:\t");
    Serial.print(mappedValue);
    Serial.print("\n");
    Serial.print("Sensor value:\t");
    Serial.print(sensorValue);
    Serial.print("\n");
  
    MIDI.sendControlChange(1, mappedValue, 1); // (CC mode, value, MIDI channel); CC mode = 1 corresponds to modulation wheel
    previousSensorValue = mappedValue;
    // Serial.print("Sent new value");
    // Serial.print(mappedValue);
    // Serial.print("\n");
  }
}

void handleResetCoordButton(){
    resetBtnState = digitalRead(resetCoordinatesBtn);
    if(resetBtnState != prevResetBtnState && resetBtnState){
      // Serial.print("resetBtnState:\t");
      // Serial.println(resetBtnState);
      prevResetBtnState = resetBtnState;
      baseValue = mpu.getAngleZ();
    }




}


/*------------------------------------------------*/

void setup() 
{
  
  // Initialize serial communication
  // the default MIDI serial speed communication at 31250 bits per second
  Serial.begin(31250); 
  pinMode(resetCoordinatesBtn, INPUT_PULLUP);
  
  MIDI.begin(); 
  kpd.setHoldTime(200);
  // Inits the clock
  uClock.init();
  
  // Set the callback function for the clock output to send MIDI Sync message.
  uClock.setOnPPQN(onPPQNCallback);
  
  // Set the callback function for the step sequencer on 16ppqn
  uClock.setOnStep(onStepCallback);  
  
  // Set the callback function for MIDI Start and Stop messages.
  uClock.setOnClockStart(onClockStart);  
  uClock.setOnClockStop(onClockStop);

  selectedSeq[0] = 0xF0; 
  selectedSeq[1] = 3;  
  // Set the last element of selectedSeq
  selectedSeq[STEP_MAX_SIZE + 2] = 0xF7;  

  
  initSequencerData();
  initGyro();

  // pins, buttons, leds and pots config
  //configureYourUserInterface();
  
  // start sequencer
  //uClock.start();
}


void loop() {
  // put your main code here, to run repeatedly:
  handleResetCoordButton();
  readKeyPad();
  sendGyroCC();

}
