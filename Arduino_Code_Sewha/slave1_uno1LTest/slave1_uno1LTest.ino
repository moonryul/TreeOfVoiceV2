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

#define NUMPIXELS1 30 // Number of Pixies in the strip
#define PIXIEPIN  5 // Pin number for SoftwareSerial output to the LED chain

SoftwareSerial pixieSerial(-1, PIXIEPIN);
Adafruit_Pixie strip = Adafruit_Pixie(NUMPIXELS1, &pixieSerial);

const int m_bufferSize = NUMPIXELS1 * 3; // 6 bytes are for the start and end bytes
byte m_buffer[m_bufferSize];

volatile int m_pos = 0;
volatile boolean m_process_it = false;
volatile boolean m_frameInProgess = false;

void setup() {
  Serial.begin(57600);
  pixieSerial.begin(115200); // Pixie REQUIRES this baud rate; begin() executes listen() within it.

  //SPI.setClockDivider(SPI_CLOCK_DIV16);
  pinMode(PIXIEPIN, OUTPUT);
  pinMode(MISO, OUTPUT); // this is needed to send bytes to the master

  SPCR |= _BV(SPE); // SPE: SPI Enable; Macro _BV(n) sets bit n to 1, whereas the other bits to 0
  SPCR &= ~_BV(MSTR); // MSTR = Master Slave Select Register; MSTR = 1 => Set as master
  // turn on interrupts
  SPCR |= _BV(SPIE); // SPIE: SPI Interrrupt Enable
}

// SPI interrupt routine
ISR (SPI_STC_vect) { // SPI_STC_vect: invoked when data arrives:

  byte c = SPDR;  // grab byte from SPI Data Register

  if ( !m_process_it) { // ignore the incoming byte while loop() is processing the fully filled buffer
                        // m_process_it becomes false when loop() finishes processing the filled buffer
    // !m_process has two cases:
    if ( !m_frameInProgess ) { // when frameInProfess is false, try to check the start byte, 255


      if ( c == 255 ) { // 255 is the start byte sent by the master
        m_pos = m_bufferSize - 1; // insert bytes starting from the end of the buffer, so that the end parts of the LEDs go into 
                                  // the chain first??? questions by Moon Jung
        m_frameInProgess = true;
        return;
      }

//      
//      if ( c == 255 ) {
//        m_pos = 0;
//        m_frameInProgess = true;
//        return;
//      }
      else { // continue to read until the start byte arrives while frameInPgoress is false
        return;
      }
    }//if ( !m_frameInProgress )

    else  { // m_frameInProgess is true: the start byte has arrived

      m_buffer[m_pos] = c;
//
//      if ( m_pos == m_bufferSize - 1) {
//        m_process_it = true;
//        m_frameInProgess = false;
//        return;
//      } // if


      if ( m_pos == 0) { // the buffer is full
        m_process_it = true;
        m_frameInProgess = false;
        return;
      } // if
      
      else {
        m_pos--; // the next location to fill in the buffer
        return;
      }

//       else {
//        m_pos ++; // the next location to fill in the buffer
//        return;
//      }

      
    } // else: m_frameInProgess is true: the start byte has arrived

  } //  if ( !m_process_it )

  // now the process_it is true: loop() is reading the buffer => ignore the incoming byte
   // until loop() has read the buffer and turns m_process_it false
  return;

} // ISR

void loop() {

  // The following process of reading the sending buffer can be interrupted when new bytes arrive at the SPI port
  
  if ( m_process_it )
  
  { // get the bytes in the fully filled buffer

    for (int i = 0; i < NUMPIXELS1; i++)
    {
      strip.setPixelColor(i, m_buffer[ i * 3 + 0], m_buffer[i * 3 + 1], m_buffer[i * 3 + 2] );
      Serial.print(i);
      Serial.print("th  ");
      Serial.print("  r:  ");
      Serial.print(m_buffer[i * 3 + 0]);
      Serial.print("  g:  ");

      Serial.print(m_buffer[i * 3 + 1]);
      Serial.print("  b:  ");
      Serial.println(m_buffer[i * 3 + 2]);
    }


     delay(2); // cf: https://learn.adafruit.com/pixie-3-watt-smart-chainable-led-pixels/

   //Chainable: Each Pixie listens on it Din pin for serial data. It will consume the FIRST  3 bytes it sees and store them. 
   // [ The last pixie (Rn,Gn,Bn) will receive its own pixel Data first time and will not relay anything.]  It will then echo
     //any subsequent bytes to its Dout pin (with less than a microsecond latency). It will keep doing so until it detects a 1mslong silence on Din. 
     // Then, it will immediately apply (latch) the color values it got and go back to listening for a new
     // color. => Dn should see 1 ms long silence on Din to terminate the current frame, not to relay the data on the chain any more.

// ==>  Each Pixie in the chain consumes its own data, then relays the rest of the data down the chain,
//  so the controller can control each Pixie individually, without being connected to each one.

// 3W: 3W: 3 Watts of power drive the LED, or 1 Watt for each Red, Green, Blue. This is a VERY bright LED. 
//     Compare to typical NeoPixels, which are around 0.2W.

    strip.show(); //  // show() Waits for 1ms elapsed since prior call; Then write the led pixel array as a stream to the physical chain of LEDs.
    
// Smart: Smart: Some really high-end features are available on each Pixie, such as Gamma correction 
// (8-bit to 16-bit) for super-smooth color gradients, 
//   over-heating protection (these things do get hot if left on at full blast for too long), communication loss protection.
     
    m_process_it = false;
    
  } //if ( m_process_it )
  
 
}// loop()

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
