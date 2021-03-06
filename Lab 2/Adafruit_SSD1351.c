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
 
#include "Adafruit_SSD1351.h"
#include "glcdfont.h"
#include "driverlib/ssi.h"
#include "driverlib/rom.h"

#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"

#ifdef __AVR__
    #include <avr/pgmspace.h>
#endif


#ifndef _BV
    #define _BV(bit) (1<<(bit))
#endif

typedef volatile unsigned char PortReg;
typedef unsigned char PortMask;

unsigned char _cs, _rs, _rst, _sid, _sclk;
  PortReg *csport, *rsport, *sidport, *sclkport;
  PortMask cspinmask, rspinmask, sidpinmask, sclkpinmask;

extern int WIDTH,HEIGHT;
extern int _width  , _height  ;
extern int rotation;
extern int cursor_y ,cursor_x ; 
extern int textsize ;
extern int textcolor ,textbgcolor;
extern int wrap   ;


 void Adafruit_SSD1351_Init(int w, int h){
	 WIDTH =w;
	 HEIGHT =h;
	 _width    = WIDTH;
   _height   = HEIGHT;
	 cursor_y =0;
	 cursor_x = 0;
	 rotation =0;
	 textsize =1;
	 textcolor =0xFFFF;
	 textbgcolor = 0xFFFF;
	 wrap =1; //wrap is true
 }

 void reset(){
 int i;
	
	 ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5,0x00); //deassert PB5/Reset
		for( i =0;i<96;i++){} // wait for 6us
			ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, GPIO_PIN_5);//assert Reset/PB5
 }
 
void swap(uint16_t *a, uint16_t *b){
		uint16_t t;
		t=*a;
		*a=*b;
		*b=t;
	}

	
	void setRotation(unsigned int x) {
  x %= 4;  // cant be higher than 3
  rotation = x;
  switch (x) {
  case 0:
  case 2:
    _width = WIDTH;
    _height = HEIGHT;
    break;
  case 1:
  case 3:
    _width = HEIGHT;
    _height = WIDTH;
    break;
  }
}

unsigned char getRotation(void) {
  rotation %= 4;
  return rotation;
}

// low level pin interface 
    void spiwrite(uint8_t c) {
    
    char i;
    
    *sclkport |= sclkpinmask;
    
    for (i=7; i>=0; i--) {
        *sclkport &= ~sclkpinmask;
        //SCLK_PORT &= ~_BV(SCLK);
        
        if (c & _BV(i)) {
            *sidport |= sidpinmask;
            //digitalWrite(_sid, HIGH);
            //SID_PORT |= _BV(SID);
        } else {
            *sidport &= ~sidpinmask;
            //digitalWrite(_sid, LOW);
            //SID_PORT &= ~_BV(SID);
        }
        
        *sclkport |= sclkpinmask;
        //SCLK_PORT |= _BV(SCLK);
    }
}

void goTo(int x, int y) {
  if ((x >= SSD1351WIDTH) || (y >= SSD1351HEIGHT)) return;
  

  writeCommand(SSD1351_CMD_SETCOLUMN);
  writeData(x);
  writeData(SSD1351WIDTH-1);

  writeCommand(SSD1351_CMD_SETROW);
  writeData(y);
  writeData(SSD1351HEIGHT-1);

  writeCommand(SSD1351_CMD_WRITERAM);  
}

unsigned int Color565(unsigned char r, unsigned char g, unsigned char b) {
  unsigned int c;
  c = r >> 3;
  c <<= 6;
  c |= g >> 2;
  c <<= 5;
  c |= b >> 3;

  return c;
}
   void writeData(uint8_t d){
	   ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_PIN_4);
	   SSIDataPut(SSI0_BASE, d);		 
}
 void writeCommand(uint8_t d){
     while(SSIBusy(SSI0_BASE)){}
	     ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, 0x00);
	     SSIDataPut(SSI0_BASE, d);
	     while(SSIBusy(SSI0_BASE)){}	 
 }
void fillScreen(uint16_t fillcolor)
{
  fillRect(0, 0, SSD1351WIDTH, SSD1351HEIGHT, fillcolor);
}

// Draw a filled rectangle with no rotation.
void drawFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t fillcolor)
 {
	
	unsigned int i;
  // Bounds check
  if ((x >= SSD1351WIDTH) || (y >= SSD1351HEIGHT))
    return;

  // Y bounds check
  if (y+h > SSD1351HEIGHT)
  {
    h = SSD1351HEIGHT - y - 1;
  }

  // X bounds check
  if (x+w > SSD1351WIDTH)
  {
    w = SSD1351WIDTH - x - 1;
  }
  
  // set location
  writeCommand(SSD1351_CMD_SETCOLUMN);
  writeData(x);
  writeData(x+w-1);
  writeCommand(SSD1351_CMD_SETROW);
  writeData(y);
  writeData(y+h-1);
  // fill!
  writeCommand(SSD1351_CMD_WRITERAM);  

  for ( i=0; i < w*h; i++) {
    writeData(fillcolor >> 8);
    writeData(fillcolor);
  }
}


void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t fillcolor) {
  // Transform x and y based on current rotation.
  switch (getRotation()) {
  case 0:  // No rotation
    drawFillRect(x, y, w, h, fillcolor);
    break;
  case 1:  // Rotated 90 degrees clockwise.
    //swap(x, y);
	swap(&x, &y);
    x = WIDTH - x - h;
    drawFillRect(x, y, h, w, fillcolor);
    break;
  case 2:  // Rotated 180 degrees clockwise.
    x = WIDTH - x - w;
    y = HEIGHT - y - h;
    drawFillRect(x, y, w, h, fillcolor);
    break;
  case 3:  // Rotated 270 degrees clockwise.
  //  swap(x, y);
	swap(&x, &y);
    y = HEIGHT - y - w;
    drawFillRect(x, y, h, w, fillcolor);
    break;
  }
}

// Draw a horizontal line ignoring any screen rotation.
void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
	unsigned int i;
  // Bounds check
  if ((x >= SSD1351WIDTH) || (y >= SSD1351HEIGHT))
    return;

  // X bounds check
  if (x+w > SSD1351WIDTH)
  {
    w = SSD1351WIDTH - x - 1;
  }

  if (w < 0) return;

  // set location
  writeCommand(SSD1351_CMD_SETCOLUMN);
  writeData(x);
  writeData(x+w-1);
  writeCommand(SSD1351_CMD_SETROW);
  writeData(y);
  writeData(y);
  // fill!
  writeCommand(SSD1351_CMD_WRITERAM);  

  for (i=0; i < w; i++) {
    writeData(color >> 8);
    writeData(color);
  }
}

void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) 
 {
	unsigned int i;
  // Bounds check
  if ((x >= SSD1351WIDTH) || (y >= SSD1351HEIGHT))
  return;

  // X bounds check
  if (y+h > SSD1351HEIGHT)
  {
    h = SSD1351HEIGHT - y - 1;
  }

  if (h < 0) return;

  // set location
  writeCommand(SSD1351_CMD_SETCOLUMN);
  writeData(x);
  writeData(x);
  writeCommand(SSD1351_CMD_SETROW);
  writeData(y);
  writeData(y+h-1);
  // fill!
  writeCommand(SSD1351_CMD_WRITERAM);  

  for ( i=0; i < h; i++) {
    writeData(color >> 8);
    writeData(color);
  }
}
 

void drawPixel(uint16_t x, uint16_t y, uint16_t  color)
{
  // Transform x and y based on current rotation.
  switch (getRotation()) {
  // Case 0: No rotation
  case 1:  // Rotated 90 degrees clockwise.
    //swap(x, y);
	swap(&x, &y);
    x = WIDTH - x - 1;
    break;
  case 2:  // Rotated 180 degrees clockwise.
    x = WIDTH - x - 1;
    y = HEIGHT - y - 1;
    break;
  case 3:  // Rotated 270 degrees clockwise.
    //swap(x, y);
	swap(&x, &y);
    y = HEIGHT - y - 1;
    break;
  }

  // Bounds check.
  if ((x >= SSD1351WIDTH) || (y >= SSD1351HEIGHT)) return;
  if ((x < 0) || (y < 0)) return;

  goTo(x, y);
  
  // setup for data
  *rsport |= rspinmask;
  *csport &= ~ cspinmask;
	
//  spiwrite(color >> 8);    
 // spiwrite(color);
	

  
    writeData(color >> 8);
    writeData(color);
  
  
 // *csport |= cspinmask;
}

//boolean v
void  invert(bool v) {
   if (v) {
     writeCommand(SSD1351_CMD_INVERTDISPLAY);
   } else {
     	writeCommand(SSD1351_CMD_NORMALDISPLAY);
   }
 }

void begin(void) {
    writeCommand(SSD1351_CMD_COMMANDLOCK);  // set command lock
    writeData(0x12);  
    writeCommand(SSD1351_CMD_COMMANDLOCK);  // set command lock
    writeData(0xB1);

    writeCommand(SSD1351_CMD_DISPLAYOFF);  		// 0xAE

    writeCommand(SSD1351_CMD_CLOCKDIV);  		// 0xB3
    writeCommand(0xF1);  						// 7:4 = Oscillator Frequency, 3:0 = CLK Div Ratio (A[3:0]+1 = 1..16)
    
    writeCommand(SSD1351_CMD_MUXRATIO);
    writeData(127);
    
    writeCommand(SSD1351_CMD_SETREMAP);
    writeData(0x74);
  
    writeCommand(SSD1351_CMD_SETCOLUMN);
    writeData(0x00);
    writeData(0x7F);
    writeCommand(SSD1351_CMD_SETROW);
    writeData(0x00);
    writeData(0x7F);

    writeCommand(SSD1351_CMD_STARTLINE); 		// 0xA1
    if (SSD1351HEIGHT == 96) {
      writeData(96);
    } else {
      writeData(0);
    }
    writeCommand(SSD1351_CMD_DISPLAYOFFSET); 	// 0xA2
    writeData(0x0);

    writeCommand(SSD1351_CMD_SETGPIO);
    writeData(0x00);
    
    writeCommand(SSD1351_CMD_FUNCTIONSELECT);
    writeData(0x01); // internal (diode drop)
    //writeData(0x01); // external bias

//    writeCommand(SSSD1351_CMD_SETPHASELENGTH);
//    writeData(0x32);

    writeCommand(SSD1351_CMD_PRECHARGE);  		// 0xB1
    writeCommand(0x32);
 
    writeCommand(SSD1351_CMD_VCOMH);  			// 0xBE
    writeCommand(0x05);

    writeCommand(SSD1351_CMD_NORMALDISPLAY);  	// 0xA6

    writeCommand(SSD1351_CMD_CONTRASTABC);
    writeData(0xC8);
    writeData(0x80);
    writeData(0xC8);

    writeCommand(SSD1351_CMD_CONTRASTMASTER);
    writeData(0x0F);

    writeCommand(SSD1351_CMD_SETVSL );
    writeData(0xA0);
    writeData(0xB5);
    writeData(0x55);
    
    writeCommand(SSD1351_CMD_PRECHARGE2);
    writeData(0x01);
    
    writeCommand(SSD1351_CMD_DISPLAYON);   
}

void drawChar(int16_t x, int16_t y, unsigned char c,
			    uint16_t color, uint16_t bg, uint8_t size) {
int8_t i,j;
uint8_t line;
						
  if((x >= _width)            || // Clip right
     (y >= _height)           || // Clip bottom
     ((x + 6 * size - 1) < 0) || // Clip left
     ((y + 8 * size - 1) < 0))   // Clip top
    return;

  for ( i=0; i<6; i++ ) {
//    uint8_t line;
    if (i == 5) 
      line = 0x0;
    else 
      line = pgm_read_byte(font+(c*5)+i);
    for ( j = 0; j<8; j++) {
      if (line & 0x1) {
        if (size == 1) // default size
          drawPixel(x+i, y+j, color);
        else {  // big size
          fillRect(x+(i*size), y+(j*size), size, size, color);
        } 
      } else if (bg != color) {
        if (size == 1) // default size
          drawPixel(x+i, y+j, bg);
        else {  // big size
          fillRect(x+i*size, y+j*size, size, size, bg);
        }
      }
      line >>= 1;
    }
  }
}
void delay(int n){
 int i;
	for(i=0;i<16*n;i++){}
}
void setCursor(int16_t x, int16_t y) {
  cursor_x = x;
  cursor_y = y;
}

void setTextSize(uint8_t s) {
  textsize = (s > 0) ? s : 1;
}

void setTextColor(uint16_t c) {
  textcolor = textbgcolor = c;
}

int16_t abs(int16_t x){
	if(x<0) return x*(-1);
	return x;
}

void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
  int16_t steep ;
	int16_t dx, dy;
  int16_t err;
  int16_t ystep;
	int16_t absX,absY;
						
	absY = abs(y1 - y0);
	absX= abs(x1 - x0);
						
//steep = abs(y1 - y0) > abs(x1 - x0);						
	steep = absY > absX;
	
	
	
  
  if (steep) {
		swap(&x0, &y0);
		swap(&x1, &y1);
  }

  if (x0 > x1) {
	swap(&x0, &x1);
    swap(&y0, &y1);
  }

  
  dx = x1 - x0;
  dy = abs(y1 - y0);

  err = dx / 2;
  

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }
  
  dx = x1 - x0;
  dy = abs(y1 - y0);

  err = dx / 2;

  for (; x0<=x1; x0++) {
    if (steep) {
      drawPixel(y0, x0, color);
    } else {
      drawPixel(x0, y0, color);
    }
    
		
		err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
		
  }
}


void drawRect(int16_t x, int16_t y,
			    int16_t w, int16_t h,
			    uint16_t color) {
  drawFastHLine(x, y, w, color);
  drawFastHLine(x, y+h-1, w, color);
  drawFastVLine(x, y, h, color);
  drawFastVLine(x+w-1, y, h, color);
}

void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {

	  int16_t f,ddF_x ,ddF_y ,x, y ;
			f     = 1 - r;
			ddF_x = 1;
			ddF_y = -2 * r;
			x     = 0;
			y     = r;

  drawPixel(x0  , y0+r, color);
  drawPixel(x0  , y0-r, color);
  drawPixel(x0+r, y0  , color);
  drawPixel(x0-r, y0  , color);

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 + x, y0 - y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + y, y0 + x, color);
    drawPixel(x0 - y, y0 + x, color);
    drawPixel(x0 + y, y0 - x, color);
    drawPixel(x0 - y, y0 - x, color);
  }
}
void drawCircleHelper( int16_t x0, int16_t y0,
               int16_t r, uint8_t cornername, uint16_t color) {

 int16_t f,ddF_x ,ddF_y ,x, y ;
			f     = 1 - r;
			ddF_x = 1;
			ddF_y = -2 * r;
			x     = 0;
			y     = r;

								 
  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;
    if (cornername & 0x4) {
      drawPixel(x0 + x, y0 + y, color);
      drawPixel(x0 + y, y0 + x, color);
    } 
    if (cornername & 0x2) {
      drawPixel(x0 + x, y0 - y, color);
      drawPixel(x0 + y, y0 - x, color);
    }
    if (cornername & 0x8) {
      drawPixel(x0 - y, y0 + x, color);
      drawPixel(x0 - x, y0 + y, color);
    }
    if (cornername & 0x1) {
      drawPixel(x0 - y, y0 - x, color);
      drawPixel(x0 - x, y0 - y, color);
    }
  }
}
void fillCircle(int16_t x0, int16_t y0, int16_t r,
			      uint16_t color) {
  drawFastVLine(x0, y0-r, 2*r+1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
}

// Used to do circles and roundrects
void fillCircleHelper(int16_t x0, int16_t y0, int16_t r,
    uint8_t cornername, int16_t delta, uint16_t color) {

  int16_t f,ddF_x ,ddF_y ,x, y ;
			f     = 1 - r;
			ddF_x = 1;
			ddF_y = -2 * r;
			x     = 0;
			y     = r;
  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;

    if (cornername & 0x1) {
      drawFastVLine(x0+x, y0-y, 2*y+1+delta, color);
      drawFastVLine(x0+y, y0-x, 2*x+1+delta, color);
    }
    if (cornername & 0x2) {
      drawFastVLine(x0-x, y0-y, 2*y+1+delta, color);
      drawFastVLine(x0-y, y0-x, 2*x+1+delta, color);
    }
  }
}

void drawRoundRect(int16_t x, int16_t y, int16_t w,
  int16_t h, int16_t r, uint16_t color) {
  // smarter version
  drawFastHLine(x+r  , y    , w-2*r, color); // Top
  drawFastHLine(x+r  , y+h-1, w-2*r, color); // Bottom
  drawFastVLine(x    , y+r  , h-2*r, color); // Left
  drawFastVLine(x+w-1, y+r  , h-2*r, color); // Right
  // draw four corners
  drawCircleHelper(x+r    , y+r    , r, 1, color);
  drawCircleHelper(x+w-r-1, y+r    , r, 2, color);
  drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
  drawCircleHelper(x+r    , y+h-r-1, r, 8, color);
}

// Fill a rounded rectangle
void fillRoundRect(int16_t x, int16_t y, int16_t w,
				 int16_t h, int16_t r, uint16_t color) {
  // smarter version
  fillRect(x+r, y, w-2*r, h, color);

  // draw four corners
  fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
  fillCircleHelper(x+r    , y+r, r, 2, h-2*r-1, color);
}

// Draw a triangle
void drawTriangle(int16_t x0, int16_t y0,
				int16_t x1, int16_t y1,
				int16_t x2, int16_t y2, uint16_t color) {
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);
}

// Fill a triangle
void fillTriangle ( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {

//  int16_t a, b, y, last;
	uint16_t a, b;
	int16_t y, last;
	int16_t  dx01 , dy01 ,dx02 ,dy02 ,dx12 ,dy12 ;
  int32_t sa ,sb ;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    //swap(y0, y1);
		//swap(x0, x1);
		swap(&y0, &y1);
		swap(&x0, &x1);
  }
  if (y1 > y2) {
    //swap(y2, y1); 
		//swap(x2, x1);
		swap(&y2, &y1); 
		swap(&x2, &x1);
  }
  if (y0 > y1) {
    //swap(y0, y1);
		//swap(x0, x1);
		swap(&y0, &y1);
		swap(&x0, &x1);
  }

  if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if(x1 < a)      a = x1;
    else if(x1 > b) b = x1;
    if(x2 < a)      a = x2;
    else if(x2 > b) b = x2;
    drawFastHLine(a, y0, b-a+1, color);
    return;
  }

  
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1;
  
    sa   = 0,
    sb   = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if(y1 == y2) last = y1;   // Include y1 scanline
  else         last = y1-1; // Skip it

  for(y=y0; y<=last; y++) {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(&a,&b);
    drawFastHLine(a, y, b-a+1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y<=y2; y++) {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(&a,&b);
    drawFastHLine(a, y, b-a+1, color);
  }
}
					
int16_t width(void)  {
  return _width;
}
 
int16_t height(void)  {
  return _height;
}


//***************** functions from  ino.c******************


//float p = 3.1415926;

	
void loop() {
}


void testlines(uint16_t color) {
	uint16_t x,y;
	
   fillScreen(BLACK);
   for ( x=0; x < width()-1; x+=6) {
     drawLine(0, 0, x, height()-1, color);
   }
   for (y=0; y < height()-1; y+=6) {
     drawLine(0, 0, width()-1, y, color);
   }
   
   fillScreen(BLACK);
   for (x=0; x < width()-1; x+=6) {
     drawLine(width()-1, 0, x, height()-1, color);
   }
   for (y=0; y < height()-1; y+=6) {
     drawLine(width()-1, 0, 0, y, color);
   }
   
   fillScreen(BLACK);
   for (x=0; x < width()-1; x+=6) {
     drawLine(0, height()-1, x, 0, color);
   }
   for (y=0; y < height()-1; y+=6) {
     drawLine(0, height()-1, width()-1, y, color);
   }

   fillScreen(BLACK);
   for (x=0; x < width()-1; x+=6) {
     drawLine(width()-1, height()-1, x, 0, color);
   }
   for (y=0; y < height()-1; y+=6) {
     drawLine(width()-1, height()-1, 0, y, color);
   }
   
}

void testfastlines(uint16_t color1, uint16_t color2) {
	uint16_t x,y;
   fillScreen(BLACK);
   for (y=0; y < height()-1; y+=5) {
     drawFastHLine(0, y, width()-1, color1);
   }
   for (x=0; x < width()-1; x+=5) {
     drawFastVLine(x, 0, height()-1, color2);
   }
}

void testdrawrects(uint16_t color) {
	uint16_t x;
 fillScreen(BLACK);
 for ( x=0; x < height()-1; x+=6) {
   drawRect((width()-1)/2 -x/2, (height()-1)/2 -x/2 , x, x, color);
 }
}

void testfillrects(uint16_t color1, uint16_t color2) {
	uint16_t x;
	
 fillScreen(BLACK);
 for (x=height()-1; x > 6; x-=6) {
   fillRect((width()-1)/2 -x/2, (height()-1)/2 -x/2 , x, x, color1);
   drawRect((width()-1)/2 -x/2, (height()-1)/2 -x/2 , x, x, color2);
 }
}

void testfillcircles(uint8_t radius, uint16_t color) {
	uint8_t x,y;
  for ( x=radius; x < width()-1; x+=radius*2) {
    for (y=radius; y < height()-1; y+=radius*2) {
      fillCircle(x, y, radius, color);
    }
  }  
}

void testdrawcircles(uint8_t radius, uint16_t color) {
	uint8_t x,y;
  for ( x=0; x < width()-1+radius; x+=radius*2) {
    for (y=0; y < height()-1+radius; y+=radius*2) {
      drawCircle(x, y, radius, color);
    }
  }  
}

void testtriangles() {
 
  int color = 0xFF00ee		;
  int t;
  int w;
  int x;
  int y = 0;
  int z ;
	
	 w = width()/2;
   x = height();
   y = 0;
   z = width();
	 fillScreen(BLACK);
	
  for(t = 0 ; t <= 15; t+=1) {
    drawTriangle(w, y, y, x, z, x, color);
    x-=4;
    y+=4;
    z-=4;
    color+=100;
  }
}

void testroundrects() {
  
  int color = 100;
  int i;
  int x = 0;
  int y = 0;
  int w;
  int h ;
	
	w = width();
	h = height();
	fillScreen(BLACK);
  for(i = 0 ; i <= 24; i++) {
    drawRoundRect(x, y, w, h, 5, color);
    x+=2;
    y+=3;
    w-=4;
    h-=6;
    color+=1100;
   // println(i);
  }
}

// **************Test Functions**************


//**************************************************************** 
 /*
 

void writeCommand(unsigned char c) {
    *rsport &= ~ rspinmask;
    //digitalWrite(_rs, LOW);
    
    *csport &= ~ cspinmask;
    //digitalWrite(_cs, LOW);
    
    //Serial.print("C ");
    spiwrite(c);
    
    *csport |= cspinmask;
    //digitalWrite(_cs, HIGH);
}


void writeData(unsigned char c) {
    *rsport |= rspinmask;
    //digitalWrite(_rs, HIGH);
    
    *csport &= ~ cspinmask;
    //digitalWrite(_cs, LOW);
    
//    Serial.print("D ");
    spiwrite(c);
    
    *csport |= cspinmask;
    //digitalWrite(_cs, HIGH);
} 

 
void drawFastVLine_13512(int x, int y, int h, unsigned int color) {
  // Transform x and y based on current rotation.
  switch (getRotation()) {
  case 0:  // No rotation
    drawFastVLine_13512(x, y, h, color);
    break;
  case 1:  // Rotated 90 degrees clockwise.
    swap(x, y);
    x = WIDTH - x - h;
    drawFastHLine_13512(x, y, h, color);
    break;
  case 2:  // Rotated 180 degrees clockwise.
    x = WIDTH - x - 1;
    y = HEIGHT - y - h;
    drawFastVLine_13512(x, y, h, color);
    break;
  case 3:  // Rotated 270 degrees clockwise.
    swap(x, y);
    y = HEIGHT - y - 1;
    drawFastHLine_13512(x, y, h, color);
    break;
  }
}

void drawFastHLine_13512(int x, int y, int w, unsigned int color) {
  // Transform x and y based on current rotation.
  switch (getRotation()) {
  case 0:  // No rotation.
    drawFastHLine_13512(x, y, w, color);
    break;
  case 1:  // Rotated 90 degrees clockwise.
    swap(x, y);
    x = WIDTH - x - 1;
    drawFastVLine_13512(x, y, w, color);
    break;
  case 2:  // Rotated 180 degrees clockwise.
    x = WIDTH - x - w;
    y = HEIGHT - y - 1;
    drawFastHLine_13512(x, y, w, color);
    break;
  case 3:  // Rotated 270 degrees clockwise.
    swap(x, y);
    y = HEIGHT - y - w;
    drawFastVLine_13512(x, y, w, color);
    break;
  }
}

*/

/*

void begin(void) {
    // set pin directions
    pinMode(_rs, OUTPUT);
    
    if (_sclk) {
        pinMode(_sclk, OUTPUT);
        
        pinMode(_sid, OUTPUT);
    } else {
        // using the hardware SPI
        SPI.begin();
        SPI.setDataMode(SPI_MODE3);
    }
	
    // Toggle RST low to reset; CS low so it'll listen to us
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, LOW);
    
    if (_rst) {
        pinMode(_rst, OUTPUT);
        digitalWrite(_rst, HIGH);
        delay(500);
        digitalWrite(_rst, LOW);
        delay(500);
        digitalWrite(_rst, HIGH);
        delay(500);
    }

    // Initialization Sequence
    writeCommand(SSD1351_CMD_COMMANDLOCK);  // set command lock
    writeData(0x12);  
    writeCommand(SSD1351_CMD_COMMANDLOCK);  // set command lock
    writeData(0xB1);

    writeCommand(SSD1351_CMD_DISPLAYOFF);  		// 0xAE

    writeCommand(SSD1351_CMD_CLOCKDIV);  		// 0xB3
    writeCommand(0xF1);  						// 7:4 = Oscillator Frequency, 3:0 = CLK Div Ratio (A[3:0]+1 = 1..16)
    
    writeCommand(SSD1351_CMD_MUXRATIO);
    writeData(127);
    
    writeCommand(SSD1351_CMD_SETREMAP);
    writeData(0x74);
  
    writeCommand(SSD1351_CMD_SETCOLUMN);
    writeData(0x00);
    writeData(0x7F);
    writeCommand(SSD1351_CMD_SETROW);
    writeData(0x00);
    writeData(0x7F);

    writeCommand(SSD1351_CMD_STARTLINE); 		// 0xA1
    if (SSD1351HEIGHT == 96) {
      writeData(96);
    } else {
      writeData(0);
    }


    writeCommand(SSD1351_CMD_DISPLAYOFFSET); 	// 0xA2
    writeData(0x0);

    writeCommand(SSD1351_CMD_SETGPIO);
    writeData(0x00);
    
    writeCommand(SSD1351_CMD_FUNCTIONSELECT);
    writeData(0x01); // internal (diode drop)
    //writeData(0x01); // external bias

//    writeCommand(SSSD1351_CMD_SETPHASELENGTH);
//    writeData(0x32);

    writeCommand(SSD1351_CMD_PRECHARGE);  		// 0xB1
    writeCommand(0x32);
 
    writeCommand(SSD1351_CMD_VCOMH);  			// 0xBE
    writeCommand(0x05);

    writeCommand(SSD1351_CMD_NORMALDISPLAY);  	// 0xA6

    writeCommand(SSD1351_CMD_CONTRASTABC);
    writeData(0xC8);
    writeData(0x80);
    writeData(0xC8);

    writeCommand(SSD1351_CMD_CONTRASTMASTER);
    writeData(0x0F);

    writeCommand(SSD1351_CMD_SETVSL );
    writeData(0xA0);
    writeData(0xB5);
    writeData(0x55);
    
    writeCommand(SSD1351_CMD_PRECHARGE2);
    writeData(0x01);
    
    writeCommand(SSD1351_CMD_DISPLAYON);		//--turn on oled panel    
}
*/


/********************************* low level pin initialization */
/*
Adafruit_SSD1351(unsigned char cs, unsigned char rs, unsigned char sid, unsigned char sclk, unsigned char rst) : Adafruit_GFX(SSD1351WIDTH, SSD1351HEIGHT) {
    _cs = cs;
    _rs = rs;
    _sid = sid;
    _sclk = sclk;
    _rst = rst;
    
    csport      = portOutputRegister(digitalPinToPort(cs));
    cspinmask   = digitalPinToBitMask(cs);

    rsport      = portOutputRegister(digitalPinToPort(rs));
    rspinmask   = digitalPinToBitMask(rs);

    sidport      = portOutputRegister(digitalPinToPort(sid));
    sidpinmask   = digitalPinToBitMask(sid);

    sclkport      = portOutputRegister(digitalPinToPort(sclk));
    sclkpinmask   = digitalPinToBitMask(sclk);

}
*/

/*
void Adafruit_SSD1351_Init(unsigned char cs, unsigned char rs, unsigned char sid, unsigned char sclk, unsigned char rst){
    _cs = cs;
    _rs = rs;
    _sid = sid;
    _sclk = sclk;
    _rst = rst;
    
    csport      = portOutputRegister(digitalPinToPort(cs));
    cspinmask   = digitalPinToBitMask(cs);

    rsport      = portOutputRegister(digitalPinToPort(rs));
    rspinmask   = digitalPinToBitMask(rs);

    sidport      = portOutputRegister(digitalPinToPort(sid));
    sidpinmask   = digitalPinToBitMask(sid);

    sclkport      = portOutputRegister(digitalPinToPort(sclk));
    sclkpinmask   = digitalPinToBitMask(sclk);

}
Adafruit_SSD1351(unsigned char cs, unsigned char rs,  unsigned char rst) : Adafruit_GFX(SSD1351WIDTH, SSD1351HEIGHT) {
    _cs = cs;
    _rs = rs;
    _sid = 0;
    _sclk = 0;
    _rst = rst;

    csport      = portOutputRegister(digitalPinToPort(cs));
    cspinmask   = digitalPinToBitMask(cs);
    
    rsport      = portOutputRegister(digitalPinToPort(rs));
    rspinmask   = digitalPinToBitMask(rs);

}
*/
