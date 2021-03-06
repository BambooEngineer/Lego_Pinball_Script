#include <Servo.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <avr/io.h> 
#include <avr/interrupt.h>



#define	PRINT(s, v)	{ Serial.print(F(s)); Serial.print(v); }

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define	MAX_DEVICES	4

#define	CLK_PIN		52  // or SCK
#define	DATA_PIN	51  // or MOSI
#define	CS_PIN		53  // or SS

// SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(CS_PIN, MAX_DEVICES);
// Arbitrary pins
//MD_MAX72XX mx = MD_MAX72XX(DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Text parameters
#define	CHAR_SPACING	1	// pixels between characters

// Global message buffers shared by Serial and Scrolling functions
#define	BUF_SIZE	75
char message[BUF_SIZE] = {"LEGO PINBALL"};
bool newMessageAvailable = true;

#define PRINTS(x)
#define  DELAYTIME  100

/////////////////////////     SCROLLING TEXT
void scrollText(char *p)
{
  uint8_t  charWidth;
  uint8_t cBuf[8];  // this should be ok for all built-in fonts

  PRINTS("\nScrolling text");
  mx.clear();

  while (*p != '\0')
  {
    charWidth = mx.getChar(*p++, sizeof(cBuf)/sizeof(cBuf[0]), cBuf);

    for (uint8_t i=0; i<charWidth + 1; i++) // allow space between characters
    {
      mx.transform(MD_MAX72XX::TSL);
      if (i < charWidth)
        mx.setColumn(0, cBuf[i]);
      delay(DELAYTIME);
    }
  }
}

///////////////

void readSerial(void)
{
  static uint8_t	putIndex = 0;

  while (Serial.available())
  {
    message[putIndex] = (char)Serial.read();
    if ((message[putIndex] == '\n') || (putIndex >= BUF_SIZE-3))	// end of message character or full buffer
    {
      // put in a message separator and end the string
      message[putIndex] = '\0';
      // restart the index for next filling spree and flag we have a message waiting
      putIndex = 0;
      newMessageAvailable = true;
    }
    else
      // Just save the next char in next location
      message[putIndex++];
  }
}

void printText(uint8_t modStart, uint8_t modEnd, char *pMsg)
// Print the text string to the LED matrix modules specified.
// Message area is padded with blank columns after printing.
{
  uint8_t   state = 0;
  uint8_t	  curLen;
  uint16_t  showLen;
  uint8_t	  cBuf[8];
  int16_t   col = ((modEnd + 1) * COL_SIZE) - 1;

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  do     // finite state machine to print the characters in the space available
  {
    switch(state)
    {
      case 0:	// Load the next character from the font table
        // if we reached end of message, reset the message pointer
        if (*pMsg == '\0')
        {
          showLen = col - (modEnd * COL_SIZE);  // padding characters
          state = 2;
          break;
        }

        // retrieve the next character form the font file
        showLen = mx.getChar(*pMsg++, sizeof(cBuf)/sizeof(cBuf[0]), cBuf);
        curLen = 0;
        state++;
        // !! deliberately fall through to next state to start displaying

      case 1:	// display the next part of the character
        mx.setColumn(col--, cBuf[curLen++]);

        // done with font character, now display the space between chars
        if (curLen == showLen)
        {
          showLen = CHAR_SPACING;
          state = 2;
        }
        break;

      case 2: // initialize state for displaying empty columns
        curLen = 0;
        state++;
        // fall through

      case 3:	// display inter-character spacing or end of message padding (blank columns)
        mx.setColumn(col--, 0);
        curLen++;
        if (curLen == showLen)
          state = 0;
        break;

      default:
        col = -1;   // this definitely ends the do loop
    }
  } while (col >= (modStart * COL_SIZE));

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

bool ledstate = true;

bool SUp = false;
bool SDown = false;
const int servo =  9;
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
unsigned long PM = 0;
const long down = 1; 
const long up = 2;
int SVoltage = LOW;

int sensor1 = 0;
int sensor2 = 0;
int score = 0;
char points[75];


Servo myservo;

void setup()
{
  mx.begin();
  myservo.attach(9);
  pinMode(8, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A4, OUTPUT);
  
 
  scrollText(message); // example function

    // had to get rid of timers and interrupts to use servo library to control
    // servo more efficiently 

    //4 second timer interrupt to turn off LEDs and servo movement
    // matrix displays lego pinball logo then displays incrementing score

/* noInterrupts();
  TCCR1A = 0; // clear regs
  TCCR1B = 0;
  TCNT1 = 0; // 4 second timer interrupt to turn off LEDs and servo movement
             // matrix displays lego pinball logo then displays incrementing score
              // 

  OCR1A = 0xF423; 
  TCCR1B|=(1<<WGM12); // CTC Mode for timer
  TCCR1B |= ((1<<CS12)|(1<<CS10));// Clock/1024
  TIMSK1 |= (1 << OCIE1A);    //Set the ISR COMPA vect
  interrupts(); // interrupts enabled
  */
}

void loop() //  
{ // 
  
  
  sensor1 = digitalRead(8); 
  sensor2 = digitalRead(7);
  digitalWrite(A4, LOW); // Synth trigger 
  

  unsigned long currentMillis = millis(); // Millis timers 
  unsigned long CM = millis();

  if(sensor1 == 0 || sensor2 == 0){
    score += 1;
    //requestEvent("HIT");
    digitalWrite(A4, HIGH);
    sprintf(points, "%05d", score);
    printText(0, MAX_DEVICES-1, points);
    ledstate = true;
    SDown = true;
    ledstate != ledstate;
    //Serial.println(score);
  }

  if(ledstate){
    digitalWrite(13, HIGH); 
  }
  else{
    digitalWrite(13, LOW);
  }
  
 
  if(SDown){
    myservo.write(0); // 0 degrees
    if (CM - PM >= 4000) {
      PM = CM;
      SUp = true;
      }
  }
  if(SUp){
    SDown = false;
    myservo.write(90); // 180 degrees
    if (CM - PM >= 50) {
      PM = CM;
      SUp = false;
      ledstate = false;
      }
  }

  // Matrix example test code
 /* readSerial(); 
  if (newMessageAvailable)
  {
    scrollText(message);
    PRINT("\nProcessing new message: ", message);
    printText(0, MAX_DEVICES-1, message);
    newMessageAvailable = false;
  }*/
  
}
 // Servo without library functions 
/*void ServoDown( unsigned long val){ // Goes down
  
    if (val - previousMillis >= down) {
      previousMillis = val;

      if (SVoltage == LOW) {
        SVoltage = HIGH;
      } else {
        SVoltage = LOW;
      }
      
      digitalWrite(servo, SVoltage);
    }
  }


void ServoUp( unsigned long val){ // Goes down
  
    if (val - previousMillis >= up) { // pwm for servo
      previousMillis = val;

      if (SVoltage == LOW) {
        SVoltage = HIGH;
      } else {
        SVoltage = LOW;
      }
      
      digitalWrite(servo, SVoltage);
    }
  }*/
