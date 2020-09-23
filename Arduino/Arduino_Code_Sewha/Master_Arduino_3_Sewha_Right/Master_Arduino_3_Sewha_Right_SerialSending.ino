

//Sewah: left: 152, right : 134
// Sewha: A total num of LED = 153+ 134

//Btw, you can copy the SoftwareSerial.cppand h files from the Arduino directory in to your project directoryand then include it with "(quotation marks) instead of the < (pointy brackets). If you do this, make sure to also change the #include <SoftwareSerial.h> in your copy of SoftwareSerial.cpp to "SoftwareSerial.h".
//~you use <pointy brackets> to include files from arduino libraries and
//"quotes" to include from project directory.

#include "HardwareSerial.h" // The include file defines Serial, Serial1, Serial2, Serial3 within it
#include "SoftwareSerial.h"
//https://www.arduino.cc/en/Reference/softwareSerial

//Not all pins on the Megaand Mega 2560 support change interrupts, 
//so only the following can be used for RX: 10, 11, 12, 13, 14, 15, 50, 51, 52, 53,
//A8(62), A9(63), A10(64), A11(65), A12(66), A13(67), A14(68), A15(69).

#define rxPin4 10
#define txPin4 11

#define rxPin5 12
#define txPin5 13

#define rxPin6 14
#define txPin6 15

// set up a new serial port
SoftwareSerial Serial4(rxPin4, txPin4);
SoftwareSerial Serial5(rxPin5, txPin5);
SoftwareSerial Serial6(rxPin6, txPin6);

// SoftwareSerial  Serial4(rxPin5, txPin5);
//The Arduino Mega has three additional serial ports:
//Serial1 on pins 19 (RX) and 18 (TX), 
//Serial2 on pins 17 (RX) and 16 (TX), 
//Serial3 on pins 15 (RX) and 14 (TX).

// A total num of LED = 186; each slave processes 40 LEDs
//const int NumPixels1 = 40;
//const int NumPixels2 = 44;
//const int NumPixels3 = 50;
//const int NumPixels4 = 52;


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


//const int NumPixelsLeftGroups = 152;
//const int NumPixelsRightGroups = 134;

const int NumPixelsLeftGroups = NumPixels1L + NumPixels2L + NumPixels3L + NumPixels4L + NumPixels5L;
const int NumPixelsRightGroups = NumPixels1R + NumPixels2R + NumPixels3R + NumPixels4R + NumPixels5R;

//const int ByteSizeLeft = NumPixelsLeftGroups * 3;

const int ByteSizeRight = NumPixelsRightGroups * 3;


const int m_totalByteSize = ByteSizeRight; // 2 is added for the start and end bytes


// SERIAL_RX_BUFFER_SIZE is defined to be 256;
// SERIAL_RX_BUFFER_SIZE == 64;
// defined in C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\HardWareSerial.h

byte m_totalReceiveBuffer[m_totalByteSize];




const byte m_startByte = '\n'; // the new line char
const byte m_endByte = '\r';   // the return char
int m_pos;

boolean m_newFrameHasBeenCompleted = false;
boolean m_newFrameHasStarted = false;

void printLEDBytesToSerialMonitor(byte* totalReceiveBuffer, int totalByteSize);
void  sendLEDBytesToSlaves(byte* totalReceiveBuffer, int totalByteSize);

void setup(void) {

	Serial.begin(115200); // For PC to send  bytes to the arduino. 

	// The serial ports for the 5 slave arduinos
	Serial1.begin(115200); // for sending messages to PC

	Serial2.begin(115200);
	Serial3.begin(115200);
	Serial4.begin(115200);
	Serial5.begin(115200);

	Serial6.begin(115200);

	//https://www.arduino.cc/reference/en/language/functions/communication/serial/
	//
	//https://m.blog.naver.com/PostView.nhn?blogId=darknisia&logNo=220569815020&proxyReferer=https%3A%2F%2Fwww.google.com%2F
	// Mega has 4 Serial Ports: Serial, Serial1, Serial2, Serial3.
	// Serial ports are defined by Pin 0 and 1; Serial1 is defined by pins 19(RX), 18(TX).

	// Connect the first USB cable  to Pin 0 and 1 by the ordinary method; Connect the second USB cable from the second
	// USB port in the PC to Pin 19 and 18; Also open another arduino IDE for the second serial port, Serial1.
	// Use the first arduino IDE to upload the arduino code, and use the second arduino IDE to report messages.

}

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

void loop(void) {

	// If Serial.read() == -1 =oxff , it means that head == tail, i.e. there are no bytes to read, 
	// that is, underflow happened 

	myReadByte(); // read a new byte changing the state of the reading process, m_newFrameHasArrived. 

	if (m_newFrameHasBeenCompleted) {
		// a new frame has been completed and is ready to be sent to the slaves; This flag is set in myReadByte()


		printLEDBytesToSerialMonitor(m_totalReceiveBuffer, m_totalByteSize); // for debug
		sendLEDBytesToSlaves(m_totalReceiveBuffer, m_totalByteSize);

		m_newFrameHasBeenCompleted = false;
	}

} // loop()

void myReadByte() {

	static int index = 0; // index of the buffer

	byte c; // received byte

	int count = Serial.available();


	Serial1.print("count=");   Serial1.print(count); // For Debug
	if (count == 0) {

		// not bytes has arrived
		return;
	}

	// there are avaiable bytes in the incoming ring buffer
	c = Serial.read(); // read the first byte in the incoming ring buffer, changing the pointer to the next place

	//Serial1.print("byte=");   Serial1.print( c); // For Debug

	// there are two cases to handle: (1) the start byte HAS arrived (2) the start byte has NOT arrived
	// These two states are indicated by  m_newFrameHasStarted

   // case 1:
	if (!m_newFrameHasStarted)
	{ // the arrival of the startByte has not been verified. 

	  // check if the start byte has arrived

		if (c == m_startByte)
		{ // the startbyte was sent by Unity

			m_newFrameHasStarted = true; // this indicates that the start byte arrived;
			m_pos = 0; // points to the beginning of the buffer  to read the next byte into; The startByte is not inserted to the buffer
		}


		{ // the read byte is not a startByte =>  Ignore the byte and continue to read to find the start byte;

		}

		return;  // After the return, myReadByte() is called again	

	} // if ( !m_newFrameHasStarted )

  // case 2:
	else
	{ // The new frame has started (the start byte arrived) =>  add the received byte to the receive buffer at the empty location m_pos

		if (m_pos == m_totalByteSize)
		{
			//  the buffer index m_pos points to the next to the last position of the receive buffer.
			//	it means that the current frame contains the expected number of bytes.

			 // check if the current byte is the endByte. If so, it means that the frame is completed 

			if (c == m_endByte)
			{ // the end was sent by Unity
				m_newFrameHasStarted = false;
				m_newFrameHasBeenCompleted == true;
			}
			else
			{  // the current frame contains the expected number of bytes and the current byte is NOT the endByte
			   // it means the current frame is corrupted and should be discarded.
				// It can be achieved by setting m_newFrameHasStarted = false, which causes the 
				// code to wait for the startByte. 
				m_newFrameHasStarted = false;
			}

		} // if (m_pos == m_totalByteSize )

		else
		{
			m_totalReceiveBuffer[m_pos] = c;
			m_pos++;

		} // else (m_pos == m_totalByteSize )


	} // else of  (!m_newFrameHasStarted)


}//myReadByte()



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


//http://gammon.com.au/forum/?id=10892
//https://sites.google.com/site/qeewiki/books/avr-guide/spi
void  sendLEDBytesToSlaves(byte* totalReceiveBuffer, int totalByteSize)
{

	//Serial.write(buf, len): 115200 bps:  serial transmission is asynchronous.
	  //If there is enough empty space in the transmit buffer, 
	  //Serial.write() will return before any characters are transmitted over serial.
	  //If the transmit buffer is full then Serial.write() will block until there is enough space in the buffer.
	  //To avoid blocking calls to Serial.write(), 
	  //you can first check the amount of free space in the transmit buffer using availableForWrite().

	//https://forum.arduino.cc/index.php?topic=52111.0

	//int numOfAvailable1 = Serial1.availableForWrite();
	int numOfAvailable2 = Serial2.availableForWrite();
	int numOfAvailable3 = Serial3.availableForWrite();
	int numOfAvailable4 = Serial4.availableForWrite();
	int numOfAvailable5 = Serial5.availableForWrite();
	int numOfAvailable6 = Serial6.availableForWrite();

	Serial1.print("  numOfAvailable2 ="); Serial1.println( numOfAvailable2);
    Serial1.print("  numOfAvailable3 ="); Serial1.println( numOfAvailable3);
    Serial1.print("  numOfAvailable4 ="); Serial1.println( numOfAvailable4);
    Serial1.print("  numOfAvailable5 ="); Serial1.println( numOfAvailable5);
    Serial1.print("  numOfAvailable6 ="); Serial1.println( numOfAvailable6);
	// check if the available space for writing to esch serial port is more than the number of bytes to send
	// In that case, serial.write() is not blocking but returns immediately. Hence, all 5 serials
	// send the bytes in parallel.

	if ( numOfAvailable2 > ByteSize1R && numOfAvailable3 > ByteSize2R
		&& numOfAvailable4 > ByteSize3R && numOfAvailable5 > ByteSize4R
		&& numOfAvailable6 > ByteSize5R )

	{

		Serial2.write(m_startByte);
		Serial2.write(&totalReceiveBuffer[0], ByteSize1R);
		Serial2.write(m_endByte);


		Serial3.write(m_startByte);
		Serial3.write(&totalReceiveBuffer[ByteSize1R], ByteSize2R);
		Serial3.write(m_endByte);


		Serial4.write(m_startByte);
		Serial4.write(&totalReceiveBuffer[ByteSize1R + ByteSize2R], ByteSize3R);
		Serial4.write(m_endByte);

		Serial5.write(m_startByte);
		Serial5.write(&totalReceiveBuffer[ByteSize1R + ByteSize2R + ByteSize3R], ByteSize4R);
		Serial5.write(m_endByte);


		Serial6.write(m_startByte);
		Serial6.write(&totalReceiveBuffer[ByteSize1R + ByteSize2R + ByteSize3R + ByteSize4R], ByteSize5R);
		Serial6.write(m_endByte);

	}

} //  sendLEDBytesToSlaves(totalReceiveBuffer,  m_totalByteSize )

// For Debugging
void printLEDBytesToSerialMonitor(byte* totalReceiveBuffer, int totalByteSize)
{

	for (int i = 0; i < totalByteSize; i++) {

		// print the received data from PC to the serial monitor via Serial1 of Mega
		if (i % 3 == 0) {
			Serial1.print("r:");
			Serial1.println(totalReceiveBuffer[i]);
		}
		else if (i % 3 == 1) {
			Serial1.print("g:");
			Serial1.println(totalReceiveBuffer[i]);
		}

		else {
			Serial1.print("b:");
			Serial1.println(totalReceiveBuffer[i]);
		}

	}// for


} //printLEDBytesToSerialMonitor( byte[] totalReceiveBuffer,  int m_totalByteSize  )
