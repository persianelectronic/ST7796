/*ST7796-ILI9488 Driver library for STM32*/
/* www.pepoya.ir*/
/* SAEED KOLIVAND*/

/* Includes ------------------------------------------------------------------*/
#include "st7796.h"
#include "spi.h"
#include "gpio.h"
#include "main.h"
#include "stm32f4xx.h"
#include "5x5_font.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h> 

/* Global Variables ------------------------------------------------------------------*/
volatile uint16_t LCD_HEIGHT = ST7796_SCREEN_HEIGHT;
volatile uint16_t LCD_WIDTH	 = ST7796_SCREEN_WIDTH;

/* Initialize SPI */
void ST7796_SPI_Init(void)
{																						
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);	//CS OFF
}

/*Send data (char) to LCD*/
void ST7796_SPI_Send(unsigned char SPI_Data)
{
HAL_SPI_Transmit(LEOPARD_BOARD, &SPI_Data, 1, 1);
}

/* Send command (char) to LCD */
void ST7796_Write_Command(uint8_t Command)
{
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);	
ST7796_SPI_Send(Command);
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);		
}

/* Send Data (char) to LCD */
void ST7796_Write_Data(uint8_t Data)
{
HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);	
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
ST7796_SPI_Send(Data);	
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

/* Set Address - Location block - to draw into */
void ST7796_Set_Address(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2)
{
	ST7796_Write_Command(0x2A);
	ST7796_Write_Data(X1>>8);
	ST7796_Write_Data(X1);
	ST7796_Write_Data(X2>>8);
	ST7796_Write_Data(X2);

	ST7796_Write_Command(0x2B);
	ST7796_Write_Data(Y1>>8);
	ST7796_Write_Data(Y1);
	ST7796_Write_Data(Y2>>8);
	ST7796_Write_Data(Y2);

	ST7796_Write_Command(0x2C);
}

/* Set Address - Location block - to draw into DMA */
void LCD_Send_Data_DMA(uint16_t x, uint16_t y, uint16_t  x_end, uint16_t  y_end, uint8_t  *p)
{ 
	  ST7796_Set_Address(x, y, x_end, y_end);
		HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
  	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);	
    if ( HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)p, (y_end - y + 1) * (x_end - x + 1) * 2) != HAL_OK)
    {
      //  while(1);	/*Halt on error*/
    }	
}

/*HARDWARE RESET*/
void ST7796_Reset(void)
{
HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
HAL_Delay(200);
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
HAL_Delay(200);
HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);	
}

/*Ser rotation of the screen - changes x0 and y0*/
void ST7796_Set_Rotation(uint8_t Rotation) 
{
uint8_t screen_rotation = Rotation;
ST7796_Write_Command(0x36);
HAL_Delay(1);
	
switch(screen_rotation) 
	{
		case SCREEN_VERTICAL_1:
			ST7796_Write_Data(0x40|0x08);
			LCD_WIDTH = 320;
			LCD_HEIGHT = 480;
			break;
		case SCREEN_HORIZONTAL_1:
			ST7796_Write_Data(0x20|0x08);
			LCD_WIDTH  = 480;
			LCD_HEIGHT = 320;
			break;
		case SCREEN_VERTICAL_2:
			ST7796_Write_Data(0x80|0x08);
			LCD_WIDTH  = 320;
			LCD_HEIGHT = 480;
			break;
		case SCREEN_HORIZONTAL_2:
			ST7796_Write_Data(0x40|0x80|0x20|0x08);
			LCD_WIDTH  = 480;
			LCD_HEIGHT = 320;
			break;
		default:
			//EXIT IF SCREEN ROTATION NOT VALID!
			break;
	}
}

/*Enable LCD display*/
void ST7796_Enable(void)
{
HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
HAL_Delay(100);	
HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET);	
HAL_Delay(100);	
HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);	
}

/*Initialize LCD display*/
void ST7796_Init(void)
{

ST7796_Enable();
ST7796_SPI_Init();
ST7796_Reset();

//SOFTWARE RESET
ST7796_Write_Command(0x01);
HAL_Delay(1000);
	
//POWER CONTROL A
ST7796_Write_Command(0xCB);
ST7796_Write_Data(0x39);
ST7796_Write_Data(0x2C);
ST7796_Write_Data(0x00);
ST7796_Write_Data(0x34);
ST7796_Write_Data(0x02);

//POWER CONTROL B
ST7796_Write_Command(0xCF);
ST7796_Write_Data(0x00);
ST7796_Write_Data(0xC1);
ST7796_Write_Data(0x30);

//DRIVER TIMING CONTROL A
ST7796_Write_Command(0xE8);
ST7796_Write_Data(0x85);
ST7796_Write_Data(0x00);
ST7796_Write_Data(0x78);

//DRIVER TIMING CONTROL B
ST7796_Write_Command(0xEA);
ST7796_Write_Data(0x00);
ST7796_Write_Data(0x00);

//POWER ON SEQUENCE CONTROL
ST7796_Write_Command(0xED);
ST7796_Write_Data(0x64);
ST7796_Write_Data(0x03);
ST7796_Write_Data(0x12);
ST7796_Write_Data(0x81);

//PUMP RATIO CONTROL
ST7796_Write_Command(0xF7);
ST7796_Write_Data(0x20);

//POWER CONTROL,VRH[5:0]
ST7796_Write_Command(0xC0);
ST7796_Write_Data(0x23);

//POWER CONTROL,SAP[2:0];BT[3:0]
ST7796_Write_Command(0xC1);
ST7796_Write_Data(0x10);

//VCM CONTROL
ST7796_Write_Command(0xC5);
ST7796_Write_Data(0x3E);
ST7796_Write_Data(0x28);

//VCM CONTROL 2
ST7796_Write_Command(0xC7);
ST7796_Write_Data(0x86);

//MEMORY ACCESS CONTROL
ST7796_Write_Command(0x36);
ST7796_Write_Data(0x48);

//PIXEL FORMAT
ST7796_Write_Command(0x3A);
ST7796_Write_Data(0x55);

//FRAME RATIO CONTROL, STANDARD RGB COLOR
ST7796_Write_Command(0xB1);
ST7796_Write_Data(0x00);
ST7796_Write_Data(0x18);

//DISPLAY FUNCTION CONTROL
ST7796_Write_Command(0xB6);
ST7796_Write_Data(0x08);
ST7796_Write_Data(0x82);
ST7796_Write_Data(0x27);

//3GAMMA FUNCTION DISABLE
ST7796_Write_Command(0xF2);
ST7796_Write_Data(0x00);

//GAMMA CURVE SELECTED
ST7796_Write_Command(0x26);
ST7796_Write_Data(0x01);

//POSITIVE GAMMA CORRECTION
ST7796_Write_Command(0xE0);
ST7796_Write_Data(0x0F);
ST7796_Write_Data(0x31);
ST7796_Write_Data(0x2B);
ST7796_Write_Data(0x0C);
ST7796_Write_Data(0x0E);
ST7796_Write_Data(0x08);
ST7796_Write_Data(0x4E);
ST7796_Write_Data(0xF1);
ST7796_Write_Data(0x37);
ST7796_Write_Data(0x07);
ST7796_Write_Data(0x10);
ST7796_Write_Data(0x03);
ST7796_Write_Data(0x0E);
ST7796_Write_Data(0x09);
ST7796_Write_Data(0x00);

//NEGATIVE GAMMA CORRECTION
ST7796_Write_Command(0xE1);
ST7796_Write_Data(0x00);
ST7796_Write_Data(0x0E);
ST7796_Write_Data(0x14);
ST7796_Write_Data(0x03);
ST7796_Write_Data(0x11);
ST7796_Write_Data(0x07);
ST7796_Write_Data(0x31);
ST7796_Write_Data(0xC1);
ST7796_Write_Data(0x48);
ST7796_Write_Data(0x08);
ST7796_Write_Data(0x0F);
ST7796_Write_Data(0x0C);
ST7796_Write_Data(0x31);
ST7796_Write_Data(0x36);
ST7796_Write_Data(0x0F);

//EXIT SLEEP
ST7796_Write_Command(0x11);
HAL_Delay(120);

//TURN ON DISPLAY
ST7796_Write_Command(0x29);

//STARTING ROTATION
ST7796_Set_Rotation(SCREEN_VERTICAL_1);
}

//INTERNAL FUNCTION OF LIBRARY, USAGE NOT RECOMENDED, USE Draw_Pixel INSTEAD
/*Sends single pixel colour information to LCD*/
void ST7796_Draw_Colour(uint16_t Colour)
{
//SENDS COLOUR
unsigned char TempBuffer[2] = {Colour>>8, Colour};	
HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);	
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
HAL_SPI_Transmit(LEOPARD_BOARD, TempBuffer, 2, 1);	
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

//INTERNAL FUNCTION OF LIBRARY
/*Sends block colour information to LCD*/
void ST7796_Draw_Colour_Burst(uint16_t Colour, uint32_t Size)
{
//SENDS COLOUR
uint32_t Buffer_Size = 0;
if((Size*2) < BURST_MAX_SIZE)
{
	Buffer_Size = Size;
}
else
{
	Buffer_Size = BURST_MAX_SIZE;
}
	
HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);	
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);

unsigned char chifted = 	Colour>>8;;
unsigned char burst_buffer[Buffer_Size];
for(uint32_t j = 0; j < Buffer_Size; j+=2)
	{
		burst_buffer[j] = 	chifted;
		burst_buffer[j+1] = Colour;
	}

uint32_t Sending_Size = Size*2;
uint32_t Sending_in_Block = Sending_Size/Buffer_Size;
uint32_t Remainder_from_block = Sending_Size%Buffer_Size;

if(Sending_in_Block != 0)
{
	for(uint32_t j = 0; j < (Sending_in_Block); j++)
		{
		HAL_SPI_Transmit(LEOPARD_BOARD, (unsigned char *)burst_buffer, Buffer_Size, 10);	
		}
}

//REMAINDER!
HAL_SPI_Transmit(LEOPARD_BOARD, (unsigned char *)burst_buffer, Remainder_from_block, 10);	
	
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

//FILL THE ENTIRE SCREEN WITH SELECTED COLOUR (either #define-d ones or custom 16bit)
/*Sets address (entire screen) and Sends Height*Width ammount of colour information to LCD*/
void ST7796_Fill_Screen(uint16_t Colour)
{
ST7796_Set_Address(0,0,LCD_WIDTH,LCD_HEIGHT);	
ST7796_Draw_Colour_Burst(Colour, LCD_WIDTH*LCD_HEIGHT);	
}

//DRAW PIXEL AT XY POSITION WITH SELECTED COLOUR
//
//Location is dependant on screen orientation. x0 and y0 locations change with orientations.
//Using pixels to draw big simple structures is not recommended as it is really slow
//Try using either rectangles or lines if possible
//
void ILI9341_Draw_Pixel(uint16_t X,uint16_t Y,uint16_t Colour) 
{
if((X >=LCD_WIDTH) || (Y >=LCD_HEIGHT)) return;	//OUT OF BOUNDS!
	
//ADDRESS
HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);	
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
ST7796_SPI_Send(0x2A);
HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);	
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);		

//XDATA
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);	
unsigned char Temp_Buffer[4] = {X>>8,X, (X+1)>>8, (X+1)};
HAL_SPI_Transmit(LEOPARD_BOARD, Temp_Buffer, 4, 1);
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);

//ADDRESS
HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);	
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);	
ST7796_SPI_Send(0x2B);
HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);			
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);			

//YDATA
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
unsigned char Temp_Buffer1[4] = {Y>>8,Y, (Y+1)>>8, (Y+1)};
HAL_SPI_Transmit(LEOPARD_BOARD, Temp_Buffer1, 4, 1);
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);

//ADDRESS	
HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);	
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);	
ST7796_SPI_Send(0x2C);
HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);			
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);			

//COLOR	
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
unsigned char Temp_Buffer2[2] = {Colour>>8, Colour};
HAL_SPI_Transmit(LEOPARD_BOARD, Temp_Buffer2, 2, 1);
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
	
}


//DRAW RECTANGLE OF SET SIZE AND HEIGTH AT X and Y POSITION WITH CUSTOM COLOUR
//
//Rectangle is hollow. X and Y positions mark the upper left corner of rectangle
//As with all other draw calls x0 and y0 locations dependant on screen orientation
//

void ST7796_Draw_Rectangle(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t Colour)
{
if((X >=LCD_WIDTH) || (Y >=LCD_HEIGHT)) return;
if((X+Width-1)>=LCD_WIDTH)
	{
		Width=LCD_WIDTH-X;
	}
if((Y+Height-1)>=LCD_HEIGHT)
	{
		Height=LCD_HEIGHT-Y;
	}
ST7796_Set_Address(X, Y, X+Width-1, Y+Height-1);
ST7796_Draw_Colour_Burst(Colour, Height*Width);
}

//DRAW LINE FROM X,Y LOCATION to X+Width,Y LOCATION
void ST7796_Draw_Horizontal_Line(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Colour)
{
if((X >=LCD_WIDTH) || (Y >=LCD_HEIGHT)) return;
if((X+Width-1)>=LCD_WIDTH)
	{
		Width=LCD_WIDTH-X;
	}
ST7796_Set_Address(X, Y, X+Width-1, Y);
ST7796_Draw_Colour_Burst(Colour, Width);
}

//DRAW LINE FROM X,Y LOCATION to X,Y+Height LOCATION
void ST7796_Draw_Vertical_Line(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Colour)
{
if((X >=LCD_WIDTH) || (Y >=LCD_HEIGHT)) return;
if((Y+Height-1)>=LCD_HEIGHT)
	{
		Height=LCD_HEIGHT-Y;
	}
ST7796_Set_Address(X, Y, X, Y+Height-1);
ST7796_Draw_Colour_Burst(Colour, Height);
}


/*Draws a character (fonts imported from fonts.h) at X,Y location with specified font colour, size and Background colour*/
/*See fonts.h implementation of font on what is required for changing to a different font when switching fonts libraries*/
void ST7796_Draw_Char(char Character, uint8_t X, uint8_t Y, uint16_t Colour, uint16_t Size, uint16_t Background_Colour) 
{
		uint8_t 	function_char;
    uint8_t 	i,j;
		
		function_char = Character;
		
    if (function_char < ' ') {
        Character = 0;
    } else {
        function_char -= 32;
		}
   	
		char temp[CHAR_WIDTH];
		for(uint8_t k = 0; k<CHAR_WIDTH; k++)
		{
		temp[k] = font[function_char][k];
		}
		
    // Draw pixels
		ST7796_Draw_Rectangle(X, Y, CHAR_WIDTH*Size, CHAR_HEIGHT*Size, Background_Colour);
    for (j=0; j<CHAR_WIDTH; j++) {
        for (i=0; i<CHAR_HEIGHT; i++) {
            if (temp[j] & (1<<i)) {			
							if(Size == 1)
							{
              ILI9341_Draw_Pixel(X+j, Y+i, Colour);
							}
							else
							{
							ST7796_Draw_Rectangle(X+(j*Size), Y+(i*Size), Size, Size, Colour);
							}
            }						
        }
    }
}

/*Draws an array of characters (fonts imported from fonts.h) at X,Y location with specified font colour, size and Background colour*/
/*See fonts.h implementation of font on what is required for changing to a different font when switching fonts libraries*/
void ST7796_Draw_Text(const char* Text, uint8_t X, uint8_t Y, uint16_t Colour, uint16_t Size, uint16_t Background_Colour)
{
    while (*Text) {
        ST7796_Draw_Char(*Text++, X, Y, Colour, Size, Background_Colour);
        X += CHAR_WIDTH*Size;
    }
}

/*Draws a full screen picture from flash. Image converted from RGB .jpeg/other to C array using online converter*/
//USING CONVERTER: http://www.digole.com/tools/PicturetoC_Hex_converter.php
//65K colour (2Bytes / Pixel)
void ILI9341_Draw_Image(const char* Image_Array, uint8_t Orientation)
{
	if(Orientation == SCREEN_HORIZONTAL_1)
	{
		ST7796_Set_Rotation(SCREEN_HORIZONTAL_1);
		ST7796_Set_Address(0,0,ST7796_SCREEN_WIDTH,ST7796_SCREEN_HEIGHT);
			
		HAL_GPIO_WritePin(GPIOE, LCD_DC_Pin, GPIO_PIN_SET);	
		HAL_GPIO_WritePin(GPIOE, LCD_CS_Pin, GPIO_PIN_RESET);
		
		unsigned char Temp_small_buffer[BURST_MAX_SIZE];
		uint32_t counter = 0;
		for(uint32_t i = 0; i < ST7796_SCREEN_WIDTH*ST7796_SCREEN_HEIGHT*2/BURST_MAX_SIZE; i++)
		{			
				for(uint32_t k = 0; k< BURST_MAX_SIZE; k++)
				{
					Temp_small_buffer[k]	= Image_Array[counter+k];		
				}						
				HAL_SPI_Transmit(&hspi1, (unsigned char*)Temp_small_buffer, BURST_MAX_SIZE, 10);	
				counter += BURST_MAX_SIZE;			
		}
		HAL_GPIO_WritePin(GPIOE, LCD_CS_Pin, GPIO_PIN_SET);
	}
	else if(Orientation == SCREEN_HORIZONTAL_2)
	{
		ST7796_Set_Rotation(SCREEN_HORIZONTAL_2);
		ST7796_Set_Address(0,0,ST7796_SCREEN_WIDTH,ST7796_SCREEN_HEIGHT);
			
		HAL_GPIO_WritePin(GPIOE, LCD_DC_Pin, GPIO_PIN_SET);	
		HAL_GPIO_WritePin(GPIOE, LCD_CS_Pin, GPIO_PIN_RESET);
		
		unsigned char Temp_small_buffer[BURST_MAX_SIZE];
		uint32_t counter = 0;
		for(uint32_t i = 0; i < ST7796_SCREEN_WIDTH*ST7796_SCREEN_HEIGHT*2/BURST_MAX_SIZE; i++)
		{			
				for(uint32_t k = 0; k< BURST_MAX_SIZE; k++)
				{
					Temp_small_buffer[k]	= Image_Array[counter+k];		
				}						
				HAL_SPI_Transmit(&hspi1, (unsigned char*)Temp_small_buffer, BURST_MAX_SIZE, 10);	
				counter += BURST_MAX_SIZE;			
		}
		HAL_GPIO_WritePin(GPIOE, LCD_CS_Pin, GPIO_PIN_SET);
	}
	else if(Orientation == SCREEN_VERTICAL_2)
	{
		ST7796_Set_Rotation(SCREEN_VERTICAL_2);
		ST7796_Set_Address(0,0,ST7796_SCREEN_HEIGHT,ST7796_SCREEN_WIDTH);
			
		HAL_GPIO_WritePin(GPIOE, LCD_DC_Pin, GPIO_PIN_SET);	
		HAL_GPIO_WritePin(GPIOE, LCD_CS_Pin, GPIO_PIN_RESET);
		
		unsigned char Temp_small_buffer[BURST_MAX_SIZE];
		uint32_t counter = 0;
		for(uint32_t i = 0; i < ST7796_SCREEN_WIDTH*ST7796_SCREEN_HEIGHT*2/BURST_MAX_SIZE; i++)
		{			
				for(uint32_t k = 0; k< BURST_MAX_SIZE; k++)
				{
					Temp_small_buffer[k]	= Image_Array[counter+k];		
				}						
				HAL_SPI_Transmit(&hspi1, (unsigned char*)Temp_small_buffer, BURST_MAX_SIZE, 10);	
				counter += BURST_MAX_SIZE;			
		}
		HAL_GPIO_WritePin(GPIOE, LCD_CS_Pin, GPIO_PIN_SET);
	}
	else if(Orientation == SCREEN_VERTICAL_1)
	{
		ST7796_Set_Rotation(SCREEN_VERTICAL_1);
		ST7796_Set_Address(0,0,ST7796_SCREEN_HEIGHT,ST7796_SCREEN_WIDTH);
			
		HAL_GPIO_WritePin(GPIOE, LCD_DC_Pin, GPIO_PIN_SET);	
		HAL_GPIO_WritePin(GPIOE, LCD_CS_Pin, GPIO_PIN_RESET);
		
		unsigned char Temp_small_buffer[BURST_MAX_SIZE];
		uint32_t counter = 0;
		for(uint32_t i = 0; i < ST7796_SCREEN_WIDTH*ST7796_SCREEN_HEIGHT*2/BURST_MAX_SIZE; i++)
		{			
				for(uint32_t k = 0; k< BURST_MAX_SIZE; k++)
				{
					Temp_small_buffer[k]	= Image_Array[counter+k];		
				}						
				HAL_SPI_Transmit(&hspi1, (unsigned char*)Temp_small_buffer, BURST_MAX_SIZE, 10);	
				counter += BURST_MAX_SIZE;			
		}
		HAL_GPIO_WritePin(GPIOE, LCD_CS_Pin, GPIO_PIN_SET);
	}
}




