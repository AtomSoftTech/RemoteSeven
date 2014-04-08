/*
 * File:   main.c
 * Author: Jason
 *
 * Created on March 4, 2014, 3:45 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xc.h>
#include <plib.h>

#include "fubmini.h"
#include "spi2.h"
#include "ra8875.h"
#include "ft5206.h"

#include "ff.h"
#include "diskio.h"

#include "ir.h"
#include "ircodes.h"

#pragma config PMDL1WAY = OFF           // Peripheral Module Disable Configuration (Allow multiple reconfigurations)
#pragma config IOL1WAY = OFF            // Peripheral Pin Select Configuration (Allow multiple reconfigurations)
#pragma config FUSBIDIO = OFF           // USB USID Selection (Controlled by Port Function)
#pragma config FVBUSONIO = OFF          // USB VBUS ON Selection (Controlled by Port Function)
#pragma config FPLLIDIV = DIV_2         // PLL Input Divider (2x Divider)
#pragma config FPLLMUL = MUL_24         // PLL Multiplier (24x Multiplier)
#pragma config UPLLIDIV = DIV_2         // USB PLL Input Divider (2x Divider)
#pragma config UPLLEN = ON              // USB PLL Enable (Enabled)
#pragma config FPLLODIV = DIV_2         // System PLL Output CLOCK2 Divider (PLL Divide by 2)
#pragma config FNOSC = PRIPLL           // Oscillator Selection Bits (Primary Osc w/PLL (XT+,HS+,EC+PLL))
#pragma config FSOSCEN = OFF            // Secondary Oscillator Enable (Disabled)
#pragma config IESO = OFF               // Internal/External Switch Over (Disabled)
#pragma config POSCMOD = XT             // Primary Oscillator Configuration (XT osc mode)
#pragma config OSCIOFNC = OFF           // CLKO Output Signal Active on the OSCO Pin (Disabled)
#pragma config FPBDIV = DIV_1           // Peripheral CLOCK2 Divisor (Pb_Clk is Sys_Clk/1)
#pragma config FCKSM = CSECME           // CLOCK2 Switching and Monitor Selection (CLOCK2 Switch Enable, FSCM Enabled)
#pragma config WDTPS = PS1024           // Watchdog Timer Postscaler (1:1024)
#pragma config WINDIS = OFF             // Watchdog Timer Window Enable (Watchdog Timer is in Non-Window Mode)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (WDT Disabled (SWDTEN Bit Controls))
#pragma config FWDTWINSZ = WISZ_25      // Watchdog Timer Window Size (Window Size is 25%)
#pragma config JTAGEN = OFF             // JTAG Enable (JTAG Disabled)
#pragma config ICESEL = ICS_PGx1        // ICE/ICD Comm Channel Select (Communicate on PGEC1/PGED1)
#pragma config PWP = OFF                // Program Flash Write Protect (Disable)
#pragma config BWP = OFF                // Boot Flash Write Protect bit (Protection Disabled)
#pragma config CP = OFF                 // Code Protect (Protection Disabled)

#define ALL_BITS 0xFFFFFFFF

char isButton(Button btn);

#define SYS_FREQ    (48000000L)


FATFS fs;          // Work area (file system object) for the volume
BYTE FILE_IN_BUFF[512];
DSTATUS iStat;

void init();
void configIO();
void delay();
char isTouchButton( TouchButton btn);

void WriteString (char *text, uint x, uint y, uint fcolor, uint bcolor);
void InitButtons (void);
void ImgBtnClick(ImageButton btn);

uint fColor = color_black;
uint bColor = color_white;

#define btnDelay 2

char messageOut[40];
uint imageCount = 0;

FIL fil;       /* File object */
char line[82]; /* Line buffer */

#define IMAGELEN ((480*800)*2) //700800 bytes needed for full image (paint app)


uchar IMAGEBUFF[IMAGEBUFF_LEN];

 uint RGB16(char R,char G,char B)
 {
     return ((B >> 3) + ((G >> 2)<<5) + ((R >> 3)<<11));
 }
void DrawTouchButton(TouchButton tB, char state)
{
    drawRect(tB.left, tB.top, tB.width, tB.height, tB.back_color, 1);
    drawRect(tB.left, tB.top, tB.width, tB.height, color_black, 0);

    if(state == 1)
    {
      DrawLine(tB.left+1,(tB.left+(tB.width-1)),(tB.top+(tB.height-1)),(tB.top+(tB.height-1)),color_white);
      DrawLine((tB.left+(tB.width-1)),(tB.left+(tB.width-1)),(tB.top+1),(tB.top+(tB.height-1)),color_white);
    }
    else
    {
      DrawLine(tB.left+1,(tB.left+(tB.width-1)),(tB.top+1),(tB.top+1),color_white);
      DrawLine((tB.left+1),(tB.left+1),(tB.top+1),(tB.top+(tB.height-1)),color_white);
    }

    SetColors(tB.fore_color,tB.back_color);

    FontWrite_Position((tB.left+(tB.width/2)/2)-2, (tB.top+(tB.height/2)/2)-2);
    String(tB.text);

    SetColors(fColor,bColor);
    
}
void WriteString (char *text, uint x, uint y, uint fcolor, uint bcolor)
{
    Text_Foreground_Color1(fcolor);//Set the foreground color
    Text_Background_Color1(bcolor);//Set the background color

    FontWrite_Position(x,y);
    String(text);

    Text_Foreground_Color1(fColor);//Set the foreground color
    Text_Background_Color1(bColor);//Set the background color
}

char LastFile[13];
uint myW = 800;
uint myH = 438;

void saveImage(void)
{
    uint x,y,w,h;
    uint count,count2;
    FRESULT res;
    UINT br, bw;         /* File read/write count */
    FRESULT fr;          /* FatFs return code */
    uint len;
    uint last;

    do
    {
        sprintf(LastFile, "image%d.dat\0",imageCount++);
        fr = f_open(&fil, LastFile, FA_OPEN_EXISTING);
        f_close(&fil);
        if(fr == FR_NO_FILE) break;
    } while(1);

    fr = f_open(&fil, LastFile, FA_CREATE_ALWAYS | FA_WRITE);

    //void LCD_ReadBuff(char *buff, uint len)
    x = 0;
    y = 480-myH;
    w = myW;
    h = myH;

    WriteCommand(0x40,0x00);    // Graphics mode
    WriteCommand(0x45,0x00);    // Graphics mode - READ LEFT TO RIGHT then TOP TO BOTTOM
    SetGraphicsCursorRead(x, y);
    LCD_CmdWrite(0x02);


    len = ((w*h)*2)/IMAGEBUFF_LEN;
    last = (((w*h)*2)%IMAGEBUFF_LEN);
    if(last > 0) len++;

    while(len--)
    {

        if(len==1)
            count2 = last;
        else
            count2 = IMAGEBUFF_LEN;

        CS_LOW(LCD);
        SPI_Write(0x40);         // Cmd: read data

        for(count = 0; count < count2;count++)
        {
            FILE_IN_BUFF[count++] = SPI_Read();
            FILE_IN_BUFF[count++] = SPI_Read();
            FILE_IN_BUFF[count++] = SPI_Read();
            FILE_IN_BUFF[count] = SPI_Read();
        }
        CS_HIGH();

        fr = f_write(&fil, FILE_IN_BUFF, count2, &bw);            /* Write it to the destination file */
        if (fr) break; /* error or disk full */
    }

    f_close(&fil);

}

void openImage(void)
{
    uint x,y,w,h;
    uint count,count2;
    FRESULT res;
    UINT br, bw;         /* File read/write count */
    FRESULT fr;          /* FatFs return code */
    uint len;
    uint last;

    fr = f_open(&fil, LastFile, FA_OPEN_EXISTING | FA_READ);

    x = 0;
    y = 480-myH;
    w = myW;
    h = myH;

    WriteCommand(0x40,0x00);    // Graphics write mode
    SetGraphicsCursorWrite(x, y);
    LCD_CmdWrite(0x02);


    len = ((w*h)*2)/IMAGEBUFF_LEN;
    last = (((w*h)*2)%IMAGEBUFF_LEN);
    if(last > 0) len++;

    while(len--)
    {
        if(len==1)
            count2 = last;
        else
            count2 = IMAGEBUFF_LEN;

        fr = f_read(&fil, FILE_IN_BUFF, count2, &br);  /* Read a chunk of source file */
        if (fr != FR_OK) break;

        Chk_Busy();
        CS_LOW(LCD);

        SPI_Write(0x00);         // Cmd: write data

        for(count = 0; count < count2;count++)
            SPI_Write(FILE_IN_BUFF[count]);

        CS_HIGH();

        //if (br != count2) break; /* error or eof */

    }

    f_close(&fil);

}

ImageButton btnPlay;
ImageButton btnStop;
ImageButton btnPause;
ImageButton btnRew;
ImageButton btnFfw;
ImageButton btnRec;
ImageButton btnPwr;

ImageButton btnZero;
ImageButton btnOne;
ImageButton btnTwo;
ImageButton btnThree;
ImageButton btnFour;
ImageButton btnFive;
ImageButton btnSix;
ImageButton btnSeven;
ImageButton btnEight;
ImageButton btnNine;
ImageButton btnStar;
ImageButton btnPound;

ImageButton btnGuide;
ImageButton btnMenu;
ImageButton btnInfo;
ImageButton btnExit;
ImageButton btnHdmi;

ImageButton btnVolU;
ImageButton btnVolD;

ImageButton btnChU;
ImageButton btnChD;

ImageButton btnUp;
ImageButton btnDown;
ImageButton btnLeft;
ImageButton btnRight;
ImageButton btnEnter;


void main ()
{
    uint tryCount = 1;
    uint FFST = 0;
    uint i;

    init();
    configIO();

    //init_ir();
    //TestDelay();

    SpiInitDevice(2,SPI_MED,0);
    TOUCH_Init();

    LCD_Initial();
    Backlight(RA8875_PWM_CLK_DIV8,80);
    
    Write_Dir(0X01,0X80);//display on

    Active_Window(0,799,0,479);//Set the working window size
    SetColors(fColor,bColor);
    ClearScreen(0);
    Chk_Busy();

    FontWrite_Position(10,10);
    String("AtomSoftTech - RA8875 7\" TFT with Capacitive Touch");

    FontWrite_Position(10,30);
    String("Insert SD Card...Waiting");
    
    SpiChnClose(2);
    SpiInitDevice(2,SPI_SLOWEST,0);

    while(isCD() == 1);
    FontWrite_Position(10,50);
    String("SD Card...Found!");

    do
    {

        iStat = FR_NOT_READY;
        Delay10ms(20);
        FontWrite_Position(10,70);

        sprintf(messageOut, "Mounting SD Card with Elm Chan's FatFS. Try #%d  \0", tryCount);
        String(messageOut);
        tryCount++;
        iStat = f_mount(&fs, "", 1);//(&fs);  //MountSD(fs);
    }
    while(iStat != FR_OK);

    FontWrite_Position(10,90);

    if(fs.fs_type == FS_FAT12)
        FFST = 12;
    if(fs.fs_type == FS_FAT16)
        FFST = 16;
    if(fs.fs_type == FS_FAT32)
        FFST = 32;

    sprintf(messageOut, "SD Card Filesystem... FAT%d  \0", FFST);
    String(messageOut);

    FontWrite_Position(10,110);

    sprintf(messageOut, "Press anywhere to continue.\0");
    String(messageOut);

    //while(CheckPen() != 0);
    //Delay100ms(1);  //Allow readability

    ClearScreen(0);
    Chk_Busy();

    SetColors(0,RGB16(40,43,48));
    ClearScreen(0);
    Chk_Busy();
    
    //Some other init stuff
    InitButtons();
    init_ir();


    while(1)
    {

        Write_Dir(0x40,0x80);//Set the character mode
        Write_Dir(0x21,0x10);//Select the internal CGROM  ISO/IEC 8859-1.
        Write_Dir(0x22,0x00);//Full alignment is disable.The text background color . Text don't rotation. 2x zoom


        if(CheckPen() == 0)        //The touch screen is pressed
        {
            do
            {
                ft5x0x_read_data();

                ////////////////////////////////////
                ////////////////////////////////////
                ////////////////////////////////////
                if(isImageButton(btnOne))
                {
                    ImgBtnClick(btnOne);
                    IR_TWC(TWC_CABLE, TWC_CABLE_1);
                }
                if(isImageButton(btnTwo))
                {
                    ImgBtnClick(btnTwo);
                    IR_TWC(TWC_CABLE, TWC_CABLE_2);
                }
                if(isImageButton(btnThree))
                {
                    ImgBtnClick(btnThree);
                    IR_TWC(TWC_CABLE, TWC_CABLE_3);
                }
                if(isImageButton(btnFour))
                {
                    ImgBtnClick(btnFour);
                    IR_TWC(TWC_CABLE, TWC_CABLE_4);
                }
                if(isImageButton(btnFive))
                {
                    ImgBtnClick(btnFive);
                    IR_TWC(TWC_CABLE, TWC_CABLE_5);
                }
                if(isImageButton(btnSix))
                {
                    ImgBtnClick(btnSix);
                    IR_TWC(TWC_CABLE, TWC_CABLE_6);
                }
                if(isImageButton(btnSeven))
                {
                    ImgBtnClick(btnSeven);
                    IR_TWC(TWC_CABLE, TWC_CABLE_7);
                }
                if(isImageButton(btnEight))
                {
                    ImgBtnClick(btnEight);
                    IR_TWC(TWC_CABLE, TWC_CABLE_8);
                }
                if(isImageButton(btnNine))
                {
                    ImgBtnClick(btnNine);
                    IR_TWC(TWC_CABLE, TWC_CABLE_9);
                }
                if(isImageButton(btnZero))
                {
                    ImgBtnClick(btnZero);
                    IR_TWC(TWC_CABLE, TWC_CABLE_0);
                }
                if(isImageButton(btnStar))
                {
                    ImgBtnClick(btnStar);
                    IR_TWC(TWC_CABLE, TWC_CABLE_STAR);
                }
                if(isImageButton(btnPound))
                {
                    ImgBtnClick(btnPound);
                    IR_TWC(TWC_CABLE, TWC_CABLE_POUND);
                }
                ////////////////////////////////////
                ////////////////////////////////////
                ////////////////////////////////////
                if(isImageButton(btnStop))
                {
                    ImgBtnClick(btnStop);
                    IR_TWC(TWC_CABLE, TWC_CABLE_STOP);
                }

                if(isImageButton(btnRew))
                {
                    ImgBtnClick(btnRew);
                    IR_TWC(TWC_CABLE, TWC_CABLE_REWIND);
                }
                
                if(isImageButton(btnPlay))
                {
                    ImgBtnClick(btnPlay);
                    IR_TWC(TWC_CABLE, TWC_CABLE_PLAY);
                }
                
                if(isImageButton(btnPause))
                {
                    ImgBtnClick(btnPause);
                    IR_TWC(TWC_CABLE, TWC_CABLE_PAUSE);
                }
                
                if(isImageButton(btnFfw))
                {
                    ImgBtnClick(btnFfw);
                    IR_TWC(TWC_CABLE, TWC_CABLE_FFW);
                }
                
                if(isImageButton(btnRec))
                {
                    ImgBtnClick(btnRec);
                    IR_TWC(TWC_CABLE, TWC_CABLE_RECORD);
                }

                if(isImageButton(btnGuide))
                {
                    ImgBtnClick(btnGuide);
                    IR_TWC(TWC_CABLE, TWC_CABLE_GUIDE);
                }

                if(isImageButton(btnMenu))
                {
                    ImgBtnClick(btnMenu);
                    IR_TWC(TWC_CABLE, TWC_CABLE_MENU);
                }

                if(isImageButton(btnInfo))
                {
                    ImgBtnClick(btnInfo);
                    IR_TWC(TWC_CABLE, TWC_CABLE_INFO);
                }

                if(isImageButton(btnExit))
                {
                    ImgBtnClick(btnExit);
                    IR_TWC(TWC_CABLE, TWC_CABLE_EXIT);
                }

                if(isImageButton(btnHdmi))
                {
                    ImgBtnClick(btnHdmi);
                    IR_DYNEX(DYNEX, DYNEX_TV,DYNEX_TV_HDMI);
                }

                if(isImageButton(btnVolU))
                {
                    ImgBtnClick(btnVolU);
                    IR_DYNEX(DYNEX, DYNEX_TV,DYNEX_TV_VOLUP);
                }

                if(isImageButton(btnVolD))
                {
                    ImgBtnClick(btnVolD);
                    IR_DYNEX(DYNEX, DYNEX_TV,DYNEX_TV_VOLDOWN);
                }

                if(isImageButton(btnChU))
                {
                    ImgBtnClick(btnChU);
                    IR_TWC(TWC_CABLE, TWC_CABLE_CHUP);
                }

                if(isImageButton(btnChD))
                {
                    ImgBtnClick(btnChD);
                    IR_TWC(TWC_CABLE, TWC_CABLE_CHDN);
                }

                if(isImageButton(btnUp))
                {
                    ImgBtnClick(btnUp);
                    IR_TWC(TWC_CABLE, TWC_CABLE_UP);
                }
                if(isImageButton(btnLeft))
                {
                    ImgBtnClick(btnLeft);
                    IR_TWC(TWC_CABLE, TWC_CABLE_LEFT);
                }
                if(isImageButton(btnRight))
                {
                    ImgBtnClick(btnRight);
                    IR_TWC(TWC_CABLE, TWC_CABLE_RIGHT);
                }
                if(isImageButton(btnDown))
                {
                    ImgBtnClick(btnDown);
                    IR_TWC(TWC_CABLE, TWC_CABLE_DOWN);
                }
                if(isImageButton(btnEnter))
                {
                    ImgBtnClick(btnEnter);
                    IR_TWC(TWC_CABLE, TWC_CABLE_ENTER);
                }



            }while(isPEN()==0);


            ts_event.Key_Sta=Key_Up;

        }
    }
}
void ImgBtnClick(ImageButton btn)
{
    OpenASI(btn.down,btn.left,btn.top);
    Delay1ms(3);
    OpenASI(btn.up,btn.left,btn.top);
}
void InitButtons (void)
{
    int SHIFT_LEFT = 5;

    uint NPAD_LEFT = 290+SHIFT_LEFT;
    uint NPAD_TOP = 100;

    uint DVR_LEFT = 140+SHIFT_LEFT;
    uint DVR_TOP = 370;

    uint main_btn_left = 105+SHIFT_LEFT;
    uint main_btn_top = 10;

    uint vol_top = 155;
    uint vol_left = 230+SHIFT_LEFT;

    uint ch_top = vol_top;
    uint ch_left = vol_left + 265+SHIFT_LEFT;

    uint dpad_c_top = 205;
    uint dpad_c_left = 650;

    btnVolU.height = 73;
    btnVolU.width = 63;
    btnVolU.top = vol_top;
    btnVolU.left = vol_left;
    sprintf(btnVolU.up, "plus.asi");
    sprintf(btnVolU.down, "dplus.asi");

    btnVolD.height = 73;
    btnVolD.width = 63;
    btnVolD.top = vol_top+73;
    btnVolD.left = vol_left;
    sprintf(btnVolD.up, "min.asi");
    sprintf(btnVolD.down, "dmin.asi");

    btnChU.height = 73;
    btnChU.width = 63;
    btnChU.top = ch_top;
    btnChU.left = ch_left;
    sprintf(btnChU.up, "plus.asi");
    sprintf(btnChU.down, "dplus.asi");

    btnChD.height = 73;
    btnChD.width = 63;
    btnChD.top = ch_top+73;
    btnChD.left = ch_left;
    sprintf(btnChD.up, "min.asi");
    sprintf(btnChD.down, "dmin.asi");

    //DVR
    btnStop.height = 80;
    btnStop.width = 80;
    btnStop.top = DVR_TOP;
    btnStop.left = DVR_LEFT;
    sprintf(btnStop.up, "stop.asi");
    sprintf(btnStop.down, "dstop.asi");

    btnRew.height = 80;
    btnRew.width = 80;
    btnRew.top = DVR_TOP;
    btnRew.left = btnStop.left+84;
    sprintf(btnRew.up, "rew.asi");
    sprintf(btnRew.down, "drew.asi");

    btnPlay.height = 80;
    btnPlay.width = 80;
    btnPlay.top = DVR_TOP;
    btnPlay.left = btnRew.left+84;
    sprintf(btnPlay.up, "play.asi");
    sprintf(btnPlay.down, "dplay.asi");

    btnPause.height = 80;
    btnPause.width = 80;
    btnPause.top = DVR_TOP;
    btnPause.left = btnPlay.left+84;
    sprintf(btnPause.up, "pause.asi");
    sprintf(btnPause.down, "dpause.asi");

    btnFfw.height = 80;
    btnFfw.width = 80;
    btnFfw.top = DVR_TOP;
    btnFfw.left = btnPause.left+84;
    sprintf(btnFfw.up, "ffw.asi");
    sprintf(btnFfw.down, "dffw.asi");

    btnRec.height = 80;
    btnRec.width = 80;
    btnRec.top = DVR_TOP;
    btnRec.left = btnFfw.left+84+30;
    sprintf(btnRec.up, "rec.asi");
    sprintf(btnRec.down, "drec.asi");

    //ROW 1
    btnOne.height = 63;
    btnOne.width = 70;
    btnOne.top = NPAD_TOP;
    btnOne.left = NPAD_LEFT;
    sprintf(btnOne.up, "one.asi");
    sprintf(btnOne.down, "done.asi");

    btnTwo.height = 63;
    btnTwo.width = 70;
    btnTwo.top = NPAD_TOP;
    btnTwo.left = btnOne.left + 70;
    sprintf(btnTwo.up, "two.asi");
    sprintf(btnTwo.down, "dtwo.asi");

    btnThree.height = 63;
    btnThree.width = 70;
    btnThree.top = NPAD_TOP;
    btnThree.left = btnTwo.left + 70;
    sprintf(btnThree.up, "three.asi");
    sprintf(btnThree.down, "dthree.asi");

    NPAD_TOP += 63;
    //ROW 2
    btnFour.height = 63;
    btnFour.width = 70;
    btnFour.top = NPAD_TOP;
    btnFour.left = btnOne.left;
    sprintf(btnFour.up, "four.asi");
    sprintf(btnFour.down, "dfour.asi");

    btnFive.height = 63;
    btnFive.width = 70;
    btnFive.top = NPAD_TOP;
    btnFive.left = btnFour.left + 70;
    sprintf(btnFive.up, "five.asi");
    sprintf(btnFive.down, "dfive.asi");

    btnSix.height = 63;
    btnSix.width = 70;
    btnSix.top = NPAD_TOP;
    btnSix.left = btnFive.left + 70;
    sprintf(btnSix.up, "six.asi");
    sprintf(btnSix.down, "dsix.asi");

    NPAD_TOP += 63;
    //ROW 3
    btnSeven.height = 63;
    btnSeven.width = 70;
    btnSeven.top = NPAD_TOP;
    btnSeven.left = btnOne.left;
    sprintf(btnSeven.up, "seven.asi");
    sprintf(btnSeven.down, "dseven.asi");

    btnEight.height = 63;
    btnEight.width = 70;
    btnEight.top = NPAD_TOP;
    btnEight.left = btnSeven.left + 70;
    sprintf(btnEight.up, "eight.asi");
    sprintf(btnEight.down, "deight.asi");

    btnNine.height = 63;
    btnNine.width = 70;
    btnNine.top = NPAD_TOP;
    btnNine.left = btnEight.left + 70;
    sprintf(btnNine.up, "nine.asi");
    sprintf(btnNine.down, "dnine.asi");

    NPAD_TOP+=63;
    //ROW 4
    btnStar.height = 63;
    btnStar.width = 70;
    btnStar.top = NPAD_TOP;
    btnStar.left = btnOne.left;
    sprintf(btnStar.up, "star.asi");
    sprintf(btnStar.down, "dstar.asi");

    btnZero.height = 63;
    btnZero.width = 70;
    btnZero.top = NPAD_TOP;
    btnZero.left = btnStar.left + 70;
    sprintf(btnZero.up, "zero.asi");
    sprintf(btnZero.down, "dzero.asi");

    btnPound.height = 63;
    btnPound.width = 70;
    btnPound.top = NPAD_TOP;
    btnPound.left = btnZero.left + 70;
    sprintf(btnPound.up, "pund.asi");
    sprintf(btnPound.down, "dpound.asi");

    //MAIN
    btnGuide.height = 63;
    btnGuide.width = 120;
    btnGuide.top = main_btn_top;
    btnGuide.left = main_btn_left;
    sprintf(btnGuide.up, "guide.asi");
    sprintf(btnGuide.down, "dguide.asi");

    btnMenu.height = 63;
    btnMenu.width = 120;
    btnMenu.top = main_btn_top;
    btnMenu.left = btnGuide.left+120;
    sprintf(btnMenu.up, "menu.asi");
    sprintf(btnMenu.down, "dmenu.asi");

    btnInfo.height = 63;
    btnInfo.width = 120;
    btnInfo.top = main_btn_top;
    btnInfo.left = btnMenu.left+120;
    sprintf(btnInfo.up, "info.asi");
    sprintf(btnInfo.down, "dinfo.asi");

    btnExit.height = 63;
    btnExit.width = 120;
    btnExit.top = main_btn_top;
    btnExit.left = btnInfo.left+120;
    sprintf(btnExit.up, "exit.asi");
    sprintf(btnExit.down, "dexit.asi");

    btnHdmi.height = 63;
    btnHdmi.width = 120;
    btnHdmi.top = main_btn_top;
    btnHdmi.left = btnExit.left+120;
    sprintf(btnHdmi.up, "hdmi.asi");
    sprintf(btnHdmi.down, "dhdmi.asi");

    //DPAD

    btnEnter.height = 42;
    btnEnter.width = 42;
    btnEnter.top = dpad_c_top;
    btnEnter.left = dpad_c_left;
    sprintf(btnEnter.up, "enter.asi");
    sprintf(btnEnter.down, "denter.asi");

    btnUp.height = 40;
    btnUp.width = 42;
    btnUp.top = dpad_c_top - 42;
    btnUp.left = dpad_c_left;
    sprintf(btnUp.up, "up.asi");
    sprintf(btnUp.down, "dup.asi");

    btnDown.height = 40;
    btnDown.width = 42;
    btnDown.top = dpad_c_top + 42;
    btnDown.left = dpad_c_left;
    sprintf(btnDown.up, "down.asi");
    sprintf(btnDown.down, "ddown.asi");

    btnLeft.height = 42;
    btnLeft.width = 40;
    btnLeft.top = dpad_c_top;
    btnLeft.left = dpad_c_left - 40;
    sprintf(btnLeft.up, "left.asi");
    sprintf(btnLeft.down, "dleft.asi");

    btnRight.height = 42;
    btnRight.width = 40;
    btnRight.top = dpad_c_top ;
    btnRight.left = dpad_c_left + 40;
    sprintf(btnRight.up, "right.asi");
    sprintf(btnRight.down, "dright.asi");

    //DRAW THEM ALL

    OpenASI(btnRew.up,btnRew.left,btnRew.top);
    OpenASI(btnStop.up,btnStop.left,btnStop.top);
    OpenASI(btnPlay.up,btnPlay.left,btnPlay.top);
    OpenASI(btnPause.up,btnPause.left,btnPause.top);
    OpenASI(btnFfw.up,btnFfw.left,btnFfw.top);
    OpenASI(btnRec.up,btnRec.left,btnRec.top);
    
    OpenASI(btnOne.up,btnOne.left,btnOne.top);
    OpenASI(btnTwo.up,btnTwo.left,btnTwo.top);
    OpenASI(btnThree.up,btnThree.left,btnThree.top);
    OpenASI(btnFour.up,btnFour.left,btnFour.top);
    OpenASI(btnFive.up,btnFive.left,btnFive.top);
    OpenASI(btnSix.up,btnSix.left,btnSix.top);
    OpenASI(btnSeven.up,btnSeven.left,btnSeven.top);
    OpenASI(btnEight.up,btnEight.left,btnEight.top);
    OpenASI(btnNine.up,btnNine.left,btnNine.top);
    OpenASI(btnStar.up,btnStar.left,btnStar.top);
    OpenASI(btnZero.up,btnZero.left,btnZero.top);
    OpenASI(btnPound.up,btnPound.left,btnPound.top);

    OpenASI(btnGuide.up,btnGuide.left,btnGuide.top);
    OpenASI(btnMenu.up,btnMenu.left,btnMenu.top);
    OpenASI(btnInfo.up,btnInfo.left,btnInfo.top);
    OpenASI(btnExit.up,btnExit.left,btnExit.top);
    OpenASI(btnHdmi.up,btnHdmi.left,btnHdmi.top);

    OpenASI(btnVolU.up,btnVolU.left,btnVolU.top);
    OpenASI(btnVolD.up,btnVolD.left,btnVolD.top);
    OpenASI(btnChU.up,btnChU.left,btnChU.top);
    OpenASI(btnChD.up,btnChD.left,btnChD.top);

    OpenASI(btnUp.up,btnUp.left,btnUp.top);
    OpenASI(btnDown.up,btnDown.left,btnDown.top);
    OpenASI(btnLeft.up,btnLeft.left,btnLeft.top);
    OpenASI(btnRight.up,btnRight.left,btnRight.top);
    OpenASI(btnEnter.up,btnEnter.left,btnEnter.top);
}

char isButton( Button btn)
{
    char isInX = 0;
    char isInY = 0;
    char isIt = 0;
    UINT x = ts_event.x1;
    UINT y = ts_event.y1;

    if(x >= btn.left)
    {
        if(x <= (btn.left+btn.width))
        {
            isInX = 1;
        }
    }

    if(y >= btn.top)
    {
        if(y <= (btn.top+btn.height))
        {
            isInY = 1;
        }
    }

    isIt = isInX + isInY;

    if(isIt >= 2)
    {
        return 1;
    }
    else
        return 0;
}

char isTouchButton( TouchButton btn)
{
    char isInX = 0;
    char isInY = 0;
    char isIt = 0;
    UINT x = ts_event.x1;
    UINT y = ts_event.y1;

    if(x >= btn.left)
    {
        if(x <= (btn.left+btn.width))
        {
            isInX = 1;
        }
    }

    if(y >= btn.top)
    {
        if(y <= (btn.top+btn.height))
        {
            isInY = 1;
        }
    }

    isIt = isInX + isInY;

    if(isIt >= 2)
    {
        return 1;
    }
    else
        return 0;
}
void delay()
{
    UINT tt;
    tt = 100;
    while(tt--)
        Nop();
}

void configIO()
{
    //ALL INPUT TO AVOID ISSUES
    PORTSetPinsDigitalIn(IOPORT_A, ALL_BITS);
    PORTSetPinsDigitalIn(IOPORT_B, ALL_BITS);
    PORTSetPinsDigitalIn(IOPORT_C, ALL_BITS);

    PORTSetPinsDigitalOut(IOPORT_A, FUB_PAD2 | FUB_PAD6);    //LCS_CS,MOSI
    PORTSetPinsDigitalOut(IOPORT_B, FUB_PAD4 | FUB_PAD7 );   //RST,SCK
    PORTSetPinsDigitalOut(IOPORT_C, FUB_PAD30);              //SD_CS

}

void init()
{
    SYSTEMConfig(SYS_FREQ, SYS_CFG_ALL);
    OSCConfig(OSC_POSC_PLL, OSC_PLL_MULT_24,OSC_PLL_POST_2,0);
    mOSCSetPBDIV( OSC_PB_DIV_1 );    // Configure the PB bus to run at 1/4 the CPU frequency
}
