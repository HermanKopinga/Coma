#include <Bounce.h>

// A 3D array :-D
const byte sevenSegDigits[2][11][7] = {
{
  { 0,0,0,0,0,0,1 },  // = 0
  { 1,0,1,1,1,0,1 },  // = 1
  { 0,0,1,0,0,1,0 },  // = 2
  { 0,0,1,0,1,0,0 },  // = 3
  { 1,0,0,1,1,0,0 },  // = 4
  { 0,1,0,0,1,0,0 },  // = 5
  { 0,1,0,0,0,0,0 },  // = 6
  { 0,0,1,1,1,0,1 },  // = 7
  { 0,0,0,0,0,0,0 },  // = 8
  { 0,0,0,0,1,0,0 },  // = 9
  { 1,1,1,1,1,1,1 } // = space
}, {
  { 0,0,0,0,0,0,1 },  // = 0
  { 1,0,1,1,1,0,1 },  // = 1
  { 0,0,1,0,0,1,0 },  // = 2
  { 0,0,1,0,1,0,0 },  // = 3
  { 1,0,0,1,1,0,0 },  // = 4
  { 0,1,0,0,1,0,0 },  // = 5
  { 0,1,0,0,0,0,0 },  // = 6
  { 0,0,1,1,1,0,1 },  // = 7
  { 0,0,0,0,0,0,0 },  // = 8
  { 0,0,0,0,1,0,0 },  // = 9
  { 1,1,1,1,1,1,1 } // = space
}
};

/*
 aa
f  b
 gg
e  c
 dd dp
 */

const byte sevenSegPins[2][8] = {
// AA BB FF DD EE CC GG DP
  {11, 7,10, 2, 1, 4, 5, 3},
// AA BB FF DD EE CC GG DP
  {22,18,21,39,38,41,42,40}
};

const int digit11pin = 12;
const int digit12pin = 9;
const int digit13pin = 8;
const int digit14pin = 6;
const int digit21pin = 23;
const int digit22pin = 20;
const int digit23pin = 19;
const int digit24pin = 43;

int dig1Min = 0;
int dig1Max = 4;
int dig2Min = 5;
int dig2Max = 9;
int dig3Min = 10;
int dig3Max = 14;
int dig4Min = 15;
int dig4Max = 20;

long ticks = 0;
int n = 0;
const byte divide = 1;
byte reset = 20;
int num;
long value0;                    // The number displayed on the LEDs.

Bounce button0 = Bounce(0, 10);

unsigned long  timeOfPress = 0;
bool debug = 0;
bool ignoreThisPress = 0;

unsigned long startTime = 0;
unsigned long lastTouch = 0;
long money = 0;
long totalCostOfMeeting = 0;
unsigned long lastUpdateTime = 0;
long people = 0;
long rate = 0;
boolean running = 0;

// Record keeping global variables.
const int sliderThreshold = 5;
int sliderValue0 = 0;
int sliderStored0 = 0;
int sliderValue1 = 0;
int sliderStored1 = 0;

const int numReadings = 10;
int readings0[numReadings];      // the readings from the analog input
int readings1[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
int total0 = 0;                  // the running total
int average0 = 0;                // the average
int total1 = 0;                  // the running total
int average1 = 0;                // the average

void setup() {
  // Configure the pins for input mode with pullup resistors.
  // The pushbuttons connect from each pin to ground.  When
  // the button is pressed, the pin reads LOW because the button
  // shorts it to ground.  When released, the pin reads HIGH
  // because the pullup resistor connects to +5 volts inside
  // the chip.  
  pinMode(0, INPUT_PULLUP);

  //Set all the LED pins as output.
  for (byte pinCount = 0; pinCount < 8; pinCount++) {
    pinMode(sevenSegPins[0][pinCount], OUTPUT);
    pinMode(sevenSegPins[1][pinCount], OUTPUT);
  }
  
  // Common pins for the 4 digits.
  pinMode(digit11pin, OUTPUT);
  pinMode(digit12pin, OUTPUT);
  pinMode(digit13pin, OUTPUT);
  pinMode(digit14pin, OUTPUT);  
  pinMode(digit21pin, OUTPUT);
  pinMode(digit22pin, OUTPUT);
  pinMode(digit23pin, OUTPUT);
  pinMode(digit24pin, OUTPUT);

  // Start with all 'dots' off.
  digitalWrite(sevenSegPins[0][7], 1);
  digitalWrite(sevenSegPins[1][7], 1);    

  pinMode(A6, INPUT);
  pinMode(A7, INPUT);

  Serial.begin(115200);

  // To avoid jump at first connect.
  sliderStored0 = sliderValue0 = analogRead(A6);
  sliderStored1 = sliderValue1 = analogRead(A7);

  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings0[thisReading] = 0;        
    readings1[thisReading] = 0;        
  }
}

// Function that writes the passed digit to the output pins. 
// Depending on which cathode is grounded another digit is lit.
void sevenSegWrite(byte displayid, byte digit, byte dot)  {
  for (byte loopCount = 0; loopCount < 8; ++loopCount) {
    digitalWrite(sevenSegPins[displayid][loopCount], sevenSegDigits[displayid][digit][loopCount]);
  }

  // Dots OFF.
  if (millis() - lastTouch <= 2000) {
    dot = 1;
  }
  digitalWrite(sevenSegPins[displayid][7], dot);
}

void loop() {
  // Always increase the ticks counter.
  ticks++;
  
  // subtract the last reading:
  total0= total0 - readings0[index];         
  total1= total1 - readings1[index];         

  // read from the sensor:  
  readings0[index] = analogRead(A6); 
  readings1[index] = analogRead(A7); 

  // add the reading to the total:
  total0= total0 + readings0[index];       
  total1= total1 + readings1[index];       
  // advance to the next position in the array:  
  index = index + 1;                    

  // if we're at the end of the array...
  if (index >= numReadings) {      
    // ...wrap around to the beginning: 
    index = 0;              
  }    

  // calculate the average:
  sliderValue0 = total0 / numReadings;        
  sliderValue1 = total1 / numReadings;        

  // Update all the buttons.  There should not be any long
  // delays in loop(), so this runs repetitively at a rate
  // faster than the buttons could be pressed and released.
  button0.update();
  
  if (button0.fallingEdge()) {
    timeOfPress = millis();
    ignoreThisPress = 0;
  }
  
  if (button0.read() == LOW && millis() - timeOfPress > 1200) {
      money = 0;  
      running = 0;      
      ignoreThisPress = 1;
  }
  
  if (button0.risingEdge() && !ignoreThisPress) {
    // start & stop
    if (running) {
      running = 0;
    }
    else {
      running = 1;
      lastUpdateTime = millis();
    }
  }

  if (sliderValue0 - sliderStored0 >= sliderThreshold || sliderStored0 - sliderValue0 >= sliderThreshold) {
    if (debug) Serial.println(sliderValue0);  
    sliderStored0 = sliderValue0;
    people = map(sliderStored0, 0, 1023, 1, 12);
    value0 = people;
    lastTouch = millis();
  }    
  if (sliderValue1 - sliderStored1 >= sliderThreshold || sliderStored1 - sliderValue1 >= sliderThreshold) {
    if (debug) Serial.println(sliderValue1);  
    sliderStored1 = sliderValue1;
    rate = map(sliderStored1, 0, 1023, 10, 1000)/10*10*100;
    value0 = rate/100;
    lastTouch = millis();    
  } 
  
  if (running) {
   money = money + (people * rate * (millis() - lastUpdateTime)/3600);
   lastUpdateTime = millis();
   if (debug) Serial.print(people);
   if (debug) Serial.print(' ');
   if (debug) Serial.print(rate);
   if (debug) Serial.print(' ');
   if (debug) Serial.print(lastUpdateTime);
   if (debug) Serial.print(' ');
   if (debug) Serial.print(millis());
   if (debug) Serial.print(' ');
   if (debug) Serial.print(millis()/1000/60);
   if (debug) Serial.print(' ');    
   if (debug) Serial.print(money/1000/100);
   if (debug) Serial.print(',');        
   if (money/1000%100 <10) {
     if (debug) Serial.print('0');
   }
   if (debug) Serial.println(money/1000%100);
  } 
  ////////////// 
  // (Re)Write the digits

  // Write a digit at a time for 20 ticks.
  n = ticks/divide;

  // Most significant digit
  if(n >= dig1Min && n <= dig1Max) {
    digitalWrite(digit12pin,0);
    digitalWrite(digit13pin,0);
    digitalWrite(digit14pin,0);
    digitalWrite(digit22pin,0);
    digitalWrite(digit23pin,0);
    digitalWrite(digit24pin,0);    
    sevenSegWrite(0, value0 % 10000 / 1000, 1);
    sevenSegWrite(1, value0 % 100000000 / 10000000, 1);    
    if (value0 >= 1000) {    
      digitalWrite(digit11pin,1);
    }
    if (value0 >= 10000000) {
      digitalWrite(digit21pin,1);    
    }
  }

  // 2nd significant digit
  else if(n >= dig2Min && n <= dig2Max) {
    digitalWrite(digit11pin,0);
    digitalWrite(digit13pin,0);
    digitalWrite(digit14pin,0);
    digitalWrite(digit21pin,0);
    digitalWrite(digit23pin,0);
    digitalWrite(digit24pin,0);    
    sevenSegWrite(0, value0 % 1000 / 100, 0);
    sevenSegWrite(1, value0 % 10000000 / 1000000, 1);    
    digitalWrite(digit12pin,1);
    if (value0 >= 1000000) {    
      digitalWrite(digit22pin,1);    
    }
  }

  // Third significant digit  
  else if(n >= dig3Min && n <= dig3Max) {
    digitalWrite(digit11pin,0);
    digitalWrite(digit12pin,0);
    digitalWrite(digit14pin,0);
    digitalWrite(digit21pin,0);
    digitalWrite(digit22pin,0);
    digitalWrite(digit24pin,0);    
    sevenSegWrite(0, value0 % 100 / 10, 1);
    sevenSegWrite(1, value0 % 1000000 / 100000, 1);
    digitalWrite(digit13pin,1);
    if (value0 >= 100000) {
      digitalWrite(digit23pin,1);    
    }
  }

  // Least significant digit  
  else if(n >= dig4Min && n <= dig4Max) {
    digitalWrite(digit11pin,0);
    digitalWrite(digit12pin,0);
    digitalWrite(digit13pin,0);    
    digitalWrite(digit21pin,0);
    digitalWrite(digit22pin,0);
    digitalWrite(digit23pin,0);        
    sevenSegWrite(0, value0 % 10, 1);
    sevenSegWrite(1, value0 % 100000 / 10000, 1);
    digitalWrite(digit14pin,1);        
    if (value0 >= 10000) {
      digitalWrite(digit24pin,1);                
    }   
  } 

  // Ticks won't be bigger than reset.
  if(ticks > reset) {
    ticks = -1;
    if (millis() - lastTouch > 2000) {
      value0 = money/1000;
    }
  }

 
}


