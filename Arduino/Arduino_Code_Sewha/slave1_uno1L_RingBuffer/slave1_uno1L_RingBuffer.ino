//COM20

// 3W pixie: assembly article: http://ytai-mer.blogspot.com/2015/11/pixie-bright-things-come-in-small.html
//Serial versus Parallel: https://www.ledsupply.com/blog/wiring-leds-correctly-series-parallel-circuits-explained/
// Serial Chain of LEDs: Forward Voltage etc. https://www.electronics-tutorials.ws/diode/diode_8.html
#include <SPI.h>//https://www.ledsupply.com/blog/wiring-leds-correctly-series-parallel-circuits-explained/
#include "SoftwareSerial.h"// About the voltage drop for a chain of LEDs: https://forums.adafruit.com/viewtopic.php?f=47&t=65815
//Powering a chain of LED from both ends: https://community.particle.io/t/when-i-connect-chain-of-led-strips-to-power-from-both-ends-do-i-need-to-make-the-power-line-disconnect-in-the-middle/35555
// Also see: https://community.particle.io/t/when-i-connect-chain-of-led-strips-to-power-from-both-ends-do-i-need-to-make-the-power-line-disconnect-in-the-middle/35555/21


// example codes: https://learn.adafruit.com/fiber-optic-pixie-princess-skirt

#include "Adafruit_Pixie.h"

#include <RingBuf.h>



#define NUMPIXELS1 30 // Number of Pixies in the strip
#define PIXIEPIN  5 // Pin number for SoftwareSerial output to the LED chain

SoftwareSerial pixieSerial(-1, PIXIEPIN);
Adafruit_Pixie strip = Adafruit_Pixie(NUMPIXELS1, &pixieSerial);

const int m_LEDChainByteSize = NUMPIXELS1 * 3; // 6 bytes are for the start and end bytes

const int m_MaxBufferSize = m_LEDChainByteSize * 10;

RingBuf<byte, m_MaxBufferSize>  m_buffer; //RingBuf<ET, S, IT, BT>::RingBuf() :

//byte m_buffer[m_bufferSize];

int m_pos = 0;

//Typically global variables are used to pass data between an ISR and the main program. To make sure variables shared between an ISR and the main program are updated correctly, declare them as volatile.
//Marking a variable as volatile tells the compiler to not "cache" the variables contents into a processor register, but always read it from memory, when needed. This may slow down processing, 
//which is why you don't just make every variable volatile, when not needed.
volatile boolean m_process_it = false;
volatile boolean m_frameInProgess = false;

volatile boolean m_got_interrupt = 0; // this is a semaphore: 
// 1 means you are outside of the interrupt handler and 0 means you are inside the interrupt
//An interrupt handler would simply INCREMENT  got_interrupt and the code 
//running outside of the interrupt handler would inspect it and, if it was non-zero,
//would DECREMENT it and take whatever actions were needed

//http://gammon.com.au/forum/?id=10892
//https://sites.google.com/site/qeewiki/books/avr-guide/spi
void setup() {
  Serial.begin(57600);
  pixieSerial.begin(115200); // Pixie REQUIRES this baud rate; begin() executes listen() within it.

  //SPI.setClockDivider(SPI_CLOCK_DIV16);
  pinMode(PIXIEPIN, OUTPUT);
  pinMode(MISO, OUTPUT); // this is needed to send bytes to the master

// Enable  SPI: SPCR |= bit(SPE); bit(n) == _BV(n), where n is a bit index
//Setting the SPE bit within the SPCR register enables SPI.   
  SPCR |= _BV(SPE); //Macro _BV(n) sets bit n to 1, whereas the other bits to 0
    
 //  The MSTR bit within the SPCR register tells the AVR whether it is a master or a slave. 
  
  SPCR &= ~_BV(MSTR); // MSTR = Master Slave Select Register; set MSTR to 1: Set as master
  
  // turn on interrupts: SPI.attachInterrupt()
  //sei();
 // interrupts(); // #define interrupts() sei()

  // SPI.attachInterrupt();  inline static void attachInterrupt() { SPCR |= _BV(SPIE); }
  //Setting the SPIE bit HIGH (1) enables the SPI interrupt.
  SPCR |= _BV(SPIE); // SPIE: SPI Interrrupt Enable
} // setup()

//Non-interrupt level code must lock out interrupts to ensure that data structures 
//and I/O are not interfered with by an interrupt handler
// Suppose you keep a linked list that you use outside of the interrupt handler,
//and add things to it inside the interrupt handler. If an interrupt happens at just the wrong time
//you'll corrupt the list structure unless you lock out interrupts when you manipulate or traverse it.
void loop() {

  // The following process of reading the sending buffer can be interrupted when new bytes arrive at the SPI port
  //  noInterrupts();
  //  long myCounter = isrCounter;  // get value set by ISR
  //  interrupts();
  //   Temporarily turning off interrupts ensures that isrCounter(a counter set inside an ISR) does not change
  //      while we are obtaining its value.


    //bool RingBuf<ET, S, IT, BT>::lockedPop(ET & outElement)
    //{
    //    noInterrupts();
    //    bool result = pop(outElement);
    //    interrupts();
    //    return result;
    //}

    //bool RingBuf<ET, S, IT, BT>::pop(ET & outElement)
    //{
    //    if (isEmpty()) return false;
    //    outElement = mBuffer[mReadIndex];
    //    mReadIndex++;
    //    mSize--;
    //    if (mReadIndex == S) mReadIndex = 0;
    //    return true;
    //}

  //if ( m_process_it )
  
 // { // get the bytes in the fully filled buffer

    // byte oldSREG = SREG;   // Save the AVR status register, which includes interrupt status;
                            // remember if interrupts are on or off
     noInterrupts();   // turn interrupts off to make the following block a critical section
     
    //for (int i = 0; i < NUMPIXELS1; i++)
    //{
      byte R, G, B;
      uint16_t mSize;
      uint16_t mReadIndex;

      // Extract one LED chain (one frame ) from the ring buffer

      // check if the current size of the buffer is greater than the one LED frame

      if (m_buffer.readIndex()  > m_LEDChainByteSize) return;

      for (int i = 0; i < m_LEDChainByteSize; i++ ) 
      {
      m_buffer.pop(R);

      m_buffer.pop(G);
      m_buffer.pop(B);

      //strip.setPixelColor(i, R, G, B );

      Serial.print(i);
      Serial.print("th  ");
      Serial.print("  r:  ");
      Serial.print(m_buffer[i * 3 + 0]);
      Serial.print("  g:  ");

      Serial.print(m_buffer[i * 3 + 1]);
      Serial.print("  b:  ");
      Serial.println(m_buffer[i * 3 + 2]);
    }

    interrupts();
    //SREG = oldSREG;    // turn interrupts back on, if they were on before

    //comBuffer.push
    //if (comBuffer.isFull()) digitalWrite(LED_BUILTIN, LOW);
    //if (comBuffer.lockedPop(pushDate)) {

   //Chainable: Each Pixie listens on it Din pin for serial data. It will consume the FIRST  3 bytes it sees and store them. 
   // [ The last pixie (Rn,Gn,Bn) will receive its own pixel Data first time and will not relay anything.]  It will then echo
     //any subsequent bytes to its Dout pin (with less than a microsecond latency). It will keep doing so until it detects a 1mslong silence on Din. 
     // Then, it will immediately apply (latch) the color values it got and go back to listening for a new
     // color. 
    // => Dn should see 1 ms long silence on Din to  not to relay the data on the chain any more,
    // so that the current LED frame is completed. This 1ms  delay is done within strip.show();

// ==>  Each Pixie in the chain consumes its own data, then relays the rest of the data down the chain,
//  so the controller can control each Pixie individually, without being connected to each one.

// 3W: 3W: 3 Watts of power drive the LED, or 1 Watt for each Red, Green, Blue. This is a VERY bright LED. 
//     Compare to typical NeoPixels, which are around 0.2W.

    strip.show(); //  // show() Waits for 1ms elapsed since prior call; Then write the led pixel array as a stream to the physical chain of LEDs.
    
// SSmart: Some really high-end features are available on each Pixie, such as Gamma correction 
// (8-bit to 16-bit) for super-smooth color gradients, 
//   over-heating protection (these things do get hot if left on at full blast for too long), communication loss protection.
     
    m_process_it = false;
    
  } //if ( m_process_it )
  
 
}// loop()

// SPI interrupt routine
//https://stackoverflow.com/questions/54152172/should-i-retrieve-spi-data-in-interrupt-routine
//Non-interrupt level code must lock out interrupts to ensure that data structures 
//and I/O are not interfered with by an interrupt handler
// Suppose you keep a linked list that you use outside of the interrupt handler,
//and add things to it inside the interrupt handler. If an interrupt happens at just the wrong time
//you'll corrupt the list structure unless you lock out interrupts when you manipulate or traverse it.

//You want to minimize the amount of time that you lock out interrupts:
// locking them out can interfere with timing and lead to dropped data.

//We usually use a simple variable called a "semaphore" to indicate that an interrupt happened.
//The interrupt handler may increment the semaphore 
//and the non-interrupt level processing code may decrement it.
//As long as the compiler can manage the semaphore in a single instruction it will be safe from corruption.
//
//In C, we need to declare the semaphore variable as volatile, which lets the C compiler know
//that it may change at any time. The compiler then generates code that doesn't depend 
//on the variable's value remaining the same between operations.
//
//volatile int got_interrupt = 0;
//An interrupt handler would simply increment got_interrupt and the code 
//running outside of the interrupt handler would inspect it and, if it was non-zero,
//would decrement it and take whatever actions were needed


//http://gammon.com.au/interrupts
//Keep it short
//Don't use delay ()
//Don't do serial prints
//Make variables shared with the main code volatile
//Variables shared with main code may need to be protected by "critical sections" (see below)
//Don't try to turn interrupts off or on

//Uses of Interrupt:
//To detect pin changes(eg.rotary encoders, button presses)
//Watchdog timer(eg. if nothing happens after 8 seconds, interrupt me)
//Timer interrupts - used for comparing / overflowing timers
//SPI data transfers
//I2C data transfers
//USART data transfers *** Serial Comm on serial ports uses interrupt
//ADC conversions(analog to digital)
//EEPROM ready for use
//Flash memory ready

//http://gammon.com.au/forum/?id=10892
//http://gammon.com.au/interrupts
//// SPI interrupt routine
//18  SPI Serial Transfer Complete                    (SPI_STC_vect)
ISR (SPI_STC_vect) { // SPI_STC_vect: invoked when SPI data arrives:

  byte c = SPDR;  // grab byte from SPI Data Register

 // if ( !m_process_it) { // ignore the incoming byte while loop() is processing the fully filled buffer
                        // m_process_it becomes false when loop() finishes processing the filled buffer
    // not m_process has two cases:
  if (!m_frameInProgess) { // when frameInProfess is false, try to check the start byte, 255


      if (c == 255) { // 255 is the start byte sent by the master
          m_TailToBuffer  = true;
          m_frameCompleted = false;
          return;
      }
      else if ( m_frameInitialized && c == '\n')
      {
          m_frameCompleted = true;
          return;
      }
      else
      {
          m_buffer.push(c);
      }

  }//if ( !m_frameInProgress )

  
} // ISR



//cd:
//void Adafruit_Pixie::setPixelColor(uint16_t n, uint8_t r, uint8_t g,
//                                   uint8_t b) {
//  if (n < numLEDs) {
//    uint8_t *p = &pixels[n * 3];
//    p[0] = r;
//    p[1] = g;
//    p[2] = b;
//  }
//}

//void Adafruit_Pixie::show() {
//  if (pixels) {
//    uint16_t n3 = numLEDs * 3;
//    while (!canShow())
//      ;                // Wait for 1ms elapsed since prior call
//    if (!brightness) { // No brightness adjust, output full blast

//      stream->write(pixels, n3); //  stream  = pixieSerial, where SoftwareSerial pixieSerial(-1, PIXIEPIN)

//    } else { // Scale back brightness for every pixel R,G,B:
//      uint16_t i, b16 = (uint16_t)brightness;
//      for (i = 0; i < n3; i++) {
//        stream->write((pixels[i] * b16) >> 8);
//      }
//    }
//    endTime = micros(); // Save EOD time for latch on next call
//  }
//}

//https://garretlab.web.fc2.com/en/arduino/inside/hardware/arduino/avr/cores/arduino/wiring.c/millis.html
// Example of crtical section:

//The millis() is defined in hardware / arduino / avr / cores / arduino / wiring.c as below.Only the souce code for Arduino UNO is quoted.The original source code supports many tips using #if¡¯s.
//
//volatile unsigned long timer0_millis = 0;
//
//unsigned long millis()
//{
//    unsigned long m;
//    uint8_t oldSREG = SREG;
//
//    // disable interrupts while we read timer0_millis or we might get an
//    // inconsistent value (e.g. in the middle of a write to timer0_millis)
//    cli();
//    m = timer0_millis;
//    SREG = oldSREG;
//
//    return m;
//}
//The millis() has no input argumentand returns unsigned long value.
//
//First it reserves the status register SREG.

//unsigned long millis()
//{
//    unsigned long m;
//    uint8_t oldSREG = SREG;
//    The SREG is a status register of ATMega328P.
//
//        Next it disables interrupt, copies the timer0_millis which holds the number of milliseconds to m, restores the reserved status register valueand returns m.The timer0_millis is set in a interrupt handler named TIMER0_OVF_vect.
//        // disable interrupts while we read timer0_millis or we might get an
//        // inconsistent value (e.g. in the middle of a write to timer0_millis)
//        cli();
//    m = timer0_millis;
//    SREG = oldSREG;
//
//    return m;
//}
//cli() is a macro that executes an assembler macro to disable interrupt.
//
//As SREG includes interrupt flag, it is not need to enable interrupt explicitly.