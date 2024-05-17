// KEYPAD //
#include <Keypad.h>
#include <Midier.h>


const byte ROWS = 4; // four rows
const byte COLS = 4; // four columns
char keys[ROWS][COLS] = {  // keypad keys, 1-9, 0, S for star (asterisk) and P for pound (square)
  {'1', '2', '3','A'},
  {'4', '5', '6','B'},
  {'7', '8', '9','C'},
  {':', '0', ';','D'}
};

int notes[] ={0,7,2,9,4,11,6,1,8,3,10,5};  // order the notes in a way that first element corresponds to key 0 , second -> key 1 ecc... (':' = 10 , ';' = 11)

byte rowPins[ROWS] = {4,5,6,7}; // keypad row pinouts
byte colPins[COLS] = {8,9,10,11}; // keypad column pinouts
const int buttonPin = 2;  // the number of the pushbutton pin
volatile byte buttonState = LOW; // variable for reading the pushbutton status (pulsante cornetta)

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
enum {DEF, R, RP, RRP}; //side buttons states (RRP = simultaneous pressure of R and RP, DEF= nessuno dei due é premuto)
unsigned char sideButtonState;  


//----States for synth and sequencer-----
enum{SYNTH, SEQ};
unsigned char mode;


int midC = 60; // MIDI note value for middle C on a standard keyboard
int transpose = 0;  // if = 0 no transposing

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
int pitchBendMIDI = 224;  // pitch bend channel 1 MIDI code


int progChg = 192;
/*
  The MIDI message consists of three bytes, first setting the status
  of the message, and the following bytes denoting the action, which
  note number, velocity, pressure etc. The data bytes are different
  depending on the type of the message set by the status byte.

  Refer to the following chart for which bytes do what:

  https://www.midi.org/specifications-old/item/table-2-expanded-messages-list-status-bytes
*/

//-------ACCELEROMETER-GYRO---------//
#include "I2Cdev.h"

#include "MPU6050_6Axis_MotionApps20.h"
//#include "MPU6050.h" // not necessary if using MotionApps include file

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

MPU6050 mpu;

// uncomment "OUTPUT_READABLE_QUATERNION" if you want to see the actual
// quaternion components in a [w, x, y, z] format (not best for parsing
// on a remote host such as Processing or something though)
//#define OUTPUT_READABLE_QUATERNION

// uncomment "OUTPUT_READABLE_YAWPITCHROLL" if you want to see the yaw/
// pitch/roll angles (in degrees) calculated from the quaternions coming
// from the FIFO. Note this also requires gravity vector calculations.
// Also note that yaw/pitch/roll angles suffer from gimbal lock (for
// more info, see: http://en.wikipedia.org/wiki/Gimbal_lock)
 #define OUTPUT_READABLE_YAWPITCHROLL


// // MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer


// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

  volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
  void dmpDataReady() {
      mpuInterrupt = true;
  }


  // orientation/motion vars
  Quaternion q;           // [w, x, y, z]         quaternion container
  VectorFloat gravity;    // [x, y, z]            gravity vector
  float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector




// #include "Wire.h"  // This library allows you to communicate with I2C devices.
// const int MPU_ADDR = 0x68;  // I2C address of the MPU-6050

// int16_t value[7];
// const int chan = 1;
// const int cc_offset = 0;







void setup() {
     Serial.begin(31250); 

    setupGyro();
    pinMode(buttonPin, INPUT_PULLUP);
   // attachInterrupt(digitalPinToInterrupt(buttonPin), handleButton, FALLING );

} 


void loop() {
// read the state of the pushbutton value:
  readKeyPad();       // read keypad + transpose switch
  readGyroData();
}


void setupGyro(){

    // Wire.begin();
    // Wire.beginTransmission(MPU_ADDR);  // Begins a transmission to the I2C slave (GY-521 board)
    // Wire.write(0x6B);                  // PWR_MGMT_1 register
    // Wire.write(0);                     // set to zero (wakes up the MPU-6050)
    // Wire.endTransmission(true);
 // join I2C bus (I2Cdev library doesn't do this automatically)
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
      Wire.begin();
      Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
      Fastwire::setup(400, true);
  #endif
  while (!Serial);

  //   // initialize device
  //   //Serial.println(F("Initializing I2C devices..."));
  mpu.initialize();
  //   //pinMode(INTERRUPT_PIN, INPUT);


  //  // verify connection
      //Serial.println(F("Testing device connections..."));
      //Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
  //     // wait for ready
  //   //  Serial.println(F("\nSend any character to begin DMP programming and demo: "));
  //   //  while (Serial.available() && Serial.read()); // empty buffer
  //   //  while (!Serial.available());                 // wait for data
  //   //  while (Serial.available() && Serial.read()); // empty buffer again

  //   // load and configure the DMP
  //   //Serial.println(F("Initializing DMP..."));
   devStatus = mpu.dmpInitialize();


   // supply your own gyro offsets here, scaled for min sensitivity
     mpu.setXGyroOffset(220);
     mpu.setYGyroOffset(300);
     mpu.setZGyroOffset(-85);
  //   mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

  //     // make sure it worked (returns 0 if so)
  if (devStatus == 0) {
      // Calibration Time: generate offsets and calibrate our MPU6050
      mpu.CalibrateAccel(6);
      mpu.CalibrateGyro(6);
      mpu.PrintActiveOffsets();
      // turn on the DMP, now that it's ready
      //Serial.println(F("Enabling DMP..."));
      mpu.setDMPEnabled(true);

  //       // enable Arduino interrupt detection
  //       //Serial.print(F("Enabling interrupt detection (Arduino external interrupt "));
  //       //Serial.print(digitalPinToInterrupt(INTERRUPT_PIN));
  //       //Serial.println(F(")..."));
  //       //attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
  //       //mpuIntStatus = mpu.getIntStatus();

  //       // set our DMP Ready flag so the main loop() function knows it's okay to use it
  //      // Serial.println(F("DMP ready! Waiting for first interrupt..."));
      dmpReady = true;

      // get expected DMP packet size for later comparison
      packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
      // ERROR!
      // 1 = initial memory load failed
      // 2 = DMP configuration updates failed
      // (if it's going to break, usually the code will be 1)
      Serial.print(F("DMP Initialization failed (code "));
      Serial.print(devStatus);
      Serial.println(F(")"));
  }

}


void readGyroData(){
    // Wire.beginTransmission(MPU_ADDR);
    // Wire.write(0x3B);                         // starting with register 0x3B 
    // Wire.endTransmission(false);              // active connection
    // Wire.requestFrom(MPU_ADDR, 7 * 2, true);  // request a total of 7*2=14 registers

    // for (int i = 0; i < 7; i++) {
    //   int16_t data = Wire.read() << 8 | Wire.read(); 
    //   data = constrain(map(data, -17000, 17000, 0, 127), 0, 127);

    //   if (data != value[i]) {
    //     value[i] = data;
    //     // MIDImessage(pitchBendMIDI, data, chan);
    //     Serial.print("data:\t");
    //     Serial.println(data);
    //     delay(300);
    //   }
    // }
   // if programming failed, don't try to do anything
    if (!dmpReady) return;
    // // read a packet from FIFO
    if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) { // Get the Latest packet 
     #ifdef OUTPUT_READABLE_QUATERNION
            // display quaternion values in easy matrix form: w x y z
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            Serial.print("quat\t");
            Serial.print(q.w);
            Serial.print("\t");
            Serial.print(q.x);
            Serial.print("\t");
            Serial.print(q.y);
            Serial.print("\t");
            Serial.println(q.z);
        #endif
     #ifdef OUTPUT_READABLE_YAWPITCHROLL
            // display Euler angles in degrees
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
            //Serial.print("gyroMIDIy:\t");   
            //Serial.println(ypr[1] *(180/M_PI));  
          
            //MIDImessage(pitchBendMIDI, constrain(map(ypr[1]* 180/M_PI, -256, 256, 0, 127), 0, 127) ,0);
            //delay(333);
              // Serial.print("ypr\t");
              //Serial.print(ypr[0] * 180/M_PI);
              //Serial.print("\t");
              //Serial.print(ypr[1] * 180/M_PI);
              //Serial.print("\t");
              //            Serial.println(ypr[2] * 180/M_PI);
        #endif
    }
}




void readKeyPad() {

  // KEYPAD //

  if (kpd.getKeys())  // supports up to ten simultaneous key presses
  {
    for (int i = 0; i < LIST_MAX; i++) // scan the whole key list
    {
      if ( kpd.key[i].stateChanged)   // find keys that have changed state
      {
          const int key = kpd.key[i].kchar -'0';
          const int note = notes[key];
        if (kpd.key[i].kstate == PRESSED) // sends MIDI on message when keys are pressed
        {
            //Serial.print("pressed key: ");
            //Serial.println(key);
            //Serial.print("pressed note: ");
            //Serial.println(note);
            //Serial.print("State: ");
            //Serial.println(sideButtonState);
         
            switch (key){
              case 17:
                if(sideButtonState == RP){
                  sideButtonState = RRP;
                  //Serial.println("sideButtonState changed to RRP by keypress ");
                   // Serial.println(key);

                }else{
                  sideButtonState = R;
                  //Serial.println("sideButtonState changed to R by keypress ");
                  //  Serial.println(key);

                }
                break;
              case 18:
                if(sideButtonState == R){
                  sideButtonState = RRP;
                  //Serial.println("sideButtonState changed to RRP by keypress ");
                  //  Serial.println(key);

                }else{
                  sideButtonState = RP;
                 // Serial.println("sideButtonState changed to RP by keypress ");
                  //  Serial.println(key);
                }
                break;
              case 10:
                switch(sideButtonState){
                  case R:
                    transpose--;
                   // Serial.println("octave down");
                    // MIDIProg(progChg,transpose);
                    break;
                  case RP:
                   // Serial.println("sequence pos down");
                    break;  
                  default:
                    break;
                }
                break;            
              case 11 :
               switch(sideButtonState){
                  case R:
                    transpose++;
                    //Serial.println("octave up");
                    // MIDIProg(progChg,transpose);
                    break;
                  case RP:
                    //Serial.println("sequence pos up");
                    break;  
                  default:
                    break;
                }
                break;   
              default :
                 if(note>=0 && note<=11) {
                  MIDImessage(kpc, midC + (transpose *12) + note, 127);
                  //Serial.print("Send note ");
                 // Serial.println(note);


                }else{
                  Serial.println("key not expected");

                 }
          }
        }

        if (kpd.key[i].kstate == RELEASED)  // sends MIDI off message when keys are released
        {
        //  Serial.print("released key: ");
        //  Serial.println(key);
        //  Serial.print("released note: ");
        //  Serial.println(note);
        //  Serial.print("State: ");
        //  Serial.println(sideButtonState);
          if(note>=0 && note<=11) {
            MIDImessage(kpc -16, midC + (transpose *12) + note, 127);
          }else{
            switch (key){
              case 17:
                switch (sideButtonState){
                  case R:
                    sideButtonState = DEF;
                    //Serial.print("sideButtonState changed to DEF by keyrelease ");
                    //Serial.println(key);
                    break;
                  case RRP:
                    sideButtonState = RP;
                   // Serial.println("sideButtonState changed to RP by keyrelease ");
                   // Serial.println(key);
                    break;
                  default:
                   // Serial.println(" default sideButtontate");
                   // Serial.println(sideButtonState);
                  break;
                }
              case 18:
                 switch (sideButtonState){
                  case RP:
                    sideButtonState = DEF;
                   // Serial.println("sideButtonState changed to DEF by keyrelease ");
                   // Serial.println(key);

                    break;
                  case RRP:
                    sideButtonState = R;
                   // Serial.println("sideButtonState changed to R by keyrelease ");
                   // Serial.println(key);
                    break;
                  default:
                    //Serial.println(" default sideButtontate");
                    //Serial.println(sideButtonState);
                    break;
                 }
                break;
              default:
                 Serial.println("key not expected");

            }
          }
        }
      }
    }
  }
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
      Serial.println("release");
    //MIDImessage(kpc -16, midC + (transpose *12), 127);
}

void MIDImessage(byte status, byte data1, byte data2)
{
 // Serial.println("MIDIMessage:");
  Serial.write(status);

 // Serial.println(status);
  Serial.write(data1);
 // Serial.println(data1);

  Serial.write(data2);
 // Serial.println(data2);

}


void MIDIProg(byte status, byte data ) {
  Serial.write(status);
  Serial.write(data);
}
