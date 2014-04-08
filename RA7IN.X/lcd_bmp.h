#ifndef _LCD_BMP_H
#define _LCD_BMP_H

#include "ff.h"
#include "diskio.h"
#include "spi2.h"
#include "ra8875.h"

/* contrast 0 to 255 */
#define contrast 160    //(unsigned char)(50 * 255 / 100)
#define RGB16(R,G,B)  ((B >> 3) + ((G >> 2)<<5) + ((R >> 3)<<11)); // 24-bit Color to 16-bit Color RRRRRGGGGGGBBBBB format

void draw_bmp(FIL *fp, unsigned char x,unsigned char y);
void atomBMP(FIL *fp, unsigned char x,unsigned char y);
void draw_bitmap(char *filename,unsigned char x,unsigned char y);

typedef struct _BININFO_  {
    UINT width;
    UINT height;
    UINT len;
} BININFO;

typedef struct _BITMAPINFOHEADER_  {
    WORD magic;
    DWORD bmp_offset;
    DWORD header_sz;
    DWORD width;
    DWORD height;
    WORD nplanes;
    WORD bitspp;
    DWORD compress_type;
    DWORD bmp_bytesz;
    DWORD hres;
    DWORD vres;
    DWORD ncolors;
    DWORD nimpcolors;
} BITMAPINFOHEADER;

#endif    // _LCD_BMP
