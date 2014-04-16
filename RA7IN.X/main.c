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

#include "main.h"

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
uint BLSTAT = 1;
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

    SpiChnClose(2);
    SpiInitDevice(2,SPI_FAST,0);
    
    init_ir();

    //ReplaceASI
    Write_Dir(0x40,0x80);//Set the character mode
    Write_Dir(0x21,0x10);//Select the internal CGROM  ISO/IEC 8859-1.
    Write_Dir(0x22,0x00);//Full alignment is disable.The text background color . Text don't rotation. 2x zoom

    OpenASI("main.asi",0,0);

    InitButtons();
    initProgBtn();

    OpenASI("ch.asi",ch.left,ch.top);
    OpenASI("vol.asi",vol.left,vol.top);

    while(1)
    {
        while(!BLSTAT)
        {
            if(isProg())
            {
                BLSTAT = 1;
                Backlight(RA8875_PWM_CLK_DIV1,100);
                Delay1ms(128);
            }
        }

        if(isProg())
        {
            Backlight(0,0);
            BLSTAT = 0;
            Delay1ms(100);
        }
        if(CheckPen() == 0)        //The touch screen is pressed
        {
            do
            {
                ft5x0x_read_data();

                if(isButton(guide)) {
                    BtnClick(guide); IR_TWC(TWC_CABLE, TWC_CABLE_GUIDE);
                }
                if(isButton(menu)) {
                    BtnClick(menu); IR_TWC(TWC_CABLE, TWC_CABLE_MENU);
                }
                if(isButton(info)) {
                    BtnClick(info); IR_TWC(TWC_CABLE, TWC_CABLE_INFO);
                }
                if(isButton(source)) {
                    BtnClick(source); IR_DYNEX(DYNEX, DYNEX_TV,DYNEX_TV_INPUT);
                }
                if(isButton(Exit)) {
                    BtnClick(Exit); IR_TWC(TWC_CABLE, TWC_CABLE_EXIT);
                }
                if(isButton(tv_power)) {
                    BtnClick(tv_power); IR_DYNEX(DYNEX, DYNEX_TV,DYNEX_TV_POWER);
                }
                if(isButton(dvr_jmp)) {
                    BtnClick(dvr_jmp); IR_TWC(TWC_CABLE, TWC_CABLE_JMPBCK);
                }
                if(isButton(dvr_list)) {
                    BtnClick(dvr_list); IR_TWC(TWC_CABLE, TWC_CABLE_LIST);
                }
                if(isButton(dvr_live)) {
                    BtnClick(dvr_live); IR_TWC(TWC_CABLE, TWC_CABLE_LIVE);
                }
                if(isButton(dvr_stop)) {
                    BtnClick(dvr_stop); IR_TWC(TWC_CABLE, TWC_CABLE_STOP);
                }
                if(isButton(dvr_rec)) {
                    BtnClick(dvr_rec); IR_TWC(TWC_CABLE, TWC_CABLE_RECORD);
                }
                if(isButton(dvr_pause)) {
                    BtnClick(dvr_pause); IR_TWC(TWC_CABLE, TWC_CABLE_PAUSE);
                }
                if(isButton(dvr_rew)) {
                    BtnClick(dvr_rew); IR_TWC(TWC_CABLE, TWC_CABLE_REWIND);
                }
                if(isButton(dvr_play)) {
                    BtnClick(dvr_play); IR_TWC(TWC_CABLE, TWC_CABLE_PLAY);
                }
                if(isButton(dvr_ffw)) {
                    BtnClick(dvr_ffw); IR_TWC(TWC_CABLE, TWC_CABLE_FFW);
                }
                if(isButton(mute)) {
                    BtnClick(mute); IR_DYNEX(DYNEX, DYNEX_TV,DYNEX_TV_MUTE);
                }
                if(isButton(fav)) {
                    BtnClick(fav); IR_TWC(TWC_CABLE, TWC_CABLE_FAV);
                }
                if(isButton(last)) {
                    BtnClick(last); IR_TWC(TWC_CABLE, TWC_CABLE_LAST);
                }
                if(isButton(dp_up)) {
                    BtnClick(dp_up); IR_TWC(TWC_CABLE, TWC_CABLE_UP);
                }
                if(isButton(dp_down)) {
                    BtnClick(dp_down); IR_TWC(TWC_CABLE, TWC_CABLE_DOWN);
                }
                if(isButton(dp_right)) {
                    BtnClick(dp_right); IR_TWC(TWC_CABLE, TWC_CABLE_RIGHT);
                }
                if(isButton(dp_left)) {
                    BtnClick(dp_left); IR_TWC(TWC_CABLE, TWC_CABLE_LEFT);
                }
                if(isButton(dp_ok)) {
                    BtnClick(dp_ok); IR_TWC(TWC_CABLE, TWC_CABLE_ENTER);
                }
                if(isButton(np_1)) {
                    BtnClick(np_1);  IR_TWC(TWC_CABLE, TWC_CABLE_1);
                }
                if(isButton(np_2)) {
                    BtnClick(np_2);  IR_TWC(TWC_CABLE, TWC_CABLE_2);
                }
                if(isButton(np_3)) {
                    BtnClick(np_3);  IR_TWC(TWC_CABLE, TWC_CABLE_3);
                }
                if(isButton(np_4)) {
                    BtnClick(np_4);  IR_TWC(TWC_CABLE, TWC_CABLE_4);
                }
                if(isButton(np_5)) {
                    BtnClick(np_5);  IR_TWC(TWC_CABLE, TWC_CABLE_5);
                }
                if(isButton(np_6)) {
                    BtnClick(np_6);  IR_TWC(TWC_CABLE, TWC_CABLE_6);
                }
                if(isButton(np_7)) {
                    BtnClick(np_7);  IR_TWC(TWC_CABLE, TWC_CABLE_7);
                }
                if(isButton(np_8)) {
                    BtnClick(np_8);  IR_TWC(TWC_CABLE, TWC_CABLE_8);
                }
                if(isButton(np_9)) {
                    BtnClick(np_9);  IR_TWC(TWC_CABLE, TWC_CABLE_9);
                }
                if(isButton(np_s)) {
                    BtnClick(np_s);  IR_TWC(TWC_CABLE, TWC_CABLE_STAR);
                }
                if(isButton(np_0)) {
                    BtnClick(np_0);  IR_TWC(TWC_CABLE, TWC_CABLE_0);
                }
                if(isButton(np_p)) {
                    BtnClick(np_p);  IR_TWC(TWC_CABLE, TWC_CABLE_POUND);
                }

                if(isButton(ch_dn)) {
                    VOL_CH_Click(ch,1);  IR_TWC(TWC_CABLE, TWC_CABLE_CHDN);
                }
                if(isButton(ch_up)) {
                    VOL_CH_Click(ch,0);  IR_TWC(TWC_CABLE, TWC_CABLE_CHUP);
                }
                if(isButton(vol_dn)) {
                    VOL_CH_Click(vol,3);  IR_DYNEX(DYNEX, DYNEX_TV,DYNEX_TV_VOLDOWN);
                }
                if(isButton(vol_up)) {
                    VOL_CH_Click(vol,2);  IR_DYNEX(DYNEX, DYNEX_TV,DYNEX_TV_VOLUP);
                }
                if(isButton(C_A)) {
                    BtnClick(C_A);  IR_TWC(TWC_CABLE, TWC_CABLE_A);
                }
                if(isButton(C_B)) {
                    BtnClick(C_B);  IR_TWC(TWC_CABLE, TWC_CABLE_B);
                }
                if(isButton(C_C)) {
                    BtnClick(C_C);  IR_TWC(TWC_CABLE, TWC_CABLE_C);
                }
                if(isButton(C_D)) {
                    BtnClick(C_D);  IR_TWC(TWC_CABLE, TWC_CABLE_D);
                }

                Delay1ms(10 );
            }while(isPEN()==0);


            ts_event.Key_Sta=Key_Up;

        }
    }
}
void InitButtons (void)
{
    // TV POWER
    tv_power.top = 32;
    tv_power.left = 57;
    tv_power.height = 71;
    tv_power.width = 62;

    // CablePOWER
    power.top = 32;
    power.left = 678;
    power.height = 71;
    power.width = 62;

    //MAIN BUTTONS
    guide.top = 30;
    guide.left = 231;
    guide.height = 60;
    guide.width = 60;

    menu.top = 30;
    menu.left = 303;
    menu.height = 60;
    menu.width = 60;

    info.top = 30;
    info.left = 373;
    info.height = 60;
    info.width = 60;

    source.top = 30;
    source.left = 443;
    source.height = 60;
    source.width = 60;

    Exit.top = 30;
    Exit.left = 513;
    Exit.height = 60;
    Exit.width = 60;

    //Quick
    mute.top = 336;
    mute.left = 615;
    mute.height = 60;
    mute.width = 60;

    fav.top = 396;
    fav.left = 664;
    fav.height = 60;
    fav.width = 60;
    
    last.top = 336;
    last.left = 713;
    last.height = 60;
    last.width = 60;

    //Num PAD
    np_1.top = 151;
    np_1.left = 318;
    np_1.height = 39;
    np_1.width = 54;

    np_2.top = 151;
    np_2.left = 373;
    np_2.height = 39;
    np_2.width = 54;

    np_3.top = 151;
    np_3.left = 429;
    np_3.height = 39;
    np_3.width = 54;

    np_4.top = 189;
    np_4.left = 318;
    np_4.height = 39;
    np_4.width = 54;

    np_5.top = 189;
    np_5.left = 373;
    np_5.height = 39;
    np_5.width = 54;

    np_6.top = 189;
    np_6.left = 429;
    np_6.height = 39;
    np_6.width = 54;

    np_7.top = 225;
    np_7.left = 318;
    np_7.height = 39;
    np_7.width = 54;

    np_8.top = 225;
    np_8.left = 373;
    np_8.height = 39;
    np_8.width = 54;

    np_9.top = 225;
    np_9.left = 429;
    np_9.height = 39;
    np_9.width = 54;

    np_s.top = 262;
    np_s.left = 318;
    np_s.height = 39;
    np_s.width = 54;

    np_0.top = 262;
    np_0.left = 373;
    np_0.height = 39;
    np_0.width = 54;

    np_p.top = 262;
    np_p.left = 429;
    np_p.height = 39;
    np_p.width = 54;

    //DVR Controls
    dvr_jmp.top = 322;
    dvr_jmp.left = 317;
    dvr_jmp.height = 47;
    dvr_jmp.width = 56;

    dvr_list.top = 322;
    dvr_list.left = 373;
    dvr_list.height = 47;
    dvr_list.width = 56;

    dvr_live.top = 322;
    dvr_live.left = 429;
    dvr_live.height = 47;
    dvr_live.width = 56;

    dvr_stop.top = 371;
    dvr_stop.left = 317;
    dvr_stop.height = 40;
    dvr_stop.width = 56;

    dvr_rec.top = 371;
    dvr_rec.left = 373;
    dvr_rec.height = 40;
    dvr_rec.width = 56;

    dvr_pause.top = 371;
    dvr_pause.left = 429;
    dvr_pause.height = 40;
    dvr_pause.width = 56;

    dvr_rew.top = 408;
    dvr_rew.left = 317;
    dvr_rew.height = 40;
    dvr_rew.width = 56;

    dvr_play.top = 408;
    dvr_play.left = 373;
    dvr_play.height = 40;
    dvr_play.width = 56;

    dvr_ffw.top = 408;
    dvr_ffw.left = 429;
    dvr_ffw.height = 40;
    dvr_ffw.width = 56;

    // Direction pad
    dp_up.top = 188;
    dp_up.left = 673;
    dp_up.height = 38;
    dp_up.width = 38;

    dp_right.top = 232;
    dp_right.left = 717;
    dp_right.height = 38;
    dp_right.width = 38;

    dp_down.top = 276;
    dp_down.left = 673;
    dp_down.height = 38;
    dp_down.width = 38;

    dp_left.top = 232;
    dp_left.left = 627;
    dp_left.height = 38;
    dp_left.width = 38;

    dp_ok.top = 226;
    dp_ok.left = 666;
    dp_ok.height = 48;
    dp_ok.width = 48;

    ch.top = 170;
    ch.left = 502;
    ch.height = 113;
    ch.width = 38;

    vol.top = 170;
    vol.left = 256;
    vol.height = 113;
    vol.width = 38;

    ch_up.top = 170;
    ch_up.left = 502;
    ch_up.width = 38;
    ch_up.height = 56;

    ch_dn.top = 226;
    ch_dn.left = 502;
    ch_dn.width = 38;
    ch_dn.height = 56;

    vol_up.top = 170;
    vol_up.left = 256;
    vol_up.width = 38;
    vol_up.height = 56;

    vol_dn.top = 226;
    vol_dn.left = 256;
    vol_dn.width = 38;
    vol_dn.height = 56;

    C_A.height = 40;
    C_A.width = 71;
    C_A.left = 112;
    C_A.top = 346;

    C_B.height = 40;
    C_B.width = 71;
    C_B.left = 194;
    C_B.top = 346;

    C_C.height = 40;
    C_C.width = 71;
    C_C.left = 112;
    C_C.top = 396;

    C_D.height = 40;
    C_D.width = 71;
    C_D.left = 194;
    C_D.top = 396;

}
void VOL_CH_Click(Button btn, uint ID)
{
    switch(ID)
    {
        case 0: //CH UP
            OpenASI("chup.asi",btn.left,btn.top);
            break;
        case 1: //CH DOWN
            OpenASI("chdn.asi",btn.left,btn.top);
            break;
        case 2: //VOL UP
            OpenASI("volup.asi",btn.left,btn.top);
            break;
        case 3: //VOL DOWN
            OpenASI("voldn.asi",btn.left,btn.top);
            break;
    }

    Delay1ms(1);

    switch(ID)
    {
        case 0: //NO CHAN
        case 1:
            OpenASI("ch.asi",btn.left,btn.top);
            break;
        case 2: //NO VOL
        case 3:
            OpenASI("vol.asi",btn.left,btn.top);
            break;
    }
}
void BtnClick(Button btn)
{
    ReplaceASI("dmain.asi",btn.left,btn.top,btn.width,btn.height);
    //while(isPEN()==0);
    Delay1ms(1);
    ReplaceASI("main.asi",btn.left,btn.top,btn.width,btn.height);
}

void ImgBtnClick(ImageButton btn)
{
    OpenASI(btn.down,btn.left,btn.top);
    Delay1ms(3);
    OpenASI(btn.up,btn.left,btn.top);
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
