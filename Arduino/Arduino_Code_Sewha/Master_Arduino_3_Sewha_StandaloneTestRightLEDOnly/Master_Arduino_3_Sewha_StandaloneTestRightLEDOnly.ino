/////////////////////////////////////////////////////////
//SPI Tutorial: https://learn.sparkfun.com/tutorials/serial-peripheral-interface-spi/all
// https://forum.arduino.cc/index.php?topic=558963.0
// Arduino UNO
// 10 (SS)
// 11 (MOSI)
// 12 (MISO)
// 13 (SCK)
//
// +5v(if required)
// GND(for signal return)
//
// Arduino Mega (master)  - Arduino Mega (slave)
// 53 (SS)              -- 53 (SS)
// 50 (MISO)            -- 50 (MISO)
// 51 (MOSI)            -- 51 (MOSI)
// 52 (SCK)            --  52 (SCK)
// https://m.blog.naver.com/PostView.nhn?blogId=yuyyulee&logNo=220331139392&proxyReferer=https%3A%2F%2Fwww.google.com%2F
/////////////////////////////////////////////////////////
// In C:\Program Files (x86)\Arduino\hardware\arduino\avr\libraries\SPI\src: SPI.h constructs SPI object as extern SPIClass SPI

// SPI.cpp defines SPI, and is included as part of the whole program.
#include <SPI.h>



int ss1 = 49; // connect master pin 49 to the first uno slave pin 10
int ss2 = 48; // connect master pin 48 to the second uno   slave pin 10
int ss3 = 47; // connect master pin 47 to the third uno  slave pin 10

int ss4 = 46; // connect master pin 48 to the fourth uno   slave pin 10
int ss5 = 45; // connect master pin 47 to the fifth uno  slave pin 10


const int NumPixels1L = 30; // The left of the 1st box
const int NumPixels1R = 25;

const int NumPixels2L = 29;
const int NumPixels2R = 25;

const int NumPixels3L = 28;
const int NumPixels3R = 27;

const int NumPixels4L = 30;
const int NumPixels4R = 30;

const int NumPixels5L = 35;
const int NumPixels5R = 27;



const int ByteSize1R = NumPixels1R * 3;
const int ByteSize2R = NumPixels2R * 3;
const int ByteSize3R = NumPixels3R * 3;
const int ByteSize4R = NumPixels4R * 3;
const int ByteSize5R = NumPixels5R * 3;


const int NumPixelsLeftGroups = NumPixels1L + NumPixels2L + NumPixels3L + NumPixels4L + NumPixels5L;  
const int NumPixelsRightGroups = NumPixels1R + NumPixels2R + NumPixels3R + NumPixels4R + NumPixels5R; 

// const int ByteSizeLeft = NumPixelsLeftGroups * 3; // for debugging

const int ByteSizeLeft = 0; // for Right LED only testing
 
const int ByteSizeRight = NumPixelsRightGroups * 3;


const int m_totalByteSize = ByteSizeLeft + ByteSizeRight; 

//byte m_receiveBuffer[SERIAL_RX_BUFFER_SIZE];


byte m_totalReceiveBuffer[m_totalByteSize] ;


// SERIAL_RX_BUFFER_SIZE == 64;
// defined in C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\HardWareSerial.h



const byte m_startByte = 255;
int m_pos;

//boolean m_newFrameHasBeenCompleted = false;

boolean m_newFrameHasBeenCompleted = true; // only for debugging 

boolean  m_newFrameToBePrinted = true;

boolean m_newFrameHasStarted = false;

// newFrameHasArrived is true when m_totalNumOfPixels of LED Pixel Data has arrived but not yet displayed or sent

//
//SPISettings SPISettingA (4000000, MSBFIRST, SPI_MODE0); // 14MHz = speed; slave 1
//SPISettings SPISettingB (4000000, MSBFIRST, SPI_MODE0); // 14MHz = speed; slave 2
//SPISettings SPISettingC (4000000, MSBFIRST, SPI_MODE0); // 14MHz = speed; slave 3
//SPISettings SPISettingD (4000000, MSBFIRST, SPI_MODE0); // 14MHz = speed; slave 4

//SPISettings mySettting(speedMaximum, dataOrder, dataMode)

//Parameters
//speedMaximum: The maximum speed of communication. For a SPI chip rated up to 20 MHz, use 20,000000.
//Arduino will automatically use the best speed that is equal to or less than the number you use with SPISettings.

//On Mega, default speed is 4 MHz (SPI clock divisor at 4). Max is 8 MHz (SPI clock divisor at 2).
//SPI can operate at extremely high speeds (millions of bytes per second), which may be too fast for some devices.
//To accommodate such devices, you can adjust the data rate.
//In the Arduino SPI library, the speed is set by the setClockDivider() function,
//which divides the master clock (16MHz on most Arduinos) down to a frequency between 8MHz (/2) and 125kHz (/128).
//https://www.dorkbotpdx.org/blog/paul/spi_transactions_in_arduino
//The clock speed you give to SPISettings is the maximum speed your SPI device can use,
//not the actual speed your Arduino compatible board can create.

//dataOrder: MSBFIRST or LSBFIRST : Byte transfer from the most significant bit (MSB) Transfer?
//dataMode : SPI_MODE0, SPI_MODE1, SPI_MODE2, or SPI_MODE3

//The SPISettings code automatically converts the max clock to the fastest clock your board can produce,
//which doesn't exceed the SPI device's capability.  As Arduino grows as a platform, onto more capable hardware,
//this approach is meant to allow SPI-based libraries to automatically use new faster SPI speeds.

void setup (void) {


  // set the pins from which SPI data is sent as outputs:

 
  pinMode(ss1, OUTPUT);
  pinMode(ss2, OUTPUT);
  pinMode(ss3, OUTPUT);
  pinMode(ss4, OUTPUT);

  pinMode(ss5, OUTPUT);
  

 // Disable these Pins at the beginning by setting them HIGH

  
  digitalWrite(ss1, HIGH);
  digitalWrite(ss2, HIGH);
  digitalWrite(ss3, HIGH);
  digitalWrite(ss4, HIGH);
  digitalWrite(ss5, HIGH);

  SPI.begin(); // set up:
  
  //To condition the hardware you call SPI.begin () which configures the SPI pins (SCK, MOSI, SS) as outputs.
  //It sets SCK and MOSI low, and SS high.
  //It then enables SPI mode with the hardware in "master" mode. This has the side-effect of setting MISO as an input.


  // Slow down the master a bit
    // The default setting is SPI_CLOCK_DIV4,
    
  //SPI.setClockDivider(SPI_CLOCK_DIV8);

  // SPI.setClockDivider(SPI_CLOCK_DIV16); // It was used for Researsal Test in ACC 20191203

  //  SPI.setClockDivider() Sets the SPI clock divider relative to the system clock.
  // On AVR based boards, the dividers available are 2, 4, 8, 16, 32, 64 or 128.
  // The default setting is SPI_CLOCK_DIV4,
  // which sets the SPI clock to one-quarter the frequency of the system clock (4 Mhz for the boards at 16 MHz).
  // SPI.setBitOrder(MSBFIRST);

  Serial1.begin(57600); // FOr Debug: To write message to the serial monitor
  
  Serial.begin(57600); // To read bytes from the PC Unity Script;  Unity Script also sets this speed

  
  //To Define another serial port:
  //https://www.arduino.cc/reference/en/language/functions/communication/serial/
  //
  //https://m.blog.naver.com/PostView.nhn?blogId=darknisia&logNo=220569815020&proxyReferer=https%3A%2F%2Fwww.google.com%2F
  // Mega has 4 Serial Ports: Serial, Serial1, Serial2, Serial3.
  // Serial ports are defined by Pin 0 and 1; Serial1 is defined by pins 19(RX), 18(TX).
  
  // Connect the first USB cable  to Pin 0 and 1 by the ordinary method; Connect the second USB cable from the second
  // USB port in the PC to Pin 19 and 18; Also open another arduino IDE for the second serial port, Serial1.
  // Use the first arduino IDE to upload the arduino code, and use the second arduino IDE to report messages.


// artificial LED RGB data for testing: 1 2 3;1 2 3; 1 2 3;.......



//
//// Left Unos
//for (int i = 0; i < NumPixels1L *3; i++)
//{
//
//  if ( i%3 == 0 ) // i is a  multiple of 3 
//  { 
//    m_totalReceiveBuffer[i]= 254;    
//  }
//   if ( i%3 == 1 ) // i is a  multiple of 3 plus 1
//  { 
//    m_totalReceiveBuffer[i]= 0;    
//  }
//   if ( i%3 ==  2 ) // i is a  multiple of 3 plus 2
//  { 
//    m_totalReceiveBuffer[i]= 0;
//    
//  }
//}
//for (int i = 0; i < NumPixels2L *3; i++) 
//  {
//
//  if ( i%3 == 0 ) // i is a  multiple of 3 
//  { 
//    m_totalReceiveBuffer[ NumPixels1L *3 + i]= 254;    
//  }
//   if ( i%3 == 1 ) // i is a  multiple of 3 plus 1
//  { 
//    m_totalReceiveBuffer[ NumPixels1L *3 + i]= 0;    
//  }
//   if ( i%3 ==  2 ) // i is a  multiple of 3 plus 2
//  { 
//    m_totalReceiveBuffer[ NumPixels1L *3 + i]= 0;
//    
//  }
//}
//
//for (int i = 0; i < NumPixels3L *3; i++)
//{
//
//  if ( i%3 == 0 ) // i is a  multiple of 3 
//  { 
//    m_totalReceiveBuffer[  NumPixels1L *3 + NumPixels2L *3 + i]= 254;    
//  }
//   if ( i%3 == 1 ) // i is a  multiple of 3 plus 1
//  { 
//    m_totalReceiveBuffer[  NumPixels1L *3 + NumPixels2L *3 + i]= 0;    
//  }
//   if ( i%3 ==  2 ) // i is a  multiple of 3 plus 2
//  { 
//    m_totalReceiveBuffer[ NumPixels1L *3 + NumPixels2L *3 + i]= 0;
//    
//  }
//}
//
//for (int i = 0; i < NumPixels4L *3; i++) 
//{
//
//  if ( i%3 == 0 ) // i is a  multiple of 3 
//  { 
//    m_totalReceiveBuffer[ NumPixels1L *3 + NumPixels2L *3 + NumPixels3L *3 + i]=254;    
//  }
//   if ( i%3 == 1 ) // i is a  multiple of 3 plus 1
//  { 
//    m_totalReceiveBuffer[ NumPixels1L *3 + NumPixels2L *3 + NumPixels3L *3 + i]=0;    
//  }
//   if ( i%3 ==  2 ) // i is a  multiple of 3 plus 2
//  { 
//    m_totalReceiveBuffer[ NumPixels1L *3 + NumPixels2L *3 + NumPixels3L *3 + i]=0;
//    
//  }
//
//}
//
//for (int i = 0; i < NumPixels5L *3; i++) 
//{
//
//  if ( i%3 == 0 ) // i is a  multiple of 3 
//  { 
//    m_totalReceiveBuffer[ NumPixels1L *3 + NumPixels2L *3 + NumPixels3L *3 +  NumPixels4L *3+ i] = 254;    
//  }
//   if ( i%3 == 1 ) // i is a  multiple of 3 plus 1
//  { 
//    m_totalReceiveBuffer[ NumPixels1L *3 + NumPixels2L *3 + NumPixels3L *3 + NumPixels4L *3+ i]= 0;    
//  }
//   if ( i%3 ==  2 ) // i is a  multiple of 3 plus 2
//  { 
//    m_totalReceiveBuffer[ NumPixels1L *3 + NumPixels2L *3 + NumPixels3L *3 + NumPixels4L *3 + i]= 0;
//    
//  }
//
//}
//



//// Right  Unos
//for (int i = 0; i < NumPixels1R *3; i++)
//{
//
//  if ( i%3 == 0 ) // i is a  multiple of 3 
//  { 
//    m_totalReceiveBuffer[ByteSizeLeft + i]=  0;    
//  }
//   if ( i%3 == 1 ) // i is a  multiple of 3 plus 1
//  { 
//    m_totalReceiveBuffer[ByteSizeLeft + i]=  254;    
//  }
//   if ( i%3 ==  2 ) // i is a  multiple of 3 plus 2
//  { 
//    m_totalReceiveBuffer[ByteSizeLeft + i]= 0;
//    
//  }
//}
//for (int i = 0; i < NumPixels2R *3; i++) 
//  {
//
//  if ( i%3 == 0 ) // i is a  multiple of 3 
//  { 
//    m_totalReceiveBuffer[ ByteSizeLeft+ NumPixels1R *3 + i]=  0;    
//  }
//   if ( i%3 == 1 ) // i is a  multiple of 3 plus 1
//  { 
//    m_totalReceiveBuffer[ ByteSizeLeft+ NumPixels1R *3 + i]= 254;    
//  }
//   if ( i%3 ==  2 ) // i is a  multiple of 3 plus 2
//  { 
//    m_totalReceiveBuffer[ ByteSizeLeft + NumPixels1R *3 + i]=  0;
//    
//  }
//}
//
//for (int i = 0; i < NumPixels3R *3; i++)
//{
//
//  if ( i%3 == 0 ) // i is a  multiple of 3 
//  { 
//    m_totalReceiveBuffer[ByteSizeLeft +  NumPixels1R *3 + NumPixels2R *3 + i]= 0;    
//  }
//  if ( i%3 == 1 ) // i is a  multiple of 3 plus 1
//  { 
//    m_totalReceiveBuffer[ ByteSizeLeft +  NumPixels1R *3 + NumPixels2R *3 + i]= 254;    
//  }
//   if ( i%3 ==  2 ) // i is a  multiple of 3 plus 2
//  { 
//    m_totalReceiveBuffer[ ByteSizeLeft + NumPixels1R *3 + NumPixels2R *3 + i]= 0;
//    
//  }
//}
//
//for (int i = 0; i < NumPixels4R *3; i++) 
//{
//
//  if ( i%3 == 0 ) // i is a  multiple of 3 
//  { 
//    m_totalReceiveBuffer[ ByteSizeLeft + NumPixels1R *3 + NumPixels2R *3 + NumPixels3R *3 + i] = 0;    
//  }
//   if ( i%3 == 1 ) // i is a  multiple of 3 plus 1
//  { 
//    m_totalReceiveBuffer[ ByteSizeLeft + NumPixels1R *3 + NumPixels2R *3 + NumPixels3R *3 + i]= 254;    
//  }
//   if ( i%3 ==  2 ) // i is a  multiple of 3 plus 2
//  { 
//    m_totalReceiveBuffer[ ByteSizeLeft + NumPixels1R *3 + NumPixels2R *3 + NumPixels3R *3 + i] = 0;
//    
//  }
//
//}
//
//for (int i = 0; i < NumPixels5R *3; i++) 
//{
//
//  if ( i%3 == 0 ) // i is a  multiple of 3 
//  { 
//    m_totalReceiveBuffer[ ByteSizeLeft + NumPixels1R *3 + NumPixels2R *3 + NumPixels3R *3 +  NumPixels4R *3+ i]= 0;    
//  }
//   if ( i%3 == 1 ) // i is a  multiple of 3 plus 1
//  { 
//    m_totalReceiveBuffer[ ByteSizeLeft + NumPixels1R *3 + NumPixels2R *3 + NumPixels3R *3 + NumPixels4R *3+ i]= 254;    
//  }
//   if ( i%3 ==  2 ) // i is a  multiple of 3 plus 2
//  { 
//    m_totalReceiveBuffer[ ByteSizeLeft + NumPixels1R *3 + NumPixels2R *3 + NumPixels3R *3 + NumPixels4R *3 + i]= 0;
//    
//  }
//
//}//for

// Right  Unos
for (int i = 0; i < NumPixels1R *3; i++)
{

  if ( i%3 == 0 ) // i is a  multiple of 3 
  { 
    m_totalReceiveBuffer[ i]=  0;    
  }
   if ( i%3 == 1 ) // i is a  multiple of 3 plus 1
  { 
    m_totalReceiveBuffer[ i]=  0;    
  }
   if ( i%3 ==  2 ) // i is a  multiple of 3 plus 2
  { 
    m_totalReceiveBuffer[ i]= 0;
    
  }
}
for (int i = 0; i < NumPixels2R *3; i++) 
  {

  if ( i%3 == 0 ) // i is a  multiple of 3 
  { 
    m_totalReceiveBuffer[  NumPixels1R *3 + i]=  0;    
  }
   if ( i%3 == 1 ) // i is a  multiple of 3 plus 1
  { 
    m_totalReceiveBuffer[  NumPixels1R *3 + i]= 0;    
  }
   if ( i%3 ==  2 ) // i is a  multiple of 3 plus 2
  { 
    m_totalReceiveBuffer[NumPixels1R *3 + i]=  0;
    
  }
}

for (int i = 0; i < NumPixels3R *3; i++)
{

  if ( i%3 == 0 ) // i is a  multiple of 3 
  { 
    m_totalReceiveBuffer[ NumPixels1R *3 + NumPixels2R *3 + i]= 0;    
  }
  if ( i%3 == 1 ) // i is a  multiple of 3 plus 1
  { 
    m_totalReceiveBuffer[  NumPixels1R *3 + NumPixels2R *3 + i]= 0;    
  }
   if ( i%3 ==  2 ) // i is a  multiple of 3 plus 2
  { 
    m_totalReceiveBuffer[  NumPixels1R *3 + NumPixels2R *3 + i]= 0;
    
  }
}

for (int i = 0; i < NumPixels4R *3; i++) 
{

  if ( i%3 == 0 ) // i is a  multiple of 3 
  { 
    m_totalReceiveBuffer[  NumPixels1R *3 + NumPixels2R *3 + NumPixels3R *3 + i] = 0;    
  }
   if ( i%3 == 1 ) // i is a  multiple of 3 plus 1
  { 
    m_totalReceiveBuffer[  NumPixels1R *3 + NumPixels2R *3 + NumPixels3R *3 + i]= 0;    
  }
   if ( i%3 ==  2 ) // i is a  multiple of 3 plus 2
  { 
    m_totalReceiveBuffer[  NumPixels1R *3 + NumPixels2R *3 + NumPixels3R *3 + i] = 0;
    
  }

}

for (int i = 0; i < NumPixels5R *3; i++) 
{

  if ( i%3 == 0 ) // i is a  multiple of 3 
  { 
    m_totalReceiveBuffer[ NumPixels1R *3 + NumPixels2R *3 + NumPixels3R *3 +  NumPixels4R *3+ i]= 0;    
  }
   if ( i%3 == 1 ) // i is a  multiple of 3 plus 1
  { 
    m_totalReceiveBuffer[ NumPixels1R *3 + NumPixels2R *3 + NumPixels3R *3 + NumPixels4R *3+ i]= 0;    
  }
   if ( i%3 ==  2 ) // i is a  multiple of 3 plus 2
  { 
    m_totalReceiveBuffer[ NumPixels1R *3 + NumPixels2R *3 + NumPixels3R *3 + NumPixels4R *3 + i]= 0;
    
  }

}//for


// for debugging
//Serial.print("message 1 [forloop] in setup:");
for (int i = 0; i < m_totalByteSize; i++) {

    // print the received data from first Mega to the second Mega to the PC monitor
    if ( i % 3 == 0) {
    // Serial.print("r:");
      Serial.println(m_totalReceiveBuffer[i]);
    }
    else if ( i % 3 == 1) {
    // Serial.print("g:");
     Serial.println(m_totalReceiveBuffer[i]);
    }

   else {
      //Serial.print("b:");
      Serial.println(m_totalReceiveBuffer[i]);
    }

  }// for


Serial.print("message 2 [func] in setup:");
  printLEDBytesToSerialMonitor();

} // setup

// https://arduino.stackexchange.com/questions/8457/serial-read-vs-serial-readbytes
// readBytes() is blocking until the determined length has been read, or it times out (see Serial.setTimeout()).
// Where read() grabs what has come, if it has come in. Hence available is used to query if it has.
//
// This is why you see the Serial.read() inside a while or if Serial.available.
// Hence I typically employ something like the following: Which emulates readBytes (for the most part).
//
//    #define TIMEOUT = 3000;
//    loop {
//        char inData[20];
//        unsigned long timeout = millis() + TIMEOUT;
//        uint8_t inIndex = 0;
//        while ( ((int32_t)(millis() - timeout) < 0) && (inIndex < (sizeof(inData)/sizeof(inData[0])))) {
//            if (Serial1.available() > 0) {
//                read the incoming byte:
//                inData[inIndex] = Serial.read();
//                if ((c == '\n') || (c == '\r')) {
//                    break;
//                }
//                Serial.write(inData[inIndex++]);
//            }
//        }
//    }
//
//SO: I will stick with using readBytes() because it seems to produce consistent results
//and I can predict the number of bytes I should receive back. –

void loop () 
{

  // If Serial.read() == -1 =oxff , it means that head == tail, i.e. there are no bytes to read, that is, underflow happened
  //Serial1.print("b:");

  //myReadByte(); // commented out for debug: read a new byte changing the state of the reading process, m_newFrameHasArrived. 

    
  
  if ( m_newFrameHasBeenCompleted ) 
  { // a new frame has been completed and is ready to be sent to the slaves; This flag is set in myReadByte()
   // showNewFrame(); // display the new frame of LED data that has arrived
  
   sendLEDBytesToSlaves();

   if (  m_newFrameToBePrinted )
   {


   Serial.print("message 3 [foloop] in loop():");

         
   for (int i = 0; i < m_totalByteSize; i++) // because the printing process is slow, the message may be confused
   // if you execute loop() two fast. 
   {
   
    // print the received data from first Mega to the second Mega to the PC monitor
    if ( i % 3 == 0) {
     //Serial.print("r:");
      Serial.println(m_totalReceiveBuffer[i]);
    }
    else if ( i % 3 == 1) {
    //Serial.print("g:");
     Serial.println(m_totalReceiveBuffer[i]);
    }

   else {
      //Serial.print("b:");
      Serial.println(m_totalReceiveBuffer[i]);
    }

  }// for
  
     // Serial.print("message 4 [function] in loop():");
      printLEDBytesToSerialMonitor();
    
      m_newFrameToBePrinted = false;
   }
   // m_newFrameHasBeenCompleted = false;
  }


} // loop()

//void myReadByte() {
//
//  static int index = 0; // index of the buffer
//
//  byte c; // received byte
//
//  int count = Serial.available();
//
//
//  Serial1.print("count=");   Serial1.print( count); // For Debug
//  if (count == 0) {
//    // not bytes has arrived
//    return;
//  }
//
//  // there are avaiable bytes in the incoming ring buffer
//  c = Serial.read(); // read the first byte in the incoming ring buffer, changing the pointer to the next place
//
//  Serial1.print("byte=");   Serial1.print( c); // For Debug
//  
//  // there are two cases to handle: (1) the start byte HAS arrived (2) the start byte has NOT arrived
//  // These two states are indicated by  m_newFrameHasStarted
//
// // case 1:
//  if ( !m_newFrameHasStarted ) { // the arrival of the startByte has not been verified. 
//    
//    // check if the start byte has arrived
//
//    if ( c == m_startByte ) { // the startbyte was sent by Unity
//      m_newFrameHasStarted = true; // this indicates that the start byte arrived;
//      m_pos = 0; // points to the beginning of the buffer  to read the next byte into; The startByte is not inserted to the buffer
//      return;
//
//    }
//
//    else { // the read byte is not a startByte =>  Ignore the byte and continue to read to find the start byte;
//      // after this  return, myReadByte() is called again
//      return;
//
//    }
//
//  } // if ( !m_newFrameHasStarted )
//
//// case 2:
//  else { // The new frame has started (the start byte arrived) =>  add the received byte to the receive buffer at the empty location m_pos
//  
//    m_totalReceiveBuffer[m_pos] = c;
//
//    if ( m_pos == m_totalByteSize - 1) { // if the buffer index points to the last position of the receive buffer
//      // it means that the receive buffer is full and the current frame has been completed.
//      m_newFrameHasBeenCompleted == true; // this flag is used to indicate that the new frame has been completed and is ready to be sent to the slaves
//      m_newFrameHasStarted = false;   // this is set to indicate that the next start byte is being waited for
//      return;
//    } // the receive buffer is full
//
//    else { // the receive buffer is not yet full
//      m_pos ++ ; // go to the next location of the buffer
//      return;
//    }//
//
//  }//  the start byte has started and the current frame is being constructed
//
//}//myReadByte()
//


void showNewFrame() {

  sendLEDBytesToSlaves();
  
  // print the ledBytes to the serial monitor via Serial1.

  printLEDBytesToSerialMonitor();
  
}// showNewFrame()


//int HardwareSerial::available(void)
//    {
//    return ((unsigned int)(SERIAL_RX_BUFFER_SIZE + _rx_buffer_head - _rx_buffer_tail)) % SERIAL_RX_BUFFER_SIZE;
//    }
// 0<= countToRead < SERIAL_RX_BUFFER_SIZE = 64; countToRead = 0 means  head == tail, that is,  when the buffer is empty or full
//int HardwareSerial::available(void)
//{
//	return   ( (unsigned int)(SERIAL_RX_BUFFER_SIZE + _rx_buffer_head - _rx_buffer_tail) ) % SERIAL_RX_BUFFER_SIZE;
//}

//https://www.nutsvolts.com/magazine/article/july2011_smileysworkshop;
//UART uses a ring buffer where head index is incremented when a new byte is written into the buffer
//https://arduino.stackexchange.com/questions/11710/does-data-coming-in-on-arduino-serial-port-store-for-some-time
//What happens if the buffer is full and my PC writes an extra character? Does the PC block until there is buffer space,
//is an old character dropped or is the next character dropped? – Kolban Jun 19 '15 at 12: 55
//2. The next(incoming) character is dropped.– Majenko♦ Jun 19 '15 at 13:17
// SUM: Yes. The receive ring buffer is 64 bytes and will discard anything past that until the program reads them out of the buffer.

// set random color values to totalRecieveBuffer
//https://gamedev.stackexchange.com/questions/32681/random-number-hlsl

//for test LED
//for (int i = 0; i < totalByteSize/3; i++) {
//  totalRecieveBuffer[3 * i] = (byte) random(10, 255); // from 10 to 254
//  totalRecieveBuffer[3 * i +1] = (byte)random(10, 255);
//  totalRecieveBuffer[3 * i +2] = (byte)random(10, 255);


void  sendLEDBytesToSlaves()
{

  // SPI.beginTransaction(SPISettingA);

  //https://forum.arduino.cc/index.php?topic=52111.0
  // With multiple slaves, only one slave is allowed to "own" the MISO line(by configuring it as an output).So when SS is brought low
  //for that slave it switches its MISO line from high - impedance to output, then it can reply to requests
  //from the master.When the SS is brought high again(inactive) that slave must reconfigure that line as high - impedance,
  //so another slave can use it.


//
//  // send the left groups of LED data to the Mega slave:
//
//  digitalWrite(ssLeft, LOW); // select the second Mega  SS line
//
//  // To send  a sequence of bytes to a slave arduiono via SPI, the sequence is marked by the start and the end
//  // of the sequence with special bytes, m_startByte and m_endByte respectivley.
//
//  SPI.transfer( m_startByte); // The SPI master also sends the startByte for the slave to identify the beginning of the frame.
//  SPI.transfer( &totalReceiveBuffer[0], ByteSizeLeft);
//
//
//  digitalWrite(ssLeft, HIGH);

  // Process the right side LED data
  
  // send the first group of data to the first slave:

// select the first SS line
  digitalWrite(ss1, LOW); 

  // To send  a sequence of bytes to a slave arduiono via SPI, the sequence is marked by the start and the end
  // of the sequence with special bytes, m_startByte and m_endByte respectivley.

  SPI.transfer( m_startByte);
  SPI.transfer( &m_totalReceiveBuffer[0], ByteSize1R);

// deselect the first SS line
  digitalWrite(ss1, HIGH);

  //SPI.endTransaction();

  // send the second group of data to the second slave:
  // SPI.beginTransaction(SPISettingB);

 // select the second SS Line
  digitalWrite(ss2, LOW);

  SPI.transfer( m_startByte);
  SPI.transfer( &m_totalReceiveBuffer[ByteSize1R], ByteSize2R);

 // deselect the second SS Line
  digitalWrite(ss2, HIGH);

  //SPI.endTransaction();

  // send the third group of data to the third slave:
  //SPI.beginTransaction(SPISettingC);

// select the third SS line
  digitalWrite(ss3, LOW); 

  SPI.transfer( m_startByte);
  SPI.transfer( &m_totalReceiveBuffer[ByteSize1R + ByteSize2R], ByteSize3R);

// deselect the third SS line
  digitalWrite(ss3, HIGH);

  //SPI.endTransaction();

  // send the fourth group of data to the fourth slave:

  // SPI.beginTransaction(SPISettingD);

  // select the fourth SS line
  digitalWrite(ss4, LOW);  

  SPI.transfer( m_startByte);
  SPI.transfer( &m_totalReceiveBuffer[ByteSize1R + ByteSize2R + ByteSize3R ], ByteSize4R );

 // deselect the fourth SS line
  digitalWrite(ss4, HIGH);

 // select the fifth SS line 
  digitalWrite(ss5, LOW);  

  SPI.transfer( m_startByte);
  SPI.transfer( &m_totalReceiveBuffer[ ByteSize1R + ByteSize2R + ByteSize3R + ByteSize4R ], ByteSize5R );

 // deselect the fifth SS line 
  digitalWrite(ss5, HIGH);


  // SPI.endTransaction();

  // If other libraries use the SPI (hardware resource)  from interrupts,
  // they will be prevented from accessing SPI until you call SPI.endTransaction().

  delay(50); // make the interrupt process slower so that there would be enough time to read the buffer

} //  sendLEDBytesToSlaves( )




void printLEDBytesToSerialMonitor()
{
    
for (int i = 0; i < m_totalByteSize; i++) 
{
   
    // print the received data from first Mega to the second Mega to the PC monitor
    if ( i % 3 == 0) {
     //Serial.print("r:");
      Serial.println(m_totalReceiveBuffer[i]);
    }
    else if ( i % 3 == 1) {
     //Serial.print("g:");
     Serial.println(m_totalReceiveBuffer[i]);
    }

   else {
     // Serial.print("b:");
      Serial.println(m_totalReceiveBuffer[i]);
    }

  }// for

}//void printLEDBytesToSerialMonitor()
