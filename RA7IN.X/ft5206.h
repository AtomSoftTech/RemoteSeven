#ifndef FT5206_H
#define	FT5206_H

#include "fubmini.h"
#include <xc.h>
#include <plib.h>
void _nop_();
/* touch panel interface define */
#define SDA FUB_P26
#define SDA_LOW        TRISBbits.TRISB9 = 0;LATBbits.LATB9 = 0//;_nop_()
#define SDA_HIGH       TRISBbits.TRISB9 = 1
#define isSDA          PORTBbits.RB9

#define SCL FUB_PAD25
#define SCL_LOW        TRISBbits.TRISB8 = 0;LATBbits.LATB8 = 0//;_nop_()
#define SCL_HIGH       TRISBbits.TRISB8 = 1

#define PEN FUB_P20
#define isPEN()          PORTReadBits(PEN)


#define CONFIG_FT5X0X_MULTITOUCH    //Define the multi-touch
//Touch Status
#define Key_Down 0x01
#define Key_Up   0x00



typedef struct _Touch_ {
    UINT    x1;
    UINT    y1;
    UINT    x2;
    UINT    y2;
    UINT    x3;
    UINT    y3;
    UINT    x4;
    UINT    y4;
    UINT    x5;
    UINT    y5;
    unsigned char   touch_point;
    unsigned char   Key_Sta;
} Touch;


extern Touch ts_event;
#define FT5206_DEVICE_MODE 0x00

#define WRITE_ADD	0x70
#define READ_ADD	0x71

void  TOUCH_Init(void);
void  TOUCH_Start(void);
void  TOUCH_Stop(void);
unsigned char TOUCH_Wait_Ack(void);
void  TOUCH_Ack(void);
void  TOUCH_NAck(void);

void  TOUCH_Send_Byte(unsigned char txd);
unsigned char TOUCH_Read_Byte(unsigned char ack);
void  TOUCH_Wr_Reg(unsigned char RegIndex,unsigned char RegValue1);
void  TOUCH_RdParFrPCTPFun(unsigned char *PCTP_Par,unsigned char ValFlag);
unsigned char TOUCH_Read_Reg(unsigned char RegIndex);
void  Draw_Big_Point(UINT x,UINT y,UINT colour);
unsigned char ft5x0x_read_data(void);  //unsigned char ft5x0x_read_data(void);

char CheckPen(void);

#endif