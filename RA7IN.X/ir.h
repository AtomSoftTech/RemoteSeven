/* 
 * File:   ir.h
 * Author: Jason
 *
 * Created on April 5, 2014, 10:34 AM
 */

#ifndef IR_H
#define	IR_H

#define DYNEX 0x61

void delay_ms(int time);
void delay_us(int time);
void delay_tns(int time);

void IR_TWC(unsigned char device, unsigned char cmd);
void IR_DYNEX(unsigned char dat1, unsigned char dat2, unsigned char data);

void IR_PULSE_TWC (unsigned char type);
void IR_PULSE_DYNEX (unsigned char type);

void TestDelay(void);

#define IR_PIN LATBbits.LATB13
#define init_ir() TRISBbits.TRISB13 = 0


#endif	/* IR_H */

