#include <xc.h>
#include <plib.h>
#include "ir.h"

// REMOTE OUTLINE
# define ZERO 0
# define ONE 1
# define START 2
# define MISC 3
//TWC 58KHz 1/2 duty , 8 HIGH, 9 LOW
void IR_TWC(unsigned char device, unsigned char cmd)
{
    unsigned char Ndevice = ~device;
    unsigned char count = 0;
    unsigned char Ncmd = ~cmd;

    IR_PULSE_TWC(START);

    for(count = 0; count < 5; count++)
    {
        if(device & 0x10)
            IR_PULSE_TWC(ONE);
        else
            IR_PULSE_TWC(ZERO);

        device<<=1;
    }
    for(count = 0; count < 6; count++)
    {
        if(cmd & 0x20)
            IR_PULSE_TWC(ONE);
        else
            IR_PULSE_TWC(ZERO);

        cmd<<=1;
    }
    for(count = 0; count < 5; count++)
    {
        if(Ndevice & 0x10)
            IR_PULSE_TWC(ONE);
        else
            IR_PULSE_TWC(ZERO);

        Ndevice<<=1;
    }
    for(count = 0; count < 6; count++)
    {
        if(Ncmd & 0x20)
            IR_PULSE_TWC(ONE);
        else
            IR_PULSE_TWC(ZERO);

        Ncmd<<=1;
    }


    IR_PULSE_TWC(MISC);
}

void TestDelay(void)
{
    while(1)
    {
        IR_PIN = 0;
        delay_us(1);
        IR_PIN = 1;
        delay_us(1);
    }
}
void IR_PULSE_TWC (unsigned char type)
{

    unsigned int time;

    switch(type)
    {
        case START:
            time = 195;
            while(time--)
            {
                IR_PIN = 0;
                delay_us(8);
                IR_PIN = 1;
                delay_us(9);
            }

            IR_PIN = 0;
            delay_ms(3);
            delay_us(330);
            break;
        case ONE:
            time = 48;
            while(time--)
            {
                IR_PIN = 0;
                delay_us(8);
                IR_PIN = 1;
                delay_us(9);
            }

            IR_PIN = 0;
            delay_ms(2);
            delay_us(540);
            break;
        case ZERO:
            time = 48;
            while(time--)
            {
                IR_PIN = 0;
                delay_us(8);
                IR_PIN = 1;
                delay_us(9);
            }

            IR_PIN = 0;
            delay_us(837);
            break;
        case MISC:
            time = 48;
            while(time--)
            {
                IR_PIN = 0;
                delay_us(8);
                IR_PIN = 1;
                delay_us(9);
            }
            IR_PIN = 0;
            break;

    }

}
//DYNEX 38KHz 2/3 duty, 8.875 LOW, 17.500 High

void IR_DYNEX(unsigned char dat1, unsigned char dat2, unsigned char data)
{
    unsigned char reci = 0;
    unsigned char count = 0;
    unsigned char dat0 = data;

    for(count = 0; count < 8; count++)
    {
        reci>>=1;

        if(dat0 & 0x01)
            reci &= 0x7F;
        else
            reci |= 0x80;

        dat0>>=1;
    }

    IR_PULSE_DYNEX(START);

    for(count = 0; count < 8; count++)
    {
        if(dat1 & 0x80)
            IR_PULSE_DYNEX(ONE);
        else
            IR_PULSE_DYNEX(ZERO);

        dat1<<=1;
    }

    for(count = 0; count < 8; count++)
    {
        if(dat2 & 0x80)
            IR_PULSE_DYNEX(ONE);
        else
            IR_PULSE_DYNEX(ZERO);

        dat2<<=1;
    }

    for(count = 0; count < 8; count++)
    {
        if(data & 0x80)
            IR_PULSE_DYNEX(ONE);
        else
            IR_PULSE_DYNEX(ZERO);

        data<<=1;
    }

    for(count = 0; count < 8; count++)
    {
        if(reci & 0x80)
            IR_PULSE_DYNEX(ONE);
        else
            IR_PULSE_DYNEX(ZERO);

        reci<<=1;
    }

    IR_PULSE_DYNEX(MISC);

}

void IR_PULSE_DYNEX (unsigned char type)
{

    unsigned int time;//0.00000001

    switch(type)
    {
        case START:
            time = 344;
            while(time--)
            {
                IR_PIN = 1;
                delay_us(8);
                IR_PIN = 0;
                delay_us(17);
            }

            IR_PIN = 1;
            delay_ms(4);
            delay_us(700);//delay_us(475);
            break;
        case ONE:
            time = 21;
            while(time--)
            {
                IR_PIN = 1;
                delay_us(8);
                IR_PIN = 0;
                delay_us(17);
            }

            IR_PIN = 1;
            delay_ms(1);
            delay_us(665);
            break;
        case ZERO:
            time = 21;
            while(time--)
            {
                IR_PIN = 1;
                delay_us(8);
                IR_PIN = 0;
                delay_us(17);
            }

            IR_PIN = 1;
            delay_us(665);
            break;
        case MISC:
            time = 21;
            while(time--)
            {
                IR_PIN = 1;
                delay_us(8);
                IR_PIN = 0;
                delay_us(17);
            }
            IR_PIN = 1;
            break;

    }

}

void delay_us(int time)
{
    while(time--)
    {
        Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();
        Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();
        Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();
    }
}
void delay_ms(int time)
{
    while(time--)
        delay_us(1000);
}

/*
 2.083333333333333e-8
 * 48MHz = 20.83ns
 * 0.0000000208333333
 * 0.000 000 020 833 333 333 333 330 sec
 *
 * 1us = 48.0000000768 cycles
 * 1ms = 48000.0000768 cycles
 *
 * for loop = ~ 7 cycles
 * while = ~ 7 cycles
 */