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
  { 4, 6, 5,10,11, 8, 7, 9},
// AA BB FF DD EE CC GG DP
  {16, 7,15,12,13,10, 9,11}
};

const int digit11pin = 0;
const int digit12pin = 1;
const int digit13pin = 2;
const int digit14pin = 3;
const int digit21pin = 17;
const int digit22pin = 14;
const int digit23pin = 6;
const int digit24pin = 8;

int dig1Min = 0;
int dig1Max = 24;
int dig2Min = 25;
int dig2Max = 49;
int dig3Min = 50;
int dig3Max = 74;
int dig4Min = 75;
int dig4Max = 100;

long ticks = 0;
int n = 0;
const byte divide = 1;
byte reset = 100;
int num;
int value0;                    // The number displayed on the LEDs.

Bounce button0 = Bounce(A4, 10);
Bounce button1 = Bounce(A5, 10);  // 10 ms debounce time is appropriate

unsigned long startTime = 0;
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
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);

  //Set all the LED pins as output.
  for (byte pinCount = 0; pinCount < 8; pinCount++) {
    pinMode(sevenSegPins[0][pinCount], OUTPUT);
   // pinMode(sevenSegPins[1][pinCount], OUTPUT);
  }
  
  // Common pins for the 4 digits.
  pinMode(digit11pin, OUTPUT);
  pinMode(digit12pin, OUTPUT);
  pinMode(digit13pin, OUTPUT);
  pinMode(digit14pin, OUTPUT);  
  //pinMode(digit21pin, OUTPUT);
  //pinMode(digit22pin, OUTPUT);
  //pinMode(digit23pin, OUTPUT);
  //pinMode(digit24pin, OUTPUT);

  // Start with all 'dots' off.
  digitalWrite(sevenSegPins[0][7], 1);
  //digitalWrite(sevenSegPins[1][7], 1);    

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  Serial.begin(115200);

  // To avoid jump at first connect.
  sliderStored0 = sliderValue0 = analogRead(A0);
  sliderStored1 = sliderValue1 = analogRead(A1);

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
  digitalWrite(sevenSegPins[displayid][7], 1);
}

void loop() {
  // Always increase the ticks counter.
  ticks++;
  
  // subtract the last reading:
  total0= total0 - readings0[index];         
  total1= total1 - readings1[index];         

  // read from the sensor:  
  readings0[index] = analogRead(A0); 
  readings1[index] = analogRead(A1); 

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
  button1.update();

  if (button0.fallingEdge()) {
    // start & stop
    if (running) {
      running = 0;
    }
    else {
      running = 1;
      lastUpdateTime = millis();
    }
  }
  if (button1.fallingEdge()) {
    money = 0;
  }


  if (sliderValue0 - sliderStored0 >= sliderThreshold || sliderStored0 - sliderValue0 >= sliderThreshold) {
    Serial.println(sliderValue0);  
    sliderStored0 = sliderValue0;
    people = map(sliderStored0, 0, 1023, 1, 12);
  }    
  if (sliderValue1 - sliderStored1 >= sliderThreshold || sliderStored1 - sliderValue1 >= sliderThreshold) {
    Serial.println(sliderValue1);  
    sliderStored1 = sliderValue1;
    rate = map(sliderStored1, 0, 1023, 10, 1000)/10*10*100;
  } 
  
  if (running) {
   money = money + (people * rate * (millis() - lastUpdateTime)/3600);
   lastUpdateTime = millis();
   Serial.print(people);
   Serial.print(' ');
   Serial.print(rate);
   Serial.print(' ');
   Serial.print(lastUpdateTime);
   Serial.print(' ');
   Serial.print(millis());
   Serial.print(' ');
   Serial.print(millis()/1000/60);
   Serial.print(' ');    
   Serial.print(money/1000/100);
   Serial.print(',');        
   if (money/1000%100 <10) {
     Serial.print('0');
   }
   Serial.println(money/1000%100);
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
    //sevenSegWrite(1, value1 % 10000 / 1000, 1);    
    if (value0 >= 1000) {    
      digitalWrite(digit11pin,1);
    }
//    if (value1 >= 1000) {
//      digitalWrite(digit21pin,1);    
//    }
  }

  // 2nd significant digit
  else if(n >= dig2Min && n <= dig2Max) {
    digitalWrite(digit11pin,0);
    digitalWrite(digit13pin,0);
    digitalWrite(digit14pin,0);
    digitalWrite(digit21pin,0);
    digitalWrite(digit23pin,0);
    digitalWrite(digit24pin,0);    
    sevenSegWrite(0, value0 % 1000 / 100, 1);
//    sevenSegWrite(1, value1 % 1000 / 100, 1);    
    if (value0 >= 100) {
      digitalWrite(digit12pin,1);
    }
//    if (value1 >= 100) {    
//      digitalWrite(digit22pin,1);    
//    }
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
//    sevenSegWrite(1, value1 % 100 / 10, 1);
    if (value0 >= 10) {
      digitalWrite(digit13pin,1);
    }
//    if (value1 >= 10) {
//      digitalWrite(digit23pin,1);    
//    }
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
//    sevenSegWrite(1, value1 % 10, 1);
    digitalWrite(digit14pin,1);        
    digitalWrite(digit24pin,1);            
  } 

  // The rest of the time the leds are off, saves 10 mA power.
  else {
    digitalWrite(digit11pin,0);
    digitalWrite(digit12pin,0);
    digitalWrite(digit13pin,0);     
    digitalWrite(digit14pin,0);
  }

  // Ticks won't be bigger than reset.
  if(ticks > reset) {
    ticks = -1;
    num++;
  }

  // Every (10 * ticks) loops the value we work with is updated.
  if(num > 10) {
    num = 0;
    value0 = money/1000;
//    value1 =  / 1000;    
  }
 // delay(1);
}


