#include "ft5206.h"
Touch ts_event;

void delayus(int time)
{
    unsigned int j;
    while(time--)
        for(j=0;j<48;j++);
}

void _nop_()
{
    unsigned char j;
    for(j=0;j<4;j++);
}

char SDA_IN(void)
{
    SDA_HIGH;
    delayus(5);
    return isSDA;
}
//IIC start
void TOUCH_Start(void)
{
	SDA_HIGH;
	_nop_();
	SCL_HIGH;
	delayus(5);
	SDA_LOW;
	delayus(5);
	SCL_LOW;
	_nop_();
}


//IIC stop
void TOUCH_Stop(void)
{
	SDA_LOW;
	_nop_();
	SCL_HIGH;
	delayus(5);
	SDA_HIGH;
	delayus(5);
	SCL_LOW;
	_nop_();
}


//Wait for an answer signal
unsigned char TOUCH_Wait_Ack(void)
{	unsigned char errtime=0;

	SDA_HIGH;
	delayus(1);
	SCL_HIGH;
	delayus(1);
  	while(isSDA == 1)
	{
	    errtime++;
	    if(errtime>250)
            {
              TOUCH_Stop();
              return 1;
            }
	}
	SCL_LOW;
	_nop_();
}




//Acknowledge
void TOUCH_Ack(void)
{	SCL_LOW;
	SDA_LOW;
	delayus(2);
	SCL_HIGH;
	delayus(2);
	SCL_LOW;
	_nop_();
}



//NO Acknowledge
void TOUCH_NAck(void)
{	SCL_LOW;
	SDA_HIGH;
	delayus(2);
	SCL_HIGH;
	delayus(2);
	SCL_LOW;
	_nop_();
}


//IIC send one byte
void TOUCH_Send_Byte(unsigned char data)
{	unsigned char t;

    for(t=0;t<8;t++)
    {
        SCL_LOW;

        if((data & 0x80) > 0)
            SDA_HIGH;
        else
            SDA_LOW;

        data <<=1;
        delayus(2);
        SCL_HIGH;
        delayus(2);
        SCL_LOW;
        delayus(2);
    }


}




/******************************************************************************************
*Function name£ºDraw_Big_Point(u16 x,u16 y)
* Parameter£ºUINT16_t x,UINT16_t y xy
* Return Value£ºvoid
* Function£ºDraw touch pen nib point 3 * 3
*********************************************************************************************/
void Draw_Big_Point(UINT x,UINT y,UINT colour)
{
    Write_Dir(0x40,0x00);//The drawing mode
    MemoryWrite_Position(x,y);//Memory write position
    LCD_CmdWrite(0x02);//Memory write Data
    LCD_DataWrite(colour);
    LCD_DataWrite(colour);
    MemoryWrite_Position(x,y+1);//Memory write position
    LCD_CmdWrite(0x02);////Memory write Data
    LCD_DataWrite(colour);
    LCD_DataWrite(colour);
}



//Read one byte£¬ack=0£¬Send Acknowledge£¬ack=1£¬NO Acknowledge
unsigned char TOUCH_Read_Byte(unsigned char ack)
{
    unsigned char t,receive=0;

    SCL_LOW;
    SDA_HIGH;
    for(t = 0; t < 8; t++)
    {
        _nop_();
        SCL_HIGH;
        _nop_();

        receive <<= 1;

        if(isSDA == 1)
            receive |= 0x01;

        delayus(2);
        SCL_LOW;
        delayus(2);
    }


    if (ack)  TOUCH_NAck();//NO Acknowledge
    else       TOUCH_Ack(); //Send Acknowledge

     return receive;
}


void TOUCH_Wr_Reg(unsigned char RegIndex,unsigned char RegValue1)
{
    TOUCH_Start();
    TOUCH_Send_Byte(WRITE_ADD);
    TOUCH_Wait_Ack();
    TOUCH_Send_Byte(RegIndex);
    TOUCH_Wait_Ack();

    TOUCH_Send_Byte(RegValue1);
    TOUCH_Wait_Ack();

    TOUCH_Stop();
    delayus(100);
}


void TOUCH_RdParFrPCTPFun(unsigned char *PCTP_Par,unsigned char ValFlag)
{	unsigned char k;

    TOUCH_Start();
    TOUCH_Send_Byte(READ_ADD);
    TOUCH_Wait_Ack();
    for(k=0;k<ValFlag;k++)
    {
        if(k == (ValFlag-1))
            *PCTP_Par++ = TOUCH_Read_Byte(1);
        else
            *PCTP_Par++ = TOUCH_Read_Byte(0);
    }
    TOUCH_Stop();
}


unsigned char TOUCH_Read_Reg(unsigned char RegIndex)
{
    unsigned char receive=0;

    TOUCH_Start();
    TOUCH_Send_Byte(WRITE_ADD);
    TOUCH_Wait_Ack();
    TOUCH_Send_Byte(RegIndex);
    TOUCH_Wait_Ack();

    TOUCH_Start();
    TOUCH_Send_Byte(READ_ADD);
    TOUCH_Wait_Ack();
    receive=TOUCH_Read_Byte(1);
    TOUCH_Stop();

    return receive;
}


void ft5x0x_i2c_txdata(unsigned char *txdata, unsigned char length)
{
    unsigned char ret =0;	UINT num;

    TOUCH_Start();
    TOUCH_Send_Byte(WRITE_ADD);
    TOUCH_Wait_Ack();
    for(num=0;num<length;num++)
    {
        TOUCH_Send_Byte(*txdata++);
        TOUCH_Wait_Ack();
    }
    TOUCH_Stop();
    Delay1ms(2);
}

unsigned char ft5x0x_i2c_rxdata(unsigned char *rxdata, unsigned char length)
{
    unsigned char num;

    TOUCH_Start();
    TOUCH_Send_Byte(READ_ADD);
    TOUCH_Wait_Ack();
    for(num=0;num<length;num++)
    {
        if(num==(length-1))
            *rxdata++ = TOUCH_Read_Byte(0);
        else
            *rxdata++ = TOUCH_Read_Byte(1);
    }

    TOUCH_Stop();

    return num;
}

unsigned char ft5x0x_read_data(void)
{
    unsigned char buf[32] = {0}; unsigned char ret = 0;

    #ifdef CONFIG_FT5X0X_MULTITOUCH
        TOUCH_RdParFrPCTPFun(buf, 31);
    #else
        TOUCH_RdParFrPCTPFun(buf, 7);
    #endif

    ts_event.touch_point = buf[2] & 0xf;

    if (ts_event.touch_point == 0)
    {
        return 0;
    }

    #ifdef CONFIG_FT5X0X_MULTITOUCH
    switch (ts_event.touch_point)
    {
        case 5:
        ts_event.x5 = (UINT)(buf[0x1b] & 0x0F)<<8 | (UINT)buf[0x1c];
        ts_event.y5 = (UINT)(buf[0x1d] & 0x0F)<<8 | (UINT)buf[0x1e];

        case 4:
        ts_event.x4 = (UINT)(buf[0x15] & 0x0F)<<8 | (UINT)buf[0x16];
        ts_event.y4 = (UINT)(buf[0x17] & 0x0F)<<8 | (UINT)buf[0x18];

        case 3:
        ts_event.x3 = (UINT)(buf[0x0f] & 0x0F)<<8 | (UINT)buf[0x10];
        ts_event.y3 = (UINT)(buf[0x11] & 0x0F)<<8 | (UINT)buf[0x12];

        case 2:
        ts_event.x2 = (UINT)(buf[9] & 0x0F)<<8 | (UINT)buf[10];
        ts_event.y2 = (UINT)(buf[11] & 0x0F)<<8 | (UINT)buf[12];

        case 1:
        ts_event.x1 = (UINT)(buf[3] & 0x0F)<<8 | (UINT)buf[4];
        ts_event.y1 = (UINT)(buf[5] & 0x0F)<<8 | (UINT)buf[6];

        break;
        default:
        return 0;
    }
    #else

    if (ts_event.touch_point == 1)
    {
        ts_event.x1 = (UINT)(buf[3] & 0x0F)<<8 | (UINT)buf[4];
        ts_event.y1 = (UINT)(buf[5] & 0x0F)<<8 | (UINT)buf[6];
        ret = 1;
    }
    else
    {
        ts_event.x1 = 0xFFFF;
        ts_event.y1 = 0xFFFF;
        ret = 0;
    }
    #endif


    return ret;
}


void inttostr(UINT value,unsigned char *str)
{
    *str++ = (value/100)+48;
    *str++ = ((value%100)/10)+48;
    *str   = ((value%100)%10)+48;
}


////////////////////////////////////

char CheckPen(void)
{
    if(isPEN()==0)										//Detect the occurrence of an interrupt
    {
        ts_event.Key_Sta=Key_Down;
        return 0;
    }
    return 1;
}

void TOUCH_Init(void)
{
    SDA_HIGH;
    SCL_HIGH;

/*
        TOUCH_Start();
    TOUCH_Send_Byte(WRITE_ADD);
    TOUCH_Wait_Ack();
    TOUCH_Send_Byte(FT5206_DEVICE_MODE);
    TOUCH_Wait_Ack();
    TOUCH_Send_Byte(0);
    TOUCH_Wait_Ack();

    TOUCH_Stop();
    delayus(100);
*/
    //RESET_LOW();
    //Delay1ms(10);
    //RESET_HIGH();
    //Delay1ms(10);
}

/*
void TPTEST(void)
{
	unsigned char ss[3];
	 IT0=1;        //Falling edge trigger
	 EX0=1;
	 EA=1;

	 TOUCH_Init();

        Active_Window(0,799,0,479);//Set the working window size
        Text_Foreground_Color1(color_white);//Set the foreground color
        Text_Background_Color1(color_black);//Set the background color
        Write_Dir(0X8E,0X80);//Began to clear the screen (display window)
        Chk_Busy();


	while(next)
	{
            Write_Dir(0x40,0x80);//Set the character mode
            Write_Dir(0x21,0x10);//Select the internal CGROM  ISO/IEC 8859-1.
            Write_Dir(0x22,0x00);//Full alignment is disable.The text background color . Text don't rotation. 2x zoom
            FontWrite_Position(40,200);
            String("Touch to display the coordinate");

            if(isPEN() == 0)        //The touch screen is pressed
            {
                    EX0=0;//Close interrupt
                    do
                    {
                            ft5x0x_read_data(ts_event);
                            ts_event.Key_Sta=Key_Up;


                            inttostr(ts_event.x1,ss);

                            FontWrite_Position(100,60);   //Set the display position
                            LCD_CmdWrite(0x02);
                            String("X = ");
                            LCD_DataWrite(ss[0]);
                            //Delay1ms(1);
                            LCD_DataWrite(ss[1]);
                            //Delay1ms(1);
                            LCD_DataWrite(ss[2]);
                            //Delay1ms(1);

                            inttostr(ts_event.y1,ss);
                            FontWrite_Position(100, 140);   //Set the display position
                            LCD_CmdWrite(0x02);
                            String("Y = ");
                            LCD_DataWrite(ss[0]);
                            //Delay1ms(1);
                            LCD_DataWrite(ss[1]);
                            //Delay1ms(1);
                            LCD_DataWrite(ss[2]);
                            //Delay1ms(1);



                            Draw_Big_Point(ts_event.x1,ts_event.y1,color_red);
                            Draw_Big_Point(ts_event.x2,ts_event.y2,color_green);
                            Draw_Big_Point(ts_event.x3,ts_event.y3,color_blue);
                            Draw_Big_Point(ts_event.x4,ts_event.y4,color_cyan);
                            Draw_Big_Point(ts_event.x5,ts_event.y5,color_purple);


                    }while(PEN==0);
                    EX0=1;
            }



    }



}
 * */
