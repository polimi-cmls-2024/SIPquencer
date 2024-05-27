import oscP5.*;
import netP5.*;

float noteNumber = 0;

boolean noteOnReceived = false;
int bgColor = 0;
int frameCountNoteOn = 0;
OscP5 oscP5;
float angle;
NetAddress sc;
int[][] channelParameters = new int[4][3];
int reverb=0;
int delay=0;
float blurFactor = 0;

float channelnote1;
float channelnote2;
float channelnote3;
float channelnote4;
float rColor[]= {255,255,255,255};
float gColor[]= {255,255,255,255};
float bColor[]= {255,255,255,255};

void setup() {  
  
  channelnote1 = 0.0;
  channelnote2 = 0.0;
  channelnote3 = 0.0;
  channelnote4 = 0.0;
  
  size(1000, 1000);
  oscP5 = new OscP5(this, 12000); // Start oscP5, listening for incoming messages at port 12000
  sc = new NetAddress("127.0.0.1", 57120); // SuperCollider's default port
  noFill();

  background(bgColor);
  stroke(255);
  strokeWeight(2);

  
}


void oscEvent(OscMessage theOscMessage) {
  /* print the address pattern and the typetag of the received OscMessage */
  print("### received an osc message.");
  print(" addrpattern: "+theOscMessage.addrPattern());
  println(" typetag: "+theOscMessage.typetag());
  

  if (theOscMessage.addrPattern().equals("/NoteOn")) {
    println("Received NoteOn message");

    // Extract the arguments from the OSC message
    int noteNumber = theOscMessage.get(0).intValue(); // Assuming the first argument is the note number
    int channel = theOscMessage.get(2).intValue();    // Assuming the third argument is the channel
    
    if (channel == 1) {
        channelnote1 = noteNumber;
      } else if (channel == 2) {
        channelnote2 = noteNumber;
      } else if (channel == 3) {
        channelnote3 = noteNumber;
      } else if (channel == 4) {
        channelnote4 = noteNumber;
      }

    
    // Print the received note information
    println("Note Number: " + noteNumber);
    println("Channel: " + channel);

    // Update the flags and frame count
    noteOnReceived = true;
    frameCountNoteOn = frameCount; // Record the frame when NoteOn was received
    }
    
  if (theOscMessage.addrPattern().equals("/Controls")) {
    println("Received Control message");
  
    // Extract the arguments from the OSC message
    int controllerNumber = theOscMessage.get(0).intValue(); // Assuming the first argument is the controller number
    int controllerValue = theOscMessage.get(1).intValue();  // Assuming the second argument is the controller value
    int channel = theOscMessage.get(2).intValue();          // Assuming the third argument is the channel
    if(controllerNumber<4){
    channelParameters[channel-1][controllerNumber-1] = controllerValue;
    }else if(controllerNumber==4){
      reverb=controllerValue;
    }else if(controllerNumber==5){
      delay=controllerValue;
    }
  
    // Print the received control information
    println("Controller Number: " + controllerNumber);
    println("Controller Value: " + controllerValue);
    println("Channel: " + channel);
  
    // Handle the control message as needed
    // (Add your custom handling logic here)
}

}


int closestEven(float num) {
  int intNum = round(num);
  
  // Check if the integer is even
  if (intNum % 2 == 0) {
    return intNum; // If it's even, return it
  } else {
    // If it's odd, return the closest even number
    if (abs(num - intNum) > 0.5) {
      return intNum + 1; // If float is closer to the next integer, return the next even number
    } else {
      return intNum - 1; // If float is closer to the previous integer, return the previous even number
    }
  }
}


void draw() {
  // Change background color if noteOn is received and revert back after the duration
  
  background(bgColor);
  translate(width/2, height/2);
  
  
    beginShape();
    
  for (float theta = 0; theta < TWO_PI; theta += 0.01) {
    float rad = r(theta,
       (channelnote1)/100 , // a -  cutoff
       (channelnote1)/100, // b - release
       closestEven(round(delay)), // m
       closestEven(round(round(channelParameters[0][2]/20)))+20, // n1
       closestEven(round(round(channelParameters[0][1]/20)))+20, // n2
       channelParameters[0][0] // n3
    ); 
    
    
    
   // println("rColor: " + rColor + ", gColor: " + gColor + ", bColor: " + bColor);

    
    stroke(rColor[0], gColor[0], bColor[0]);
    
    
    float g = rad * cos(theta) * 400;
    float j = rad * sin(theta) * 400;
    vertex(g, j);
    for (int i = 1; i <= 5; i++) {
    blurFactor = ((reverb+3)/3) * i; // Adjust the blur factor as needed
    float blurX = g + random(-blurFactor, blurFactor);
    float blurY = j + random(-blurFactor, blurFactor);
    vertex(blurX, blurY);
    rColor[0] = map(channelParameters[0][0], 0, 127, 0, 255);
    gColor[0] = map(channelParameters[0][1], 0, 127, 0, 255);
    bColor[0] = map(channelParameters[0][2], 0, 127, 0, 255);
  }
  }
  endShape();
  
  beginShape();
    
  for (float theta = 0; theta < TWO_PI; theta += 0.01) {
    float rad = r(theta,
       (channelnote2)/100 , // a -  cutoff
       (channelnote2)/100, // b - release
       closestEven(round(delay)), // m
       closestEven(round(round(channelParameters[1][2]/20)))+20, // n1
       closestEven(round(round(channelParameters[1][2]/20)))+20, // n2
       channelParameters[2][2] // n3
    ); 
    
    
    
   // println("rColor: " + rColor + ", gColor: " + gColor + ", bColor: " + bColor);

    
    stroke(rColor[1], gColor[1], bColor[1]);
    
    
    float g = rad * cos(theta) * 600;
    float j = rad * sin(theta) * 600;
    vertex(g, j);
    for (int i = 1; i <= 5; i++) {
    blurFactor = ((reverb+3)/3) * i; // Adjust the blur factor as needed
    float blurX = g + random(-blurFactor, blurFactor);
    float blurY = j + random(-blurFactor, blurFactor);
    vertex(blurX, blurY);
    rColor[1] = map(channelParameters[1][0], 0, 127, 0, 255);
    gColor[1] = map(channelParameters[1][1], 0, 127, 0, 255);
    bColor[1] = map(channelParameters[1][2], 0, 127, 0, 255);
  }
  }
  endShape();
  
   beginShape();
    
  for (float theta = 0; theta < TWO_PI; theta += 0.01) {
    float rad = r(theta,
       (channelnote3)/100 , // a -  cutoff
       (channelnote3)/100, // b - release
       closestEven(round(delay)), // m
       closestEven(round(round(channelParameters[2][2]/20)))+20, // n1
       closestEven(round(round(channelParameters[2][1]/20)))+20, // n2
       channelParameters[2][1] // n3
    ); 
    
    
    
   // println("rColor: " + rColor + ", gColor: " + gColor + ", bColor: " + bColor);

    
    stroke(rColor[2], gColor[2], bColor[2]);
    
    
    float g = rad * cos(theta) * 800;
    float j = rad * sin(theta) * 800;
    vertex(g, j);
    for (int i = 1; i <= 5; i++) {
    blurFactor = ((reverb+3)/3) * i; // Adjust the blur factor as needed
    float blurX = g + random(-blurFactor, blurFactor);
    float blurY = j + random(-blurFactor, blurFactor);
    vertex(blurX, blurY);
    rColor[2] = map(channelParameters[2][0], 0, 127, 0, 255);
    gColor[2] = map(channelParameters[2][1], 0, 127, 0, 255);
    bColor[2] = map(channelParameters[2][2], 0, 127, 0, 255);
  }
  }
  endShape();
  
 
}

float r(float theta, float a, float b, float m, float n1, float n2, float n3) {
  return pow(pow(abs(cos(m * theta / 4.0) / a), n2) + 
             pow(abs(sin(m * theta / 4.0) / b), n3), -1.0 / n1);
}
