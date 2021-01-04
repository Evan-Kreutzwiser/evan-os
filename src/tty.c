
#include <bootboot.h>

#include <serial.h>
#include <tty.h>

#include <stdint.h>

extern BOOTBOOT bootboot;							  // See ../dist/bootboot.h
extern uint8_t fb;									  // Linear framebuffer mapped here
extern volatile unsigned char _binary_font_psf_start; // Font file

// PSF file header
typedef struct {
	uint32_t magic;
	uint32_t version;
	uint32_t headersize;
	uint32_t flags;
	uint32_t numglyph;
	uint32_t bytesperglyph;
	uint32_t height;
	uint32_t width;
	uint8_t glyphs;
} __attribute__((packed)) psf2_t;

// Font
volatile psf2_t *font = (psf2_t *)&_binary_font_psf_start;

uint32_t char_x, char_y;
uint32_t fg_color = 0x00ffffff, bg_color = 0x00000000;

uint32_t max_x = 50, max_y = 25;

void tty_init() {
	// Set how many characters wide and tall the tty is based on the screen dimensionss
	max_x = bootboot.fb_width / 8;
	max_y = bootboot.fb_height / 16;
}

// Set position
void tty_set_x(uint32_t new_x) {
	char_x = new_x;
}
void tty_set_y(uint32_t new_y) {
	char_y = new_y;
}
// Set colors
void tty_set_fg_color(uint32_t color) {
	fg_color = color;
}
void tty_set_bg_color(uint32_t color) {
	bg_color = color;
}
// Get colors
uint32_t tty_get_fg_color(void) {
	return fg_color;
}
uint32_t tty_get_bg_color(void) {
	return bg_color;
}

void tty_put_char_at(char c, uint32_t xpos, uint32_t ypos) {

	// Make a pointer the the screen's framebuffer
	volatile uint8_t* pixeladdress;
	// Scanline width
	uint32_t scanline = bootboot.fb_scanline;
	// Bit mask
	uint8_t mask;

	// Make a pointer to the data for the glyph to print
	volatile uint8_t* glyph = (uint8_t *)&_binary_font_psf_start + font->headersize + c * font->bytesperglyph;

	for (uint32_t y = 0; y < font->height; y++) {
		mask = 0x80; // Set which bit to print
		// Set the address of the pixel to plot
		pixeladdress = (uint8_t *)&fb + (scanline * (y + ypos)) + (xpos * 4);

		for (uint32_t x = 0; x < font->width; x++) {
			// Plot a pixel of the glyph
			*((uint32_t *)(pixeladdress)) = /* Address of the pixel in the frambuffer (fb) */
				((uint8_t)*glyph) & (mask)	/* Check the value of the bit in the glyph */
					? fg_color : bg_color;	   // Set the color based on the bit's value
			mask >>= 1;		   // Shift the mask 1 bit to the left to prepare to print the next pixel
			pixeladdress += 4; // Move the address that gets colored to the next byte
		}
		glyph++; // Select Next
	}
}

void tty_print_char(char c) {

	serial_write(c); // Output the character to the serial output

	switch (c) {
		case '\n':
			char_x = 0;
			char_y++;
			break;
		default:
			tty_put_char_at(c, (char_x*font->width), char_y*font->height);
			char_x++;
			break;
		}
		
		if (char_x >= max_x) {
			char_x = 0;
			char_y++;
		}
}

void tty_print_string(char *s) {
	
	// The index of the chracter in the string
	uint32_t c = 0;

	while (s[c] != 0x0) {
		// Print the character
		tty_print_char(s[c]);
		// Select the next character
		c++;
	}
}

void puts_at_pos(char *s, uint32_t xpos, uint32_t ypos) {

	// The index of the chracter in the string
	uint32_t c = 0;

	// Continue until a null terminator is hit
	while (s[c] != 0x0) {
		// Print the character
		tty_put_char_at(s[c], xpos+(c*font->width), ypos);
		c++; // Select the next character
	}
}

void puts_at_pos_transparent(char *s, uint32_t xpos, uint32_t ypos) {
	// Make a pointer the the screen's framebuffer
	volatile uint8_t *pixeladdress;
	// Width of one line of pixels on the screen
	uint32_t scanline = bootboot.fb_scanline;
	// Pointer the glyph data
	volatile uint8_t *glyph;
	uint8_t mask;
	uint32_t c = 0;			  // The index of the chracter in the string
	uint32_t fg = 0x00ffffff; // Colors

	// Continue until a null terminator is hit
	while (s[c] != 0x0) {
		// Make a pointer to the data for the glyph to print
		glyph = (uint8_t *)&_binary_font_psf_start + font->headersize + (s[c] * font->bytesperglyph);

		for (uint32_t y = 0; y < font->height; y++) {
			mask = 0x80; // Set which bit to print
			// Set the address of the pixel to plot
			pixeladdress = (uint8_t *)&fb + (scanline * (y + ypos)) + (xpos * 4) + (c * (font->width * 4));

			for (uint32_t x = 0; x < font->width; x++) {
				// If the bit is set
				if (((uint8_t)*glyph) & (mask)) {
					// Plot a pixel of the glyph
					*((uint32_t *)(pixeladdress)) = fg;
				}
				mask >>= 1;		   // Shift the mask 1 bit to the left to prepare to print the next pixel
				pixeladdress += 4; // Move the address that gets colored to the next byte
			}
			glyph++; // Select Next
		}
		c++; // Select the next character
	}
}