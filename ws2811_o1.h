#include "config.h"

// based on code and docs here: http://wp.josh.com/2014/05/13/ws2812-neopixels-are-not-so-finicky-once-you-get-to-know-them/, but delay mechanism changed somewhat significantly.  both simpler, and works on Arduino unlike code in the article.  
// need some code to calculate timings, below work well currently, but glitch occasionally

// These are the timing constraints taken mostly from the WS2812 datasheets
// These are chosen to be conservative and avoid problems rather than for maximum throughput
// these defines not currently used
#define T1H  900    // Width of a 1 bit in ns
#define T1L  600    // Width of a 1 bit in ns
#define T0H  400    // Width of a 0 bit in ns
#define T0L  900    // Width of a 0 bit in ns
 
// dependant on MCU and possibly compiler version - tested with atmega32u4 in brushed alienwii v2.3 with Arduino 1.0.6
#define T1H_delay asm("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"); // 14 cycles - 2 overhead = 12
#define T1L_delay asm("nop\n"); // 9 cycles - 10 overhed = 0, but this delay appears necessary
#define T0H_delay asm("nop\n nop\n nop\n nop\n "); // 6 cycles - 2 overhead = 4
#define T0L_delay asm("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"); // 14 cycles - 2 overhead = 12
 
void sendBit(bool) __attribute__ ((optimize(0)));  // does this do anything? works, so...
void sendBit(bool bit) {
	if (bit) {      // 1-bit
		LED_WS2811_PORT |= (1 << LED_WS2811_PIN);
		T1H_delay
		LED_WS2811_PORT &= ~(1 << LED_WS2811_PIN);
		T1L_delay
	 } else {             // 0-bit
		cli(); // this delay is short, 390ns, we suspend interrupts as too long here is unacceptable
		LED_WS2811_PORT |= (1 << LED_WS2811_PIN);
		T0H_delay
		LED_WS2811_PORT &= ~(1 << LED_WS2811_PIN);
		sei();
		T0L_delay
	 }
}

void sendByte(uint8_t byte) {
    for( unsigned char bit = 0 ; bit < 8 ; bit++ ) {
		sendBit( bitRead( byte , 7 ) ); // bits in highest-to-lowest order
		byte <<= 1;
     }
}

// may need to change order depending on the leds, this works for me 
void ws2811_setled(uint8_t r, uint8_t g , uint8_t b ) {
	sendByte(r);
	sendByte(g);
	sendByte(b);
}
 
void led_ws2811_init(void) {
	LED_WS2811_DDR |= (1 << LED_WS2811_PIN);  //configure pin as output
	LED_WS2811_PORT &= ~(1 << LED_WS2811_PIN);
	delay(2); // latch low, probably not necessary but only happens once, during setup()
}

