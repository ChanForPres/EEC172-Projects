/*************************************************** 
  This is a library for the 1.5" & 1.27" 16-bit Color OLEDs 
  with SSD1331 driver chip
  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1431
  ------> http://www.adafruit.com/products/1673
  These displays use SPI to communicate, 4 or 5 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!
  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/
#include "stdint.h"
#include "stdbool.h"

#define SSD1351WIDTH 128
#define SSD1351HEIGHT 128  // SET THIS TO 96 FOR 1.27"!
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))

//define from ino.h
#define sclk 2
#define mosi 3
#define dc   4
#define cs   5
#define rst  6

// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF



/*
#define swap(a, b) { unsigned int t = a; a = b; b = t; }

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

*/
/*
#ifdef __SAM3X8E__
    typedef volatile RwReg PortReg;
    typedef unsigned int PortMask;
#else
    typedef volatile unsigned char PortReg;
    typedef unsigned char PortMask;
#endif
*/
//typedef volatile unsigned char PortReg;
//typedef unsigned char PortReg;
//typedef unsigned char PortMask;
// Select one of these defines to set the pixel color order
#define SSD1351_COLORORDER_RGB
// #define SSD1351_COLORORDER_BGR

#if defined SSD1351_COLORORDER_RGB && defined SSD1351_COLORORDER_BGR
  #error "RGB and BGR can not both be defined for SSD1351_COLORODER."
#endif

// Timing Delays
#define SSD1351_DELAYS_HWFILL	    (3)
#define SSD1351_DELAYS_HWLINE       (1)

// SSD1351 Commands
#define SSD1351_CMD_SETCOLUMN 		0x15
#define SSD1351_CMD_SETROW    		0x75
#define SSD1351_CMD_WRITERAM   		0x5C
#define SSD1351_CMD_READRAM   		0x5D
#define SSD1351_CMD_SETREMAP 		0xA0
#define SSD1351_CMD_STARTLINE 		0xA1
#define SSD1351_CMD_DISPLAYOFFSET 	0xA2
#define SSD1351_CMD_DISPLAYALLOFF 	0xA4
#define SSD1351_CMD_DISPLAYALLON  	0xA5
#define SSD1351_CMD_NORMALDISPLAY 	0xA6
#define SSD1351_CMD_INVERTDISPLAY 	0xA7
#define SSD1351_CMD_FUNCTIONSELECT 	0xAB
#define SSD1351_CMD_DISPLAYOFF 		0xAE
#define SSD1351_CMD_DISPLAYON     	0xAF
#define SSD1351_CMD_PRECHARGE 		0xB1
#define SSD1351_CMD_DISPLAYENHANCE	0xB2
#define SSD1351_CMD_CLOCKDIV 		0xB3
#define SSD1351_CMD_SETVSL 		0xB4
#define SSD1351_CMD_SETGPIO 		0xB5
#define SSD1351_CMD_PRECHARGE2 		0xB6
#define SSD1351_CMD_SETGRAY 		0xB8
#define SSD1351_CMD_USELUT 		0xB9
#define SSD1351_CMD_PRECHARGELEVEL 	0xBB
#define SSD1351_CMD_VCOMH 		0xBE
#define SSD1351_CMD_CONTRASTABC		0xC1
#define SSD1351_CMD_CONTRASTMASTER	0xC7
#define SSD1351_CMD_MUXRATIO            0xCA
#define SSD1351_CMD_COMMANDLOCK         0xFD
#define SSD1351_CMD_HORIZSCROLL         0x96
#define SSD1351_CMD_STOPSCROLL          0x9E
#define SSD1351_CMD_STARTSCROLL         0x9F

/*
class Adafruit_SSD1351  : public virtual Adafruit_GFX {
 public:
  Adafruit_SSD1351(unsigned char CS, unsigned char RS, unsigned char SID, unsigned char SCLK, unsigned char RST);
  Adafruit_SSD1351(unsigned char CS, unsigned char RS, unsigned char RST);

  unsigned int Color565(unsigned char r, unsigned char g, unsigned char b);
*/

  // drawing primitives!
 void setRotation(unsigned int x);
 unsigned char getRotation(void);
	void Adafruit_SSD1351_Init(int w, int h);
  void swap(uint16_t *a, uint16_t *b);

	//void drawPixel(int16_t x, int16_t y, uint16_t color);
	void drawPixel(uint16_t x, uint16_t y, uint16_t  color);
  void fillRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  void fillScreen(uint16_t fillcolor);
void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void  invert(bool v);
  // commands
  void begin(void);
  void goTo(int x, int y);

  void reset(void);
void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
void setTextColor(uint16_t c);
void setTextSize(uint8_t s) ;
void setCursor(int16_t x, int16_t y) ;
  /* low level */


  void writeData(uint8_t d);
  void writeCommand(uint8_t c);
  void spiwrite(uint8_t);
  void drawFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t fillcolor);

//Test functions
void delay(int n);
//void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) ;
	
int16_t abs(int16_t x);
int16_t width(void);
int16_t height(void);

void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void drawCircleHelper( int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);
void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color); 
void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);
void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,int16_t x2, int16_t y2, uint16_t color);
//void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,int16_t x2, int16_t y2, uint16_t color);
void fillTriangle ( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h,
      int16_t radius, uint16_t color);


//functions from ino.c
void testlines(uint16_t color) ;
void testdrawtext(char *text, uint16_t color) ;
void testfastlines(uint16_t color1, uint16_t color2) ;
void testdrawrects(uint16_t color) ;
void testfillrects(uint16_t color1, uint16_t color2) ;
void testfillcircles(uint8_t radius, uint16_t color) ;
void testdrawcircles(uint8_t radius, uint16_t color) ;
void testtriangles(void) ;
void testroundrects(void);
