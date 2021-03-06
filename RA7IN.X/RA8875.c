//----------------------------------------------------------------------
//EASTRISING TECHNOLOGY CO,.LTD.//
// Module    : ER-TFTM070-5  7.0 INCH TFT LCD  800*480   7 INCH  Resistive Touch Screen
// Lanuage   : C51 Code
// Create    : JAVEN LIU
// Date      : Nov-30-2013
// Drive IC  : RA8875    Font chip  23L32S4W    Flash:128M char
// INTERFACE : 4-Wire SPI
// MCU 		 : STC12LE5C60S2  //51 series  1T MCU
// MCU VDD		 : 3.3V   
//----------------------------------------------------------------------
#include "spi2.h"
#include "ra8875.h"
#include "ff.h"
#include "diskio.h"
#include "ft5206.h"

#define write_data_addr  0x0c  //slave addresses with write data
#define read_data_addr  0x0d  //slave addresses with write data
#define write_cmd_addr  0x0e  //slave addresses with write command
#define read_cmd_addr  0x0f  //slave addresses with read status

unsigned int X1,Y1,X2,Y2,X3,Y3,X4,Y4;
unsigned char taby[4];
unsigned char tabx[4];
uint x[6],y[6],xmin,ymin,xmax,ymax;


volatile uchar IMAGE_BUFF[IMAGEBUFF_LEN];

void Delay1ms(uint i)
{
    unsigned int j;
    while(i--)
        for(j=0;j<48000;j++);
}


void Delay10ms(uint i)
{
    while(i--)
        Delay1ms(10);
}

void Delay100ms(uint i)
{
    while(i--)
	Delay1ms(100);
}

void NextStep(void)
{
     Delay10ms(2);
    /*while(next)
    {
        Delay100ms(1);
    }
    while(!next);
    Delay100ms(12);
    while(!next);
    */
}


//*********4W_SPI_Init()
void SPI_Init(void)
{
//    SCLK = 1;
//    SDI = 1;
//    SDO = 1;
//    SCS = 1;
    CS_HIGH();
    SCK_HIGH();
}


//////////////SPI Write command
void LCD_CmdWrite(unsigned char cmd)
{	
    //SCLK = 1;
    //SDI = 1;
    CS_LOW(LCD);//SCS = 0;
    //SPI_Delay();
    SPI_Write(0x80);
    SPI_Write(cmd);
    CS_HIGH();//SCS = 1;
    //SPI_Delay();
}

//////////////SPI Write data or  parameter
void LCD_DataWrite(unsigned char Data)
{
    //SCLK = 1;
    //SDI = 1;
    CS_LOW(LCD);//SCS = 0;
    SPI_Write(0x00);
    SPI_Write(Data);
    //SPI_Delay();
    CS_HIGH();//SCS = 1;
}

///////////////Read data or  parameter
unsigned char LCD_DataRead(void)
{
    unsigned char Data;
    //SCLK = 1;
    //SDO = 1;
    CS_LOW(LCD);//SCS = 0;
    SPI_Write(0x40);
    Data = SPI_Read();
    //SPI_Delay();
    CS_HIGH();//SCS = 1;
    return Data;
}


///////////////Read data to buff
void LCD_ReadBuff(char *buff, uint len)
{

    CS_LOW(LCD);//SCS = 0;
    SPI_Write(0x40);

    while(len--)
        *buff++ = SPI_Read();

    CS_HIGH();//SCS = 1;

}

void WriteCommandW(uchar command, uint data)
{
    WriteCommand(command, data & 0xFF);
    WriteCommand(command+1, data >> 8);
}
void WriteCommand(unsigned char command, unsigned int data)
{
    CS_LOW(LCD);
    SPI_Write(0x80);         // cmd: write command
    SPI_Write(command);
    if (data <= 0xFF) {   // only if in the valid range
        SPI_Write(0x00);
        SPI_Write(data);
    }
    CS_HIGH();
}
void SetGraphicsCursorRead(uint x, uint y)
{
    //WriteCommand(0x40, 0);  // Graphics mode
    //WriteCommand(0x45, 0);  // left->right, top->bottom
    WriteCommand(0x4A, x& 0xFF);
    WriteCommand(0x4B, (x>>8));
    WriteCommand(0x4C, y& 0xFF);
    WriteCommand(0x4D, (y>>8));

}

void SetGraphicsCursorWrite(uint x, uint y)
{
    //WriteCommand(0x40, 0);  // Graphics mode
    //WriteCommand(0x45, 0);  // left->right, top->bottom
    WriteCommand(0x46, x & 0xFF);
    WriteCommand(0x47, (x>>8)&3);
    WriteCommand(0x48, y & 0xFF);
    WriteCommand(0x49, (y>>8)&1);

}

void getPixelStream(uchar * p, uint count, uint x, uint y)
{

    WriteCommand(0x40,0x00);    // Graphics write mode
    SetGraphicsCursorRead(x, y);
    LCD_CmdWrite(0x02);
    CS_LOW(LCD);
    SPI_Write(0x40);         // Cmd: read data
    SPI_Write(0x00);         // dummy read
    while (count--) {
        *p++ = SPI_Read();//pixel;
    }
    CS_HIGH();
    
}
void putPixelStream(uchar * p, uint count, uint x, uint y)
{
    WriteCommand(0x40,0x00);    // Graphics write mode
    SetGraphicsCursorRead(x, y);
    LCD_CmdWrite(0x02);
    CS_LOW(LCD);
    SPI_Write(0x00);         // Cmd: write data
    while (count--) {
        SPI_Write(*p++);
    }
    CS_HIGH();
}
////////////////Write command and parameter
void Write_Dir(unsigned char Cmd,unsigned char Data)
{
    LCD_CmdWrite(Cmd);
    LCD_DataWrite(Data);
}

///////////SPI Read  status
unsigned char LCD_StatusRead(void)
{
    unsigned char Data;
    //SCLK = 1;
    //SDO = 1;
    CS_LOW(LCD);//SCS = 0;
    //SPI_Delay();
    SPI_Write(0xc0);
    Data = SPI_Read();
    //SPI_Delay();
    CS_HIGH();//SCS = 1;
    return Data;
}

////////LCM reset
void LCD_Reset(void)
{
    LCD_RST_LOW();//MCU_RST = 0;
    Delay1ms(1);
    LCD_RST_HIGH();//MCU_RST = 1;
    Delay1ms(1);
}	

///////////////check busy
void Chk_Busy(void)
{
    unsigned char temp;
    do
    {
        temp=LCD_StatusRead();
    }while((temp&0x80)==0x80);
}
///////////////check bte busy
void Chk_BTE_Busy(void)
{
    unsigned char temp;
    do
    {
        temp=LCD_StatusRead();
    }while((temp&0x40)==0x40);
}
///////////////check dma busy
void Chk_DMA_Busy(void)
{
    unsigned char temp;
    do
    {
        LCD_CmdWrite(0xBF);
        temp =LCD_DataRead();
    }while((temp&0x01)==0x01);
}

/////////////PLL setting
void PLL_ini(void)
{
    LCD_CmdWrite(0x88);      
    LCD_DataWrite(0x0C);
    Delay1ms(1);     
    LCD_CmdWrite(0x89);        
    LCD_DataWrite(0x02);  
    Delay1ms(1);
}

char waitPoll(uchar regname, uchar waitflag) {
  /* Wait for the command to finish */
  while (1)
  {
    uchar temp;
    LCD_CmdWrite(regname);//INTC

    temp = LCD_DataRead();
    if (!(temp & waitflag))
      return 1;
  }

  return 0; // MEMEFIX: yeah i know, unreached! - add timeout?
}

/////////////////Set the working window area
void Active_Window(uint XL,uint XR ,uint YT ,uint YB)
{
    unsigned char temp;
    //setting active window X
    temp=XL;
    LCD_CmdWrite(0x30);//HSAW0
    LCD_DataWrite(temp);
    temp=XL>>8;
    LCD_CmdWrite(0x31);//HSAW1
    LCD_DataWrite(temp);

    temp=XR;
    LCD_CmdWrite(0x34);//HEAW0
    LCD_DataWrite(temp);
    temp=XR>>8;
    LCD_CmdWrite(0x35);//HEAW1
    LCD_DataWrite(temp);

    //setting active window Y
    temp=YT;
    LCD_CmdWrite(0x32);//VSAW0
    LCD_DataWrite(temp);
    temp=YT>>8;
    LCD_CmdWrite(0x33);//VSAW1
    LCD_DataWrite(temp);

    temp=YB;
    LCD_CmdWrite(0x36);//VEAW0
    LCD_DataWrite(temp);
    temp=YB>>8;
    LCD_CmdWrite(0x37);//VEAW1
    LCD_DataWrite(temp);
}




/////////////LCM initial
void LCD_Initial(void)
{ 	
    PLL_ini();
    LCD_CmdWrite(0x10);	 //SYSR   char[4:3] color  char[2:1]=  MPU interface
    LCD_DataWrite(0x0c);   //            65K

    LCD_CmdWrite(0x04);    //PCLK
    LCD_DataWrite(0x81);   //
    Delay1ms(1);

     //Horizontal set
    LCD_CmdWrite(0x14); //HDWR//Horizontal Display Width Setting char[6:0]
    LCD_DataWrite(0x63);//Horizontal display width(pixels) = (HDWR + 1)*8
    LCD_CmdWrite(0x15);//Horizontal Non-Display Period Fine Tuning Option Register (HNDFTR)
    LCD_DataWrite(0x00);//Horizontal Non-Display Period Fine Tuning(HNDFT) [3:0]
    LCD_CmdWrite(0x16); //HNDR//Horizontal Non-Display Period char[4:0]
    LCD_DataWrite(0x03);//Horizontal Non-Display Period (pixels) = (HNDR + 1)*8
    LCD_CmdWrite(0x17); //HSTR//HSYNC Start Position[4:0]
    LCD_DataWrite(0x03);//HSYNC Start Position(PCLK) = (HSTR + 1)*8
    LCD_CmdWrite(0x18); //HPWR//HSYNC Polarity ,The period width of HSYNC.
    LCD_DataWrite(0x0B);//HSYNC Width [4:0]   HSYNC Pulse width(PCLK) = (HPWR + 1)*8
     //Vertical set
    LCD_CmdWrite(0x19); //VDHR0 //Vertical Display Height char [7:0]
    LCD_DataWrite(0xdf);//Vertical pixels = VDHR + 1
    LCD_CmdWrite(0x1a); //VDHR1 //Vertical Display Height char [8]
    LCD_DataWrite(0x01);//Vertical pixels = VDHR + 1
    LCD_CmdWrite(0x1b); //VNDR0 //Vertical Non-Display Period char [7:0]
    LCD_DataWrite(0x20);//Vertical Non-Display area = (VNDR + 1)
    LCD_CmdWrite(0x1c); //VNDR1 //Vertical Non-Display Period char [8]
    LCD_DataWrite(0x00);//Vertical Non-Display area = (VNDR + 1)
    LCD_CmdWrite(0x1d); //VSTR0 //VSYNC Start Position[7:0]
    LCD_DataWrite(0x16);//VSYNC Start Position(PCLK) = (VSTR + 1)
    LCD_CmdWrite(0x1e); //VSTR1 //VSYNC Start Position[8]
    LCD_DataWrite(0x00);//VSYNC Start Position(PCLK) = (VSTR + 1)
    LCD_CmdWrite(0x1f); //VPWR //VSYNC Polarity ,VSYNC Pulse Width[6:0]
    LCD_DataWrite(0x01);//VSYNC Pulse Width(PCLK) = (VPWR + 1)


    Active_Window(0,799,0,479);

    LCD_CmdWrite(0x8a);//PWM setting
    LCD_DataWrite(0x80);
    LCD_CmdWrite(0x8a);//PWM setting
    LCD_DataWrite(0x81);//open PWM
    LCD_CmdWrite(0x8b);//Backlight brightness setting
    LCD_DataWrite(0xff);//Brightness parameter 0xff-0x00
}


///////////////Background color settings
void Text_Background_Color1(uint b_color)
{

    LCD_CmdWrite(0x60);//BGCR0
    LCD_DataWrite((unsigned char)(b_color>>11));

    LCD_CmdWrite(0x61);//BGCR0
    LCD_DataWrite((unsigned char)(b_color>>5));

    LCD_CmdWrite(0x62);//BGCR0
    LCD_DataWrite((unsigned char)(b_color));
} 

///////////////Background color settings
void Text_Background_Color(unsigned char setR, uchar setG, uchar setB)
{
    LCD_CmdWrite(0x60);//BGCR0
    LCD_DataWrite(setR);

    LCD_CmdWrite(0x61);//BGCR0
    LCD_DataWrite(setG);

    LCD_CmdWrite(0x62);//BGCR0
    LCD_DataWrite(setB);
} 

////////////////Foreground color settings
void Text_Foreground_Color1(uint b_color)
{

    LCD_CmdWrite(0x63);//BGCR0
    LCD_DataWrite((unsigned char)(b_color>>11));

    LCD_CmdWrite(0x64);//BGCR0
    LCD_DataWrite((unsigned char)(b_color>>5));

    LCD_CmdWrite(0x65);//BGCR0
    LCD_DataWrite((unsigned char)(b_color));
} 

////////////////Foreground color settings
void Text_Foreground_Color(unsigned char setR, uchar setG, uchar setB)
{	    
    LCD_CmdWrite(0x63);//BGCR0
    LCD_DataWrite(setR);

    LCD_CmdWrite(0x64);//BGCR0
    LCD_DataWrite(setG);

    LCD_CmdWrite(0x65);//BGCR0��
    LCD_DataWrite(setB);
}
//////////////////BTE area size settings
void BTE_Size(uint width,uint height)
{
    unsigned char temp;
    temp=width;
    LCD_CmdWrite(0x5c);//BET area width literacy
    LCD_DataWrite(temp);
    temp=width>>8;
    LCD_CmdWrite(0x5d);//BET area width literacy
    LCD_DataWrite(temp);

    temp=height;
    LCD_CmdWrite(0x5e);//BET area height literacy
    LCD_DataWrite(temp);
    temp=height>>8;
    LCD_CmdWrite(0x5f);//BET area height literacy
    LCD_DataWrite(temp);
}		

////////////////////BTE starting position
void BTE_Source(uint SX,uint DX ,uint SY ,uint DY)
{
    unsigned char temp,temp1;

    temp=SX;
    LCD_CmdWrite(0x54);//BTE horizontal position of read/write data
    LCD_DataWrite(temp);
    temp=SX>>8;
    LCD_CmdWrite(0x55);//BTE horizontal position of read/write data
    LCD_DataWrite(temp);

    temp=DX;
    LCD_CmdWrite(0x58);//BET written to the target horizontal position
    LCD_DataWrite(temp);
    temp=DX>>8;
    LCD_CmdWrite(0x59);//BET written to the target horizontal position
    LCD_DataWrite(temp);

    temp=SY;
    LCD_CmdWrite(0x56);//BTE vertical position of read/write data
    LCD_DataWrite(temp);
    temp=SY>>8;
    LCD_CmdWrite(0x57);
    temp1 = LCD_DataRead();
    temp1 &= 0x80;
    temp=temp|temp1;
    LCD_CmdWrite(0x57);//BTE vertical position of read/write data
    LCD_DataWrite(temp);

    temp=DY;
    LCD_CmdWrite(0x5a);//BET written to the target  vertical  position
    LCD_DataWrite(temp);
    temp=DY>>8;
    LCD_CmdWrite(0x5b);
    temp1 = LCD_DataRead();
    temp1 &= 0x80;
    temp=temp|temp1;
    LCD_CmdWrite(0x5b);//BET written to the target  vertical  position
    LCD_DataWrite(temp);
}				
///////////////Memory write position
void MemoryWrite_Position(uint X,uint Y)
{
    unsigned char temp;

    temp=X;
    LCD_CmdWrite(0x46);
    LCD_DataWrite(temp);
    temp=X>>8;
    LCD_CmdWrite(0x47);
    LCD_DataWrite(temp);

    temp=Y;
    LCD_CmdWrite(0x48);
    LCD_DataWrite(temp);
    temp=Y>>8;
    LCD_CmdWrite(0x49);
    LCD_DataWrite(temp);
}

////////////////Text write position
void FontWrite_Position(uint X,uint Y)
{
    unsigned char temp;
    temp=X;
    LCD_CmdWrite(0x2A);
    LCD_DataWrite(temp);
    temp=X>>8;
    LCD_CmdWrite(0x2B);
    LCD_DataWrite(temp);

    temp=Y;
    LCD_CmdWrite(0x2C);
    LCD_DataWrite(temp);
    temp=Y>>8;
    LCD_CmdWrite(0x2D);
    LCD_DataWrite(temp);
}

//////////////writing text
void String(unsigned char *str)
{   
    Write_Dir(0x40,0x80);//Set the character mode
    LCD_CmdWrite(0x02);
    while(*str != '\0')
    {
        LCD_DataWrite(*str);
        ++str;
        Chk_Busy();
    }
}

		

/////////////////Scroll window size
void Scroll_Window(uint XL,uint XR ,uint YT ,uint YB)
{
    unsigned char temp;
    temp=XL;
    LCD_CmdWrite(0x38);//HSSW0
    LCD_DataWrite(temp);
    temp=XL>>8;
    LCD_CmdWrite(0x39);//HSSW1
    LCD_DataWrite(temp);

    temp=XR;
    LCD_CmdWrite(0x3c);//HESW0
    LCD_DataWrite(temp);
    temp=XR>>8;
    LCD_CmdWrite(0x3d);//HESW1
    LCD_DataWrite(temp);

    temp=YT;
    LCD_CmdWrite(0x3a);//VSSW0
    LCD_DataWrite(temp);
    temp=YT>>8;
    LCD_CmdWrite(0x3b);//VSSW1
    LCD_DataWrite(temp);

    temp=YB;
    LCD_CmdWrite(0x3e);//VESW0
    LCD_DataWrite(temp);
    temp=YB>>8;
    LCD_CmdWrite(0x3f);//VESW1
    LCD_DataWrite(temp);
}  

///////////////Window scroll offset Settings
void Scroll(uint X,uint Y)
{
    unsigned char temp;

    temp=X;
    LCD_CmdWrite(0x24);//HOFS0
    LCD_DataWrite(temp);
    temp=X>>8;
    LCD_CmdWrite(0x25);//HOFS1
    LCD_DataWrite(temp);

    temp=Y;
    LCD_CmdWrite(0x26);//VOFS0
    LCD_DataWrite(temp);
    temp=Y>>8;
    LCD_CmdWrite(0x27);//VOFS1
    LCD_DataWrite(temp);
}	   	  

///////////////The FLASH reading area   setting
void DMA_block_mode_size_setting(uint BWR,uint BHR,uint SPWR)
{
    LCD_CmdWrite(0xB4);
    LCD_DataWrite(BWR);
    LCD_CmdWrite(0xB5);
    LCD_DataWrite(BWR>>8);

    LCD_CmdWrite(0xB6);
    LCD_DataWrite(BHR);
    LCD_CmdWrite(0xB7);
    LCD_DataWrite(BHR>>8);

    LCD_CmdWrite(0xB8);
    LCD_DataWrite(SPWR);
    LCD_CmdWrite(0xB9);
    LCD_DataWrite(SPWR>>8);
}
void fillRect(void) {
  LCD_CmdWrite(RA8875_DCR);
  LCD_DataWrite(RA8875_DCR_LINESQUTRI_STOP | RA8875_DCR_DRAWSQUARE);
  LCD_DataWrite(RA8875_DCR_LINESQUTRI_START | RA8875_DCR_FILL | RA8875_DCR_DRAWSQUARE);
}
void drawRect(uint x, uint y, uint w, uint h, uint color, char filled)
{
    w += x;
    h += y;

  /* Set X */
  LCD_CmdWrite(0x91);
  LCD_DataWrite(x);
  LCD_CmdWrite(0x92);
  LCD_DataWrite(x >> 8);

  /* Set Y */
  LCD_CmdWrite(0x93);
  LCD_DataWrite(y);
  LCD_CmdWrite(0x94);
  LCD_DataWrite(y >> 8);

  /* Set X1 */
  LCD_CmdWrite(0x95);
  LCD_DataWrite(w);
  LCD_CmdWrite(0x96);
  LCD_DataWrite((w) >> 8);

  /* Set Y1 */
  LCD_CmdWrite(0x97);
  LCD_DataWrite(h);
  LCD_CmdWrite(0x98);
  LCD_DataWrite((h) >> 8);

  /* Set Color */
  LCD_CmdWrite(0x63);
  LCD_DataWrite((color & 0xf800) >> 11);
  LCD_CmdWrite(0x64);
  LCD_DataWrite((color & 0x07e0) >> 5);
  LCD_CmdWrite(0x65);
  LCD_DataWrite((color & 0x001f));

  /* Draw! */
  LCD_CmdWrite(RA8875_DCR);
  if (filled)
  {
    LCD_DataWrite(0xB0);
  }
  else
  {
    LCD_DataWrite(0x90);
  }

  /* Wait for the command to finish */
  waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}
/////////////FLASH read start position Settings
void DMA_Start_address_setting(ulong set_address)
{ 
    LCD_CmdWrite(0xB0);
    LCD_DataWrite(set_address);

    LCD_CmdWrite(0xB1);
    LCD_DataWrite(set_address>>8);

    LCD_CmdWrite(0xB2);
    LCD_DataWrite(set_address>>16);

    LCD_CmdWrite(0xB3);
    LCD_DataWrite(set_address>>24);
}
///////////drawing circle
void  Draw_Circle(uint X,uint Y,uint R)
{
    unsigned char temp;

    temp=X;
    LCD_CmdWrite(0x99);
    LCD_DataWrite(temp);
    temp=X>>8;
    LCD_CmdWrite(0x9a);
    LCD_DataWrite(temp);

    temp=Y;
    LCD_CmdWrite(0x9b);
    LCD_DataWrite(temp);
    temp=Y>>8;
    LCD_CmdWrite(0x9c);
    LCD_DataWrite(temp);

    temp=R;
    LCD_CmdWrite(0x9d);
    LCD_DataWrite(temp);


} 

///////////drawing elliptic curve
void  Draw_Ellipse(uint X,uint Y,uint R1,uint R2)
{
    unsigned char temp;
    temp=X;
    LCD_CmdWrite(0xA5);
    LCD_DataWrite(temp&0xff);
    temp=X>>8;
    LCD_CmdWrite(0xA6);
    LCD_DataWrite(temp);

    temp=Y;
    LCD_CmdWrite(0xA7);
    LCD_DataWrite(temp&0xff);
    temp=Y>>8;
    LCD_CmdWrite(0xA8);
    LCD_DataWrite(temp);

    temp=R1;
    LCD_CmdWrite(0xA1);
    LCD_DataWrite(temp&0xff);
    temp=R1>>8;
    LCD_CmdWrite(0xA2);
    LCD_DataWrite(temp);

    temp=R2;
    LCD_CmdWrite(0xA3);
    LCD_DataWrite(temp&0xff);
    temp=R2>>8;
    LCD_CmdWrite(0xA4);
    LCD_DataWrite(temp);
} 

///////////drawing line, uchar rectangle, uchar triangle
void Draw_Line(uint XS,uint XE ,uint YS,uint YE)
{	
    unsigned char temp;    
    temp=XS;
    LCD_CmdWrite(0x91);
    LCD_DataWrite(temp&0xff);
    temp=XS>>8;
    LCD_CmdWrite(0x92);
    LCD_DataWrite(temp);

    temp=XE;
    LCD_CmdWrite(0x95);
    LCD_DataWrite(temp&0xff);
    temp=XE>>8;
    LCD_CmdWrite(0x96);
    LCD_DataWrite(temp);

    temp=YS;
    LCD_CmdWrite(0x93);
    LCD_DataWrite(temp&0xff);
    temp=YS>>8;
    LCD_CmdWrite(0x94);
    LCD_DataWrite(temp);

    temp=YE;
    LCD_CmdWrite(0x97);
    LCD_DataWrite(temp&0xff);
    temp=YE>>8;
    LCD_CmdWrite(0x98);
    LCD_DataWrite(temp);
}

////////////draw a triangle of three point 
void Draw_Triangle(uint X3,uint Y3)
{
    unsigned char temp;    
    temp=X3;
    LCD_CmdWrite(0xA9);
    LCD_DataWrite(temp&0xff);
    temp=X3>>8;
    LCD_CmdWrite(0xAA);
    LCD_DataWrite(temp);

    temp=Y3;
    LCD_CmdWrite(0xAB);
    LCD_DataWrite(temp&0xff);
    temp=Y3>>8;
    LCD_CmdWrite(0xAC);
    LCD_DataWrite(temp);
}

//////////check interrupt flag char
char Chk_INT(void)
{
    unsigned char temp;
    temp=LCD_StatusRead();
    if ((temp&0x20)==0x20)
        return 1;
    else
        return 0;
}

char Chk_INT2(void)
{
    unsigned char temp;
    LCD_CmdWrite(0x74);//INTC
    temp =LCD_DataRead();
    if ((temp&0x80)==0x80)
        return 1;
    else
        return 0;
}

  

////////////Show the picture of the flash
void Displaypicture(unsigned char picnum)
{  
   unsigned char picnumtemp;
   Write_Dir(0X06,0X00);//FLASH frequency setting
   Write_Dir(0X05,0X87);//FLASH setting 

	picnumtemp=picnum;

   Write_Dir(0XBF,0X02);//FLASH setting
   Active_Window(0,799,0,479); 
   MemoryWrite_Position(0,0);//Memory write position
   DMA_Start_address_setting(768000*(picnumtemp-1));//DMA Start address setting
   DMA_block_mode_size_setting(800,480,800);
   Write_Dir(0XBF,0X03);//FLASH setting
	Chk_DMA_Busy();
} 

//////Shear pictures
//Shear pictures number:picnum
//display position:x1,y1,x2,y2
//the upper left of the shear image coordinates :x,y
void CutPictrue(unsigned char picnum,uint x1,uint y1,uint x2,uint y2,unsigned long x,unsigned long y)
{
    unsigned long cutaddress;
    unsigned char picnumtemp;
    Write_Dir(0X06,0X00);//FLASH frequency setting   
    Write_Dir(0X05,0Xac);//FLASH setting

    picnumtemp=picnum;

    Write_Dir(0XBF,0X02);//FLASH setting
    Active_Window(x1,x2,y1,y2);
    MemoryWrite_Position(x1,y1);//Memory write position
    cutaddress=(picnumtemp-1)*768000+y*1600+x*2;
    DMA_Start_address_setting(cutaddress);
    DMA_block_mode_size_setting(x2-x1+1,y2-y1+1,800);
    Write_Dir(0XBF,0X03);//FLASH setting
    Chk_DMA_Busy();
}

//full display test
void Test(void)
{	///display red
	Text_Background_Color1(color_red);//Background color setting
	Write_Dir(0X8E,0X80);//Began to clear the screen (display window)
	NextStep();
	///display green
	Text_Background_Color1(color_green);//Background color setting
	Write_Dir(0X8E,0X80);//Began to clear the screen (display window)
	NextStep();
	///display blue
	Text_Background_Color1(color_blue);//Background color setting
	Write_Dir(0X8E,0X80);//Began to clear the screen (display window)
	NextStep();
	///display white
	Text_Background_Color1(color_white);//Background color setting
	Write_Dir(0X8E,0X80);//Began to clear the screen (display window)
	NextStep();
	///display cyan
	Text_Background_Color1(color_cyan);//Background color setting
	Write_Dir(0X8E,0X80);//Began to clear the screen (display window)
	NextStep();
	///display yellow
	Text_Background_Color1(color_yellow);//Background color setting
	Write_Dir(0X8E,0X80);//Began to clear the screen (display window)
	NextStep();
	///display purple
	Text_Background_Color1(color_purple);//Background color setting
	Write_Dir(0X8E,0X80);//Began to clear the screen (display window)
	NextStep();
	///display black
	Text_Background_Color1(color_black);//Background color setting
	Write_Dir(0X8E,0X80);//Began to clear the screen (display window)
	NextStep();
}


void drawLine3(uint x0, uint x1, uint y0, uint y1, uint color)
{
  /* Set X */
  LCD_CmdWrite(0x91);
  LCD_DataWrite(x0);
  LCD_CmdWrite(0x92);
  LCD_DataWrite(x0 >> 8);

  /* Set Y */
  LCD_CmdWrite(0x93);
  LCD_DataWrite(y0);
  LCD_CmdWrite(0x94);
  LCD_DataWrite(y0 >> 8);

  /* Set X1 */
  LCD_CmdWrite(0x95);
  LCD_DataWrite(x1);
  LCD_CmdWrite(0x96);
  LCD_DataWrite((x1) >> 8);

  /* Set Y1 */
  LCD_CmdWrite(0x97);
  LCD_DataWrite(y1);
  LCD_CmdWrite(0x98);
  LCD_DataWrite((y1) >> 8);

  /* Set Color */
  LCD_CmdWrite(0x63);
  LCD_DataWrite((color & 0xf800) >> 11);
  LCD_CmdWrite(0x64);
  LCD_DataWrite((color & 0x07e0) >> 5);
  LCD_CmdWrite(0x65);
  LCD_DataWrite((color & 0x001f));

  /* Draw! */
  LCD_CmdWrite(RA8875_DCR);
  LCD_DataWrite(0x80);

  /* Wait for the command to finish */
  waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

void writeReg(uchar reg, uchar val) 
{
  LCD_CmdWrite(reg);
  LCD_DataWrite(val);
}
/**************************************************************************/
void PWM1out(uchar p) {
  writeReg(RA8875_P1DCR, p);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void PWM2out(uchar p) {
  writeReg(RA8875_P2DCR, p);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void PWM1config(char on, uchar clock) {
  if (on) {
    writeReg(RA8875_P1CR, RA8875_P1CR_ENABLE | (clock & 0xF));
  } else {
    writeReg(RA8875_P1CR, RA8875_P1CR_DISABLE | (clock & 0xF));
  }
}

void PWM2config(char on, uchar clock) {
  if (on) {
    writeReg(RA8875_P2CR, RA8875_P2CR_ENABLE | (clock & 0xF));
  } else {
    writeReg(RA8875_P2CR, RA8875_P2CR_DISABLE | (clock & 0xF));
  }
}

/**************************************************************************/
/*
    Below Code added by AtomSoft aka Jason Lopez
*/
/**************************************************************************/

//
// Clear the window aka Clear Screen (CLS)
//
// if area is > 0 then active window is cleared
// if area is 0 then entire window is cleared
void ClearScreen(char area)
{
    char val = 0x80;

    if(area)
        val |= 0x40;

    LCD_CmdWrite(0x8E);     //Memory Clear Control Register
    LCD_DataWrite(val);    //Clear full window
}
//
// Display On or Off
//
void Display(char val)
{
    LCD_CmdWrite(0x01);
    LCD_DataWrite((val<<8));
}
//
// Layer Control
//
void Layers(char val)
{
    LCD_CmdWrite(0x20);
    LCD_DataWrite((val<<8));
}
//
// Change Font Size
//
void FontSize (char size)
{
    uchar reg ;
    size-=1;
    reg = (size<<2) | size;

    LCD_CmdWrite(0x22);
    LCD_DataWrite(reg);
}
//
// Set Fore/Back Colors
//
void SetColors(uint f_color, uint b_color)
{
    LCD_CmdWrite(0x60);//BGCR0
    LCD_DataWrite((unsigned char)(b_color>>11));

    LCD_CmdWrite(0x61);//BGCR0
    LCD_DataWrite((unsigned char)(b_color>>5));

    LCD_CmdWrite(0x62);//BGCR0
    LCD_DataWrite((unsigned char)(b_color));

    LCD_CmdWrite(0x63);//BGCR0
    LCD_DataWrite((unsigned char)(f_color>>11));

    LCD_CmdWrite(0x64);//BGCR0
    LCD_DataWrite((unsigned char)(f_color>>5));

    LCD_CmdWrite(0x65);//BGCR0
    LCD_DataWrite((unsigned char)(f_color));
}
//
// Setup Graphic Cursor ???
//
void GraphicCursor(uint x, uint y, uchar color1, uchar color2)
{
    LCD_CmdWrite(0x80);
    LCD_DataWrite((uchar)(x&0xFF));

    LCD_CmdWrite(0x81);
    LCD_DataWrite((x>>8));

    LCD_CmdWrite(0x82);
    LCD_DataWrite((uchar)(y&0xFF));

    LCD_CmdWrite(0x83);
    LCD_DataWrite((y>>8));

    LCD_CmdWrite(0x84);
    LCD_DataWrite(color1);

    LCD_CmdWrite(0x85);
    LCD_DataWrite(color2);


}
//
// Backlight Control
//
void Backlight(uchar div, uchar pwm)
{
    PWM1config(1,div);
    PWM1out(pwm);
}
//
// Draw Routines
//
// Draw Line :
// xs = POINT 1 X location
// ys = POINT 1 Y location
//
// xe = POINT 2 X location
// ye = POINT 2 Y location
// color = color of line
//
void DrawLine ( uint xs,uint ys,uint xe,uint ye, uint color)
{
    LCD_CmdWrite(0x91);
    LCD_DataWrite((uchar)xs);
    LCD_CmdWrite(0x92);
    LCD_DataWrite(xs>>8);

    LCD_CmdWrite(0x93);
    LCD_DataWrite((uchar)xe);
    LCD_CmdWrite(0x94);
    LCD_DataWrite(xe>>8);

    LCD_CmdWrite(0x95);
    LCD_DataWrite((uchar)ys);
    LCD_CmdWrite(0x96);
    LCD_DataWrite(ys>>8);

    LCD_CmdWrite(0x97);
    LCD_DataWrite((uchar)ye);
    LCD_CmdWrite(0x98);
    LCD_DataWrite(ye>>8);

    LCD_CmdWrite(0x63);
    LCD_DataWrite((color & 0xf800) >> 11);
    LCD_CmdWrite(0x64);
    LCD_DataWrite((color & 0x07e0) >> 5);
    LCD_CmdWrite(0x65);
    LCD_DataWrite((color & 0x001f));

    LCD_CmdWrite(0x90);
    LCD_DataWrite(RA8875_DCR_LINESQUTRI_START|RA8875_DCR_DRAWLINE);    // Start Line Drawing

    waitPoll(RA8875_DCR,RA8875_DCR_LINESQUTRI_STATUS);
}
//
//  Draw Square/Rectangle
//  DrawSquare :
//  x = LEFT
//  y = TOP
//  w = WIDTH
//  h = HEIGHT
//  color = COLOR of Square/Rect
//  fill = 0 = NOFILL, 1+ = FILL
//
void DrawSquare ( uint x,uint y,uint w,uint h, uint color, char fill)
{
    w+= x;
    h+= y;

    /* Set X */
    LCD_CmdWrite(0x91);
    LCD_DataWrite(x);
    LCD_CmdWrite(0x92);
    LCD_DataWrite(x >> 8);

    /* Set Y */
    LCD_CmdWrite(0x93);
    LCD_DataWrite(y);
    LCD_CmdWrite(0x94);
    LCD_DataWrite(y >> 8);

    /* Set X1 */
    LCD_CmdWrite(0x95);
    LCD_DataWrite(w);
    LCD_CmdWrite(0x96);
    LCD_DataWrite((w) >> 8);

    /* Set Y1 */
    LCD_CmdWrite(0x97);
    LCD_DataWrite(h);
    LCD_CmdWrite(0x98);
    LCD_DataWrite((h) >> 8);

    /* Set Color */
    LCD_CmdWrite(0x63);
    LCD_DataWrite((color & 0xf800) >> 11);
    LCD_CmdWrite(0x64);
    LCD_DataWrite((color & 0x07e0) >> 5);
    LCD_CmdWrite(0x65);
    LCD_DataWrite((color & 0x001f));

    /* Draw! */
    LCD_CmdWrite(RA8875_DCR);
    if (fill)
        LCD_DataWrite(0xB0);
    else
        LCD_DataWrite(0x90);

    /* Wait for the command to finish */
    waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}
void DrawCircle(uint x0, uint y0, uint r, uint color, char filled)
{
  /* Set X */
  LCD_CmdWrite(0x99);
  LCD_DataWrite(x0);
  LCD_CmdWrite(0x9a);
  LCD_DataWrite(x0 >> 8);

  /* Set Y */
  LCD_CmdWrite(0x9b);
  LCD_DataWrite(y0);
  LCD_CmdWrite(0x9c);
  LCD_DataWrite(y0 >> 8);

  /* Set Radius */
  LCD_CmdWrite(0x9d);
  LCD_DataWrite(r);

  /* Set Color */
  LCD_CmdWrite(0x63);
  LCD_DataWrite((color & 0xf800) >> 11);
  LCD_CmdWrite(0x64);
  LCD_DataWrite((color & 0x07e0) >> 5);
  LCD_CmdWrite(0x65);
  LCD_DataWrite((color & 0x001f));

  /* Draw! */
  LCD_CmdWrite(RA8875_DCR);
  if (filled)
  {
    LCD_DataWrite(RA8875_DCR_CIRCLE_START | RA8875_DCR_FILL);
  }
  else
  {
    LCD_DataWrite(RA8875_DCR_CIRCLE_START | RA8875_DCR_NOFILL);
  }

  /* Wait for the command to finish */
  waitPoll(RA8875_DCR, RA8875_DCR_CIRCLE_STATUS);
}

void OpenASI (char *filename, uint x, uint y)
{
    uint w = 0;
    uint h = 0;
    UINT br;         /* File read/write count */
    FRESULT fr;          /* FatFs return code */
    uint len;
    uint line;
    FIL fil;
    uint top, left;
    
    Chk_Busy();
    fr = f_open(&fil, filename, FA_OPEN_EXISTING | FA_READ);
    fr = f_read(&fil, IMAGE_BUFF, 0x14, &br);

    h = IMAGE_BUFF[0x10] | (IMAGE_BUFF[0x11]<<8);
    w = IMAGE_BUFF[0x12] | (IMAGE_BUFF[0x13]<<8);;

    len = ((w*h)*2);
    line = (w*2);

    f_lseek(&fil, 0x14);
    
    for(top=0;top<h;top++)
    {
        WriteCommand(0x40,0x00);    // Graphics write mode
        SetGraphicsCursorWrite(x, (y+top));
        LCD_CmdWrite(0x02);

        fr = f_read(&fil, IMAGE_BUFF, line, &br);  /* Read a chunk of source file */
        if (fr != FR_OK) break;

        Chk_Busy();
        CS_LOW(LCD);
        SPI_Write(0x00);         // Cmd: write data

        for(left=0;left<line;left+=2)
        {
            SPI_Write(IMAGE_BUFF[left]);
            SPI_Write(IMAGE_BUFF[left+1]);
        }

        CS_HIGH();
    }
    
    CS_HIGH();
    f_close(&fil);
}

void ReplaceASI (char *filename, uint x, uint y, uint w, uint h)
{
    UINT br;         /* File read/write count */
    FRESULT fr;          /* FatFs return code */

    FIL fil;
    uint top, left;

    //BLOCK
    uint readY, stride;
    uint addX;

    Chk_Busy();
    fr = f_open(&fil, filename, FA_OPEN_EXISTING | FA_READ);

    stride = w*2;
    addX = 1600;
    for(top=0;top<h;top++)
    {
        WriteCommand(0x40,0x00);    // Graphics write mode
        SetGraphicsCursorWrite(x, (y+top));
        LCD_CmdWrite(0x02);

        readY = ((top+y) * addX) + (x*2) + 0x14;

        f_lseek(&fil, readY);
        fr = f_read(&fil, IMAGE_BUFF, stride, &br);  /* Read a chunk of source file */
        if (fr != FR_OK) break;

        Chk_Busy();
        CS_LOW(LCD);
        SPI_Write(0x00);         // Cmd: write data

        for(left=0;left<stride;left+=2)
        {
            SPI_Write(IMAGE_BUFF[left]);
            SPI_Write(IMAGE_BUFF[left+1]);
        }

        CS_HIGH();
    }

    CS_HIGH();
    f_close(&fil);
}
char isImageButton (ImageButton btn)
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
