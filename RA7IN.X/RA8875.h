#ifndef _ra8875_h
#define _ra8875_h

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned long ulong;

#define RA8875_DCR                    0x90
#define RA8875_DCR_LINESQUTRI_START   0x80
#define RA8875_DCR_LINESQUTRI_STOP    0x00
#define RA8875_DCR_LINESQUTRI_STATUS  0x80
#define RA8875_DCR_CIRCLE_START       0x40
#define RA8875_DCR_CIRCLE_STATUS      0x40
#define RA8875_DCR_CIRCLE_STOP        0x00
#define RA8875_DCR_FILL               0x20
#define RA8875_DCR_NOFILL             0x00
#define RA8875_DCR_DRAWLINE           0x00
#define RA8875_DCR_DRAWTRIANGLE       0x01
#define RA8875_DCR_DRAWSQUARE         0x10

#define RA8875_P1CR             0x8A
#define RA8875_P1CR_ENABLE      0x80
#define RA8875_P1CR_DISABLE     0x00
#define RA8875_P1CR_CLKOUT      0x10
#define RA8875_P1CR_PWMOUT      0x00

#define RA8875_P1DCR            0x8B

#define RA8875_P2CR             0x8C
#define RA8875_P2CR_ENABLE      0x80
#define RA8875_P2CR_DISABLE     0x00
#define RA8875_P2CR_CLKOUT      0x10
#define RA8875_P2CR_PWMOUT      0x00

#define RA8875_P2DCR            0x8D

#define RA8875_PWM_CLK_DIV1     0x00
#define RA8875_PWM_CLK_DIV2     0x01
#define RA8875_PWM_CLK_DIV4     0x02
#define RA8875_PWM_CLK_DIV8     0x03
#define RA8875_PWM_CLK_DIV16    0x04
#define RA8875_PWM_CLK_DIV32    0x05
#define RA8875_PWM_CLK_DIV64    0x06
#define RA8875_PWM_CLK_DIV128   0x07
#define RA8875_PWM_CLK_DIV256   0x08
#define RA8875_PWM_CLK_DIV512   0x09
#define RA8875_PWM_CLK_DIV1024  0x0A
#define RA8875_PWM_CLK_DIV2048  0x0B
#define RA8875_PWM_CLK_DIV4096  0x0C
#define RA8875_PWM_CLK_DIV8192  0x0D
#define RA8875_PWM_CLK_DIV16384 0x0E
#define RA8875_PWM_CLK_DIV32768 0x0F

#define color_brown   0x40c0
#define color_black   0x0000
#define color_white   0xffff
#define color_red     0xf800
#define color_green   0x07e0
#define color_blue    0x001f
#define color_yellow  color_red|color_green
#define color_cyan    color_green|color_blue
#define color_purple  color_red|color_blue


#define RESET FUB_P7
#define RESET_LOW()        PORTClearBits(RESET)
#define RESET_HIGH()       PORTSetBits(RESET)



#define SPI_Write(var) SpiTransfer(var)
#define SPI_Read() SpiTransfer(0xFF)

void Delay1ms(uint i);
void Delay10ms(uint i);
void Delay100ms(uint i);
void NextStep(void);

void LCD_CmdWrite(unsigned char cmd);
void LCD_DataWrite(unsigned char Data);
unsigned char LCD_DataRead(void);

void Write_Dir(unsigned char Cmd,unsigned char Data);
unsigned char LCD_StatusRead(void);

void LCD_Reset(void);
void Chk_Busy(void);
void Chk_BTE_Busy(void);
void Chk_DMA_Busy(void);
void PLL_ini(void);
void Active_Window(uint XL,uint XR ,uint YT ,uint YB);
void LCD_Initial(void);
void Text_Background_Color1(uint b_color);
void Text_Background_Color(unsigned char setR, uchar setG, uchar setB);
void Text_Foreground_Color1(uint b_color);
void Text_Foreground_Color(unsigned char setR,uchar setG,uchar setB);
void BTE_Size(uint width, uint height);
void BTE_Source(uint SX,uint DX ,uint SY ,uint DY);
void MemoryWrite_Position(uint X,uint Y);
void FontWrite_Position(uint X,uint Y);
void String(unsigned char *str);
void Scroll_Window(uint XL,uint XR ,uint YT ,uint YB);
void Scroll(uint X,uint Y);
void DMA_block_mode_size_setting(uint BWR,uint BHR,uint SPWR);
void DMA_Start_address_setting(ulong set_address);
void  Draw_Circle(uint X,uint Y,uint R);
void  Draw_Ellipse(uint X,uint Y,uint R1,uint R2);
void Draw_Line(uint XS,uint XE ,uint YS,uint YE);
void Draw_Triangle(uint X3,uint Y3);
char Touch_Status(void);
char Chk_INT(void);
char Chk_INT2(void);
unsigned char ADC_X(void);
unsigned char ADC_Y(void);
unsigned char ADC_XY(void);
void TP(void);
void Displaypicture(unsigned char picnum);
void CutPictrue(unsigned char picnum,uint x1,uint y1,uint x2,uint y2,unsigned long x,unsigned long y);
void Test(void);

void MakeCircle(uint x0, uint y0, uint r, uint color, char filled);
char waitPoll(uchar regname, uchar waitflag);
void fillRect(void);
void drawRect(uint x, uint y, uint w, uint h, uint color, char filled);
void drawLine(uint x0, uint x1, uint y0, uint y1, uint color);

#define FILL 1
#define NOFILL 0

typedef struct _Button_ {
    UINT top;
    UINT left;
    UINT width;
    UINT height;
} Button;


typedef struct _Fader_ {
    UINT    bar_value;
    UINT    bar_max;

    UINT    sel_width;
    UINT    sel_height;
    UINT    sel_color;

    UINT    bar_x;
    UINT    bar_y;
    UINT    bar_width;
    UINT    bar_height;
    UINT    bar_color;
    
} Fader;

typedef struct _TouchButton_ {
    UINT top;
    UINT left;
    UINT width;
    UINT height;
    UINT fore_color;
    UINT back_color;
    char text[13];
} TouchButton;

typedef struct _ImageButton_ {
    UINT top;
    UINT left;
    UINT width;
    UINT height;
    char up[13];
    char down[13];
} ImageButton;

//BELOW ARE ATOMSOFT TECH MODS
void PWM2config(char on, uchar clock);
void PWM1config(char on, uchar clock);
void PWM2out(uchar p);
void PWM1out(uchar p);

void ClearScreen(char area);
void Display(char val);
void Layers(char val);
void FontSize (char size);
void SetColors(uint f_color, uint b_color);
void GraphicCursor(uint x, uint y, uchar color1, uchar color2);
void Backlight(uchar div, uchar pwm);
void DrawLine (uint xs,uint ys,uint xe,uint ye, uint color);
void DrawSquare ( uint x,uint y,uint w,uint h, uint color, char fill);
void DrawCircle(uint x0, uint y0, uint r, uint color, char filled);
void LCD_ReadBuff(char *buff, uint len);
void SetGraphicsCursorRead(uint x, uint y);
void SetGraphicsCursorWrite(uint x, uint y);
void WriteCommandW(uchar command, uint data);
void WriteCommand(unsigned char command, unsigned int data);
void putPixelStream(uchar * p, uint count, uint x, uint y);
void OpenASI (char *filename, uint x, uint y);
char isImageButton (ImageButton btn);

#endif