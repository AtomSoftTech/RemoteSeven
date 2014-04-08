/* 
 * File:   spi2.h
 * Author: Jason
 *
 * Created on March 1, 2014, 12:13 PM
 */

#ifndef SPI2_H
#define	SPI2_H

#include "fubmini.h"
#include <xc.h>
#include <plib.h>

//-----------------------
// SPEEDS
//-----------------------
#define SPI_SLOWEST 480     //100KHz
#define SPI_SLOW    120     //400KHz
#define SPI_MED     10      //4.8MHz
#define SPI_FAST    2       //24MHz
#define SPI_FASTEST 1       //48MHz
//-----------------------
// CHIP SELECTS
//-----------------------
#define SD_CS FUB_P30
#define SD_CS_LOW()       PORTClearBits(SD_CS);
#define SD_CS_HIGH()      PORTSetBits(SD_CS);

#define LCD_CS FUB_P2
#define LCD_CS_LOW()        PORTClearBits(LCD_CS)
#define LCD_CS_HIGH()       PORTSetBits(LCD_CS)

//-----------------------
// DATA PINS
//-----------------------
#define MOSI FUB_P6
#define MOSI_LOW()       PORTClearBits(MOSI)
#define MOSI_HIGH()      PORTSetBits(MOSI)

#define MISO FUB_P18
#define isMISO()          PORTReadBits(MISO)

#define SCK  FUB_P4
#define SCK_LOW()       PORTClearBits(SCK)
#define SCK_HIGH()      PORTSetBits(SCK)

//-----------------------
// MISC
//-----------------------
#define SD_CD FUB_P29
#define isCD()          PORTCbits.RC8

#define LCD_RST FUB_P7
#define LCD_RST_LOW()       PORTClearBits(LCD_RST)
#define LCD_RST_HIGH()      PORTSetBits(LCD_RST)
//-----------------------
// CS TYPES
//-----------------------
#define LCD 0
#define SD 1

//-----------------------
// EXTERNAL FUNCTIONS
//-----------------------
void CS_LOW(char type);
void CS_HIGH();
void SpiInitDevice(int chn, int srcClkDiv, int samp);
void SpiConfigPins();
unsigned char SpiTransfer(unsigned char byte);
void release_spi();
unsigned char SpiTransfer2(unsigned char byte);

#endif	/* SPI2_H */

