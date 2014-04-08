/***************************************************

Bitmap file format to N6610 LCD format
yus    - http://projectproto.blogspot.com/
May 2010

*****************************************************/
#include "lcd_bmp.h"


/***** private variables *****/
//unsigned char imgBuff[4608] = {0};            // for pf_read of bitmap data
WORD rb;
BITMAPINFOHEADER MyBMP;

uchar IMAGEBUFF2[IMAGEBUFF_LEN];

void draw_bitmap(char *filename,unsigned char x,unsigned char y)
{
    unsigned char DW_buff[4] = {0};
    unsigned char W_buff[2] = {0};
    UINT br;
    FRESULT res;
    FIL fil;

    res = f_open(&fil, filename, FA_OPEN_EXISTING | FA_READ);
    if(res) return;

    f_read(&fil, W_buff, 2, &br);            // 0x00 - Magic Word
    MyBMP.magic = LD_WORD(W_buff);

    if(MyBMP.magic==0x4d42)        //Magic Word "BM" aka 0x4d42 for BMPs
    {
        f_lseek(&fil, 0x0A);                      // 0x0A - Offset of RAW Pixel Data

        f_read(&fil, DW_buff, 4, &rb);
        MyBMP.bmp_offset = LD_DWORD(DW_buff);

        f_read(&fil, DW_buff, 4, &rb);        // 0x0E (Skip DWORD)

        f_read(&fil, DW_buff, 4, &rb);        // 0x12 bitmap width in pixels
        MyBMP.width = LD_DWORD(DW_buff);

        f_read(&fil, DW_buff, 4, &rb);        // 0x16 bitmap height in pixels
        MyBMP.height = LD_DWORD(DW_buff);

        if( (MyBMP.width>128) || (MyBMP.height>160) )
            return;
        
        f_read(&fil, W_buff, 2, &rb);        // 0x1A (Skip WORD of data)
        f_read(&fil, W_buff, 2, &rb);        // 0x1C    (Bits Per Pixel)

        MyBMP.bitspp = LD_WORD(W_buff);
        if( (MyBMP.bitspp!=24) )
            return;

        f_read(&fil, DW_buff, 4, &rb);        // 0x1E (compression method)
        MyBMP.compress_type = LD_DWORD(DW_buff);

        if( MyBMP.compress_type != 0 )
            return;

        f_read(&fil, DW_buff, 4, &rb);        // 0x22 (raw bitmap data size)
        MyBMP.bmp_bytesz = LD_DWORD(DW_buff);

        if(MyBMP.bitspp == 24)
        {
            memset(IMAGEBUFF2,0,IMAGEBUFF_LEN);   //Clear Buffer
            atomBMP(&fil,x,y);          //Start Draw Routine
        }
    }
}


void atomBMP(FIL *fp, unsigned char x,unsigned char y)
{
    UINT pixel, row, column, line;
    UINT buffL;
    DWORD count;
    DWORD total;
    DWORD MyOffset;
    UINT len;
    UINT loopC;
    uint LoopLen = IMAGEBUFF_LEN;
    char pad =  0;

    total = MyBMP.width*3;    // max is 384 (128 width * 3 RGB bytes)

    if( total%4 != 0)
        total = total + (4 - (total%4));  // should be multiple of DWORD size (4 bytes)

    
    pad = (4 - ((MyBMP.width*3)%4)); //JUST DID THIS

    len = (total * MyBMP.height);     // Should be = to bmp_bytesz-2

    LoopLen = total;
    
    loopC = MyBMP.height;

    if(total%LoopLen)
        loopC++;

     //Jump to the end of the file minus the buffer len
    MyOffset = (MyBMP.bmp_offset+len)-LoopLen; //(File Start + File Len)- Buffer = start POSITION
    f_lseek(fp, MyOffset);


    for(buffL = 0; buffL < loopC; buffL++)
    {
        f_read(fp, IMAGEBUFF2, LoopLen, &rb);         //Read MaxBuff amount into our buffer

        WriteCommand(0x40,0x00);    // Graphics write mode
        SetGraphicsCursorWrite(x, y++);
        LCD_CmdWrite(0x02);

        Chk_Busy();
        CS_LOW(LCD);

        SPI_Write(0x00);         // Cmd: write data

        //count = LoopLen - (pad);

        for(column=0; column<(MyBMP.width*3); column+=3)           //Loop 1 width
        {
            pixel = RGB16(IMAGEBUFF2[(count)], IMAGEBUFF2[(count+1)], IMAGEBUFF2[(count+2)]); //Setup our pixel
            SPI_Write(pixel&0xFF);
            SPI_Write(pixel>>8);
            count-=3;
        }

        CS_HIGH();

        MyOffset = (MyBMP.bmp_offset+len)-(LoopLen*(buffL+1));  //New offset is MaxBuff*buff counts size minus the end of file
        f_lseek(fp, MyOffset); //Goto above offset
    }

}
void draw_bmp(FIL *fp, unsigned char x,unsigned char y)
{
    UINT pixel, row, column, line;
    DWORD count;
    DWORD total;
    DWORD MyOffset;
    UINT len;
    char pad =  0;
    char tmp = 0;

    total = MyBMP.width*3;    // max is 384 (128 width * 3 RGB bytes)
    
    if( total%4 != 0)
        total = total + (4 - (total%4));  // should be multiple of DWORD size (4 bytes)

    pad = (4 - ((MyBMP.width*3)%4)); //JUST DID THIS

    len = (total * MyBMP.height);     // Should be = to bmp_bytesz-2


     //Jump to the end of the file minus the buffer len
    MyOffset = (MyBMP.bmp_offset+len)-IMAGEBUFF_LEN; //(File Start + File Len)- Buffer = start POSITION
    f_lseek(fp, MyOffset);


    for(row=0; row<(len/IMAGEBUFF_LEN); row++){   //Loop MaxBuff/Len of file which is the ((h*(w+pad))*3)
        f_read(fp, IMAGEBUFF2, IMAGEBUFF_LEN, &rb);         //Read MaxBuff amount into our buffer
        for(line=1; line<((IMAGEBUFF_LEN/total)+1); line++){    //Loop for width amounts in buff
            WriteCommand(0x40,0x00);    // Graphics write mode
            SetGraphicsCursorWrite(x, y++);
            LCD_CmdWrite(0x02);

            Chk_Busy();
            CS_LOW(LCD);

            SPI_Write(0x00);         // Cmd: write data

            for(column=0; column<(MyBMP.width*3); column+=3)           //Loop 1 width
            {
                count =(IMAGEBUFF_LEN-(total*line))+(column);
                pixel = RGB16(IMAGEBUFF2[(count+2)], IMAGEBUFF2[(count+1)], IMAGEBUFF2[(count)]); //Setup our pixel
                SPI_Write(pixel>>8);
                SPI_Write(pixel&0xFF);

            }
            CS_HIGH();
        }
        MyOffset = ((MyBMP.bmp_offset+len)-(IMAGEBUFF_LEN*(row+2)));  //New offset is MaxBuff*buff counts size minus the end of file
        f_lseek(fp, MyOffset); //Goto above offset
    }


}