/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "comp.h"
#include "crc.h"
#include "dac.h"
#include "dma.h"
#include "lptim.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "app_touchgfx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "font_ASCII_32x32.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint16_t adc_volt[2];
float volt=0;
float volt2=0;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
#define AREF 3.0
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* ==================== 引脚宏定义 (用户按需修改) ==================== */
#define LCD_CS_PORT     GPIOA
#define LCD_CS_PIN      GPIO_PIN_15

#define LCD_RST_PORT    GPIOA
#define LCD_RST_PIN     GPIO_PIN_11

#define LCD_DC_PORT     GPIOA
#define LCD_DC_PIN      GPIO_PIN_12

/* 软件SPI引脚 (使用任意GPIO，这里用PA0=SCK, PA1=MOSI) */
#define LCD_SCK_PORT    GPIOB
#define LCD_SCK_PIN     GPIO_PIN_3

#define LCD_MOSI_PORT   GPIOA
#define LCD_MOSI_PIN    GPIO_PIN_6

/* ==================== 基本操作宏 ==================== */
#define CS_LOW()    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET)
#define CS_HIGH()   HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET)

#define RST_LOW()   HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET)
#define RST_HIGH()  HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET)

#define DC_LOW()    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET)
#define DC_HIGH()   HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET)

#define SCK_LOW()   HAL_GPIO_WritePin(LCD_SCK_PORT, LCD_SCK_PIN, GPIO_PIN_RESET)
#define SCK_HIGH()  HAL_GPIO_WritePin(LCD_SCK_PORT, LCD_SCK_PIN, GPIO_PIN_SET)

#define MOSI_LOW()  HAL_GPIO_WritePin(LCD_MOSI_PORT, LCD_MOSI_PIN, GPIO_PIN_RESET)
#define MOSI_HIGH() HAL_GPIO_WritePin(LCD_MOSI_PORT, LCD_MOSI_PIN, GPIO_PIN_SET)

/* ==================== 软件SPI发送一个字节 ==================== */
//static void SPI_WriteByte(uint8_t data) {
//    for(int i = 7; i >= 0; i--) {
//        SCK_LOW();          // 时钟先拉低 (CPOL=0)
//        SPI_DELAY();
//        
//        if(data & (1 << i))
//            MOSI_HIGH();
//        else
//            MOSI_LOW();
//        
//        SPI_DELAY();
//        SCK_HIGH();         // 数据在上升沿采样 (CPHA=0)
//        SPI_DELAY();
//    }
//    SCK_LOW();  // 恢复
//}
static inline void SPI_WriteByte(uint8_t data) {
    // 使用HAL库的SPI发送函数
    // HAL_SPI_Transmit(SPI句柄, 数据缓冲区, 数据长度, 超时时间)
    HAL_SPI_Transmit_DMA(&hspi1, &data,1);
}
/* ==================== 命令/数据写入 ==================== */
static inline void WriteCmd(uint8_t cmd) {
    CS_LOW();
    DC_LOW();       // DC=0 表示命令
    SPI_WriteByte(cmd);
    CS_HIGH();
}

static inline void WriteData(uint8_t data) {
    CS_LOW();
    DC_HIGH();      // DC=1 表示数据
    SPI_WriteByte(data);
    CS_HIGH();
}

static inline void WriteData16(uint16_t data) {
    CS_LOW();
    DC_HIGH();
    SPI_WriteByte(data >> 8);   // 高字节
    SPI_WriteByte(data & 0xFF); // 低字节
    CS_HIGH();
}
static void SPI_WriteWord(uint16_t data) {
    CS_LOW();
    DC_HIGH();
    SPI_WriteByte(data >> 8);   // 高字节
    SPI_WriteByte(data & 0xFF); // 低字节
	//HAL_SPI_Transmit_DMA(&hspi1, &data,2);
    CS_HIGH();
}
#define LCD_WR_DATA WriteData
#define LCD_WR_REG WriteCmd
/* ==================== ILI9341初始化 ==================== */

void LCD_Init(void) {
 
    RST_HIGH(); HAL_Delay(10);
    RST_LOW();  HAL_Delay(10); 
    RST_HIGH(); HAL_Delay(10);
    
   LCD_WR_REG(0xCF);  
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0xC9); //C1 
	LCD_WR_DATA(0X30); 
	LCD_WR_REG(0xED);  
	LCD_WR_DATA(0x64); 
	LCD_WR_DATA(0x03); 
	LCD_WR_DATA(0X12); 
	LCD_WR_DATA(0X81); 
	LCD_WR_REG(0xE8);  
	LCD_WR_DATA(0x85); 
	LCD_WR_DATA(0x10); 
	LCD_WR_DATA(0x7A); 
	LCD_WR_REG(0xCB);  
	LCD_WR_DATA(0x39); 
	LCD_WR_DATA(0x2C); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x34); 
	LCD_WR_DATA(0x02); 
	LCD_WR_REG(0xF7);  
	LCD_WR_DATA(0x20); 
	LCD_WR_REG(0xEA);  
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x00); 
	LCD_WR_REG(0xC0);    //Power control 
	LCD_WR_DATA(0x1B);   //VRH[5:0] 
	LCD_WR_REG(0xC1);    //Power control 
	LCD_WR_DATA(0x00);   //SAP[2:0];BT[3:0] 01 
	LCD_WR_REG(0xC5);    //VCM control 
	LCD_WR_DATA(0x30); 	 //3F
	LCD_WR_DATA(0x30); 	 //3C
	LCD_WR_REG(0xC7);    //VCM control2 
	LCD_WR_DATA(0XB7); 
	LCD_WR_REG(0x36);    // Memory Access Control 
	LCD_WR_DATA(0x08); 
	LCD_WR_REG(0x3A);   
	LCD_WR_DATA(0x55); 
	LCD_WR_REG(0xB1);   
	LCD_WR_DATA(0x00);   
	LCD_WR_DATA(0x1A); 
	LCD_WR_REG(0xB6);    // Display Function Control 
	LCD_WR_DATA(0x0A); 
	LCD_WR_DATA(0xA2); 
	LCD_WR_REG(0xF2);    // 3Gamma Function Disable 
	LCD_WR_DATA(0x00); 
	LCD_WR_REG(0x26);    //Gamma curve selected 
	LCD_WR_DATA(0x01); 
	LCD_WR_REG(0xE0);    //Set Gamma 
	LCD_WR_DATA(0x0F); 
	LCD_WR_DATA(0x2A); 
	LCD_WR_DATA(0x28); 
	LCD_WR_DATA(0x08); 
	LCD_WR_DATA(0x0E); 
	LCD_WR_DATA(0x08); 
	LCD_WR_DATA(0x54); 
	LCD_WR_DATA(0XA9); 
	LCD_WR_DATA(0x43); 
	LCD_WR_DATA(0x0A); 
	LCD_WR_DATA(0x0F); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x00); 		 
	LCD_WR_REG(0XE1);    //Set Gamma 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x15); 
	LCD_WR_DATA(0x17); 
	LCD_WR_DATA(0x07); 
	LCD_WR_DATA(0x11); 
	LCD_WR_DATA(0x06); 
	LCD_WR_DATA(0x2B); 
	LCD_WR_DATA(0x56); 
	LCD_WR_DATA(0x3C); 
	LCD_WR_DATA(0x05); 
	LCD_WR_DATA(0x10); 
	LCD_WR_DATA(0x0F); 
	LCD_WR_DATA(0x3F); 
	LCD_WR_DATA(0x3F); 
	LCD_WR_DATA(0x0F); 
	LCD_WR_REG(0x2B); 
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x01);
	LCD_WR_DATA(0x3f);
	LCD_WR_REG(0x2A); 
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0xef);	 
	LCD_WR_REG(0x11); //Exit Sleep
	HAL_Delay(120);
	LCD_WR_REG(0x29); //display on		

}

/* ==================== 颜色宏 ==================== */
#define RGB565(r,g,b) 65535-(((r>>3)<<11) | ((g>>2)<<5) | (b>>3))

/* ==================== 画图函数 ==================== */
void SPI1_Boost(){
	 hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
}
void LCD_FillScreen(uint16_t color) {
	uint16_t x=0,y=0;
   WriteCmd(0x2A);
    WriteData(x >> 8); WriteData(x & 0xFF);
    WriteData((x+239) >> 8); WriteData((x+239) & 0xFF);
    
    WriteCmd(0x2B);
    WriteData(y >> 8); WriteData(y & 0xFF);
    WriteData((y+319) >> 8); WriteData((y+319) & 0xFF);
    
    WriteCmd(0x2C);  // 写显存命令
    
    CS_LOW();
    DC_HIGH();
    
    for(uint32_t i = 0; i < 240*320; i++) {
      CS_LOW();
    DC_HIGH();
	HAL_SPI_Transmit_DMA(&hspi1, &color,2);
    CS_HIGH();
    }
    
    CS_HIGH();
}


// MicroLIB 缺失的符号
void __aeabi_assert(const char *expr, const char *file, int line)
{
    (void)expr;
    (void)file;
    (void)line;
	return;
    //while(1);  // 或者直接返回
}

int _atexit_init(void)
{
    return 0;
}

int _atexit_mutex(void)
{
    return 0;
}

// 快速填充 (用缓冲区减少循环次数)
void LCD_FillScreen_Fast(uint16_t color) {
    uint8_t buf[64*2];  // 一次发64个像素
    
    // 填充缓冲区
    for(int i = 0; i < 64*2; i+=2) {
        buf[i]   = color >> 8;
        buf[i+1] = color & 0xFF;
    }
    
    uint32_t total = 240 * 320 / 64; // 分多少次发送
    
    WriteCmd(0x2C);
    CS_LOW();
    DC_HIGH();
    
    for(uint32_t j = 0; j < total; j++) {
        for(int i = 0; i < 64*2; i++) {
            SPI_WriteByte(buf[i]);
        }
    }
    
    CS_HIGH();
}

/* ==================== 测试函数 ==================== */
void LCD_Test(void) {
    // 测试1: 红色
    LCD_FillScreen_Fast(RGB565(255, 0, 0));
//HAL_Delay(1000);
    
    // 测试2: 绿色
    LCD_FillScreen_Fast(RGB565(0, 255, 0));
 //   HAL_Delay(1000);
    
    // 测试3: 蓝色
    LCD_FillScreen_Fast(RGB565(0, 0, 255));
   // HAL_Delay(1000);
    
    // 测试4: 白色
    LCD_FillScreen_Fast(RGB565(255, 255, 255));
  //  HAL_Delay(1000);
    
    // 测试5: 黑色
    LCD_FillScreen_Fast(RGB565(0, 0, 0));
  //  HAL_Delay(1000);
}

/* ==================== 分段测试 (用于诊断) ==================== */
void LCD_Diagnostic(void) {
    // 1. 仅复位，看屏幕是否有变化
    RST_HIGH();
    HAL_Delay(200);
    // 此时如果屏幕亮，应该是白屏或花屏
    
    // 2. 仅发送开启显示命令
    WriteCmd(0x29);
    HAL_Delay(100);
    
    // 3. 写一个像素点
    WriteCmd(0x2A); 
    WriteData(0x00); WriteData(0x00);  // X=0
    WriteData(0x00); WriteData(0x00);  // X=0
    
    WriteCmd(0x2B);
    WriteData(0x00); WriteData(0x00);  // Y=0
    WriteData(0x00); WriteData(0x00);  // Y=0
    
    WriteCmd(0x2C);
    CS_LOW();
    DC_HIGH();
    SPI_WriteByte(0xFF);  // 一个红色像素的高字节
    SPI_WriteByte(0x00);  // 低字节
    CS_HIGH();
}
void LCD_SetSpinAngle(uint16_t spinAngle) {
    uint8_t madctl;
    
    // 取模360并映射到最近的标准方向
    // 标准方向: 0, 90, 180, 270
    uint16_t angle = spinAngle % 360;
    
    // 四舍五入到最近的90度
    // 0-44 -> 0, 45-134 -> 90, 135-224 -> 180, 225-314 -> 270, 315-359 -> 0
    uint16_t dir;
    if(angle < 45 || angle >= 315) {
        dir = 0;
    } else if(angle < 135) {
        dir = 90;
    } else if(angle < 225) {
        dir = 180;
    } else {
        dir = 270;
    }
    
    // 根据方向设置MADCTL寄存器的值
    switch(dir) {
        case 0:     // 0度 (竖屏, 默认)
            madctl = 0x48;  // MY=0, MX=1, MV=0, ML=0, BGR=1
            break;
            
        case 90:    // 90度 (横屏)
            madctl = 0x28;  // MY=0, MX=1, MV=1, ML=0, BGR=1
            break;
            
        case 180:   // 180度 (倒屏)
            madctl = 0x88;  // MY=1, MX=1, MV=0, ML=0, BGR=1
            break;
            
        case 270:   // 270度 (反向横屏)
            madctl = 0xE8;  // MY=1, MX=0, MV=1, ML=0, BGR=1
            break;
            
        default:    // 默认竖屏
            madctl = 0x48;
            break;
    }
    
    // 发送MADCTL命令
    WriteCmd(0x36);
    WriteData(madctl);
    
    // 重新设置全屏窗口
    WriteCmd(0x2A);  // 列地址
    WriteData(0x00); WriteData(0x00);
    WriteData(0x00); WriteData(0xEF);  // 239
    
    WriteCmd(0x2B);  // 行地址
    WriteData(0x00); WriteData(0x00);
    WriteData(0x01); WriteData(0x3F);  // 319
    
    // 清屏
    LCD_FillScreen(0x0000);  // 黑色清屏
}

/* ==================== DrawPoint 函数 ==================== */
void DrawPoint(uint16_t x, uint16_t y, uint16_t color) {
    if(x >= 240 || y >= 320) return;
    
    WriteCmd(0x2A);
    WriteData(x >> 8); WriteData(x & 0xFF);
    WriteData(x >> 8); WriteData(x & 0xFF);
    
    WriteCmd(0x2B);
    WriteData(y >> 8); WriteData(y & 0xFF);
    WriteData(y >> 8); WriteData(y & 0xFF);
    
    WriteCmd(0x2C);
    CS_LOW();
    DC_HIGH();
    SPI_WriteWord(color);
    CS_HIGH();
}

/* ==================== WriteChar3232 函数 ==================== */
void WriteChar3232(uint16_t x, uint16_t y, const uint8_t *font_data, uint16_t color, uint16_t bg_color) {
    if(x + 32 > 240 || y + 32 > 320) return;
    
    WriteCmd(0x2A);
    WriteData(x >> 8); WriteData(x & 0xFF);
    WriteData((x+31) >> 8); WriteData((x+31) & 0xFF);
    
    WriteCmd(0x2B);
    WriteData(y >> 8); WriteData(y & 0xFF);
    WriteData((y+31) >> 8); WriteData((y+31) & 0xFF);
    
    WriteCmd(0x2C);
    CS_LOW();
    DC_HIGH();
    
    for(int col = 0; col < 32; col++) {
        for(int row = 0; row < 32; row++) {
            if(font_data[row * 4 + col / 8] & (0x80 >> (col % 8))) {
                SPI_WriteWord(color);
            } else {
                SPI_WriteWord(bg_color);
            }
        }
    }
    
    CS_HIGH();
}

void WriteASCII3232(uint16_t x,uint16_t y,uint16_t color, uint16_t bg_color,char which){
	uint8_t ascii_dat=which;
	ascii_dat-=32;
	WriteChar3232(y,x,Font_ASCII[ascii_dat].dat,color,bg_color);
}

/* ==================== WriteString3232 函数 ==================== */
void WriteString3232(uint16_t x, uint16_t y, uint16_t color, uint16_t bg_color, const char *str) {
    uint16_t start_x = x;
    
    while(*str) {
        if(*str == '\n') {
            // 换行
            y += 32;  // 32像素高度 + 3像素间距
            x = start_x;
        } else if(*str == ' ') {
            // 空格
            x += 28;  // 32像素宽度 + 3像素间距
        } else {
            // 显示字符
            WriteASCII3232(x, y, color, bg_color, *str);
            x += 28;  // 32像素宽度 + 3像素间距
        }
        
        // 超出屏幕宽度自动换行
        if(x + 30 > 320) {
            y += 32;
            x = start_x;
        }
        
        str++;
    }
}



float measure12V(){
	int resraw=0;
	HAL_DAC_SetValue(&hdac3,DAC_CHANNEL_1,DAC_ALIGN_12B_R,0);
	for(uint16_t i=0;i<4096;i++){
		HAL_DAC_SetValue(&hdac3,DAC_CHANNEL_1,DAC_ALIGN_12B_R,i);
		if(HAL_COMP_GetOutputLevel(&hcomp1)==COMP_OUTPUT_LEVEL_LOW){
			resraw=i;
			break;
		}
	}
	return ((resraw/4096.0f)*11.0f*AREF);
}
float av12_voltage=0;
extern uint16_t adc1[49];
struct adc1_read{
	float temp;
	float amp;
	float volt;
}adc1_readings;
void adc1_convert(struct adc1_read* read1){
	read1->temp=adc1[2];
	read1->amp=adc1[1];
	read1->volt=((adc1[0]-32767.0f)/65536.0f)*3.0f*16.0f*-1.7605f;
}
char temp_buf[10];
int system_startup_check(){
	
	WriteString3232(0,20,RGB565(255,255,255),RGB565(0,0,0),"LOADING ");
	int pass=0;
	if(measure12V()>10&&measure12V()<15)pass++;
	adc1_convert(&adc1_readings);
	if(adc1_readings.volt<30&&adc1_readings.volt>-1)pass+=2;
	if(pass>=3){
   LCD_FillScreen(RGB565(0,0,0));
   WriteString3232(0,60,RGB565(0,255,0),RGB565(0,0,0),"System OK\n");
		HAL_Delay(10);
		return 1;
	}else if(pass==1){
    LCD_FillScreen(RGB565(255,0,0));
		WriteString3232(0,60,RGB565(255,0,255),RGB565(0,0,0),"Check Input\n");
		HAL_Delay(10);
		char buf[16];
		snprintf(buf,12,"INP=%.2fV",adc1_readings.volt);
		WriteString3232(0,100,RGB565(0,255,255),RGB565(0,0,0),buf);
		HAL_Delay(10);
		return 0;
	}else if(pass==2){
  //  LCD_FillScreen(RGB565(0,0,0));
		int v12_now=(int)(measure12V());
		snprintf(temp_buf,10,"VSYS=%dV\n",v12_now);
		WriteString3232(0,60,RGB565(255,255,0),RGB565(0,0,0),"12V Fault\nCheck PSU\n");
		WriteString3232(0,180,RGB565(255,255,0),RGB565(0,0,0),temp_buf);
		HAL_Delay(10);
		
		return 0;
		
	}else if(pass==0){
   // LCD_FillScreen(RGB565(0,0,0));
		WriteString3232(0,60,RGB565(255,0,0),RGB565(0,0,0),"Multiple Errors\n");
		HAL_Delay(10);
		return 0;
	}
	return 0;
}



#define CURRENT_OFFSET 0.56



uint8_t Touch_Read(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin, uint16_t *x, uint16_t *y) {
    uint8_t rx_buf[3];  // 接收缓冲区
    uint32_t x_sum = 0, y_sum = 0;
    uint8_t valid = 0;
    
    for (uint8_t i = 0; i < 5; i++) {  // 采样5次
        // ====== 读取X坐标 ======
        HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);  // CS拉低
        
        // 发送命令 0xD0，同时接收第1个字节
        uint8_t cmd = 0xD0;
        HAL_SPI_TransmitReceive(hspi, &cmd, &rx_buf[0], 1, HAL_MAX_DELAY);
        
        // 发送假数据，同时接收第2个字节
        cmd = 0x00;
        HAL_SPI_TransmitReceive(hspi, &cmd, &rx_buf[1], 1, HAL_MAX_DELAY);
        
        HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);    // CS拉高
        
        // 解析X坐标 (12位: rx_buf[0]高4位 + rx_buf[1]全部)
        uint16_t x_val = ((uint16_t)rx_buf[1]<<8 ) | (rx_buf[0]);
        
        // ====== 读取Y坐标 ======
        HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);  // CS拉低
        
        cmd = 0x90;
        HAL_SPI_TransmitReceive(hspi, &cmd, &rx_buf[0], 1, HAL_MAX_DELAY);
        
        cmd = 0x00;
        HAL_SPI_TransmitReceive(hspi, &cmd, &rx_buf[1], 1, HAL_MAX_DELAY);
        
        HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);    // CS拉高
        
        // 解析Y坐标
        uint16_t y_val = ((uint16_t)rx_buf[1]<<8 ) | (rx_buf[0] );
        
        // 判断是否有效（阈值100，可根据实际情况调整）
        if (x_val > 100 && y_val > 100) {
            x_sum += x_val;
            y_sum += y_val;
            valid++;
        }
    }
    
    // 至少3次有效采样才返回坐标
    if (valid >= 3) {
        *x = x_sum / valid;
        *y = y_sum / valid;
        return 1;  // 触摸有效
    }
    
    *x = 0;
    *y = 0;
    return 0;  // 未触摸
}

void TouchScreen_Calibrate(){
	LCD_FillScreen(RGB565(255,255,255));
	WriteASCII3232(0,0,RGB565(0,255,0),RGB565(0,0,0),'+');
	WriteASCII3232(320-32,0,RGB565(0,255,0),RGB565(0,0,0),'+');
	WriteASCII3232(0,240-32,RGB565(0,255,0),RGB565(0,0,0),'+');
	WriteASCII3232(320-32,240-32,RGB565(0,255,0),RGB565(0,0,0),'+');
	uint8_t flag=0;
	while(1){
		uint16_t ii,jj,rdX,rdY;
				if(Touch_Read(&hspi2,GPIOF,GPIO_PIN_0,&ii,&jj)){
					rdX=ii;
					rdY=jj;
				}
				uint32_t tmpx=(uint32_t)rdY*320*2;
				uint32_t tmpy=(uint32_t)rdX*240*2;
				tmpx/=65536;
				tmpy/=65536;
			  rdX=tmpx;
				rdY=tmpy;
				if(rdX<50&&rdY<30){
					WriteASCII3232(0,0,RGB565(255,0,255),RGB565(0,0,0),'+');
					flag|=1;
				}
					if(rdX>320-50&&rdY<30){
					WriteASCII3232(320-32,0,RGB565(255,0,255),RGB565(0,0,0),'+');
						flag|=2;
				}
					if(rdX<50&&rdY>240-30){
					WriteASCII3232(0,240-32,RGB565(255,0,255),RGB565(0,0,0),'+');
						flag|=4;
				}
					if(rdX>320-50&&rdY>240-30){
					WriteASCII3232(320-32,240-32,RGB565(255,0,255),RGB565(0,0,0),'+');
						flag|=8;
				}
					if(flag==15)break;
			}
		}
float ii=0;
int steps=32;
int32_t dacval=0;
float target_current,current_prop=5;

float testcurrent=0.0f;
int readX=0,readY=0;
int32_t sinetables[20]={116708, 152775, 185294, 211128, 227697, 233414, 227697, 211128, 185294, 152775,116708, 80641, 48122, 22288, 5719, 0, 5719, 22288, 48122, 80641};
extern uint8_t flag_start;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_ADC4_Init();
  MX_COMP1_Init();
  MX_DAC1_Init();
  MX_DAC3_Init();
  MX_LPTIM1_Init();
  MX_TIM2_Init();
  MX_USART1_Init();
  MX_SPI1_Init();
  MX_CRC_Init();
  MX_TIM6_Init();
  MX_SPI2_Init();
  MX_TIM16_Init();
  MX_TouchGFX_Init();
  /* USER CODE BEGIN 2 */
	HAL_LPTIM_Encoder_Start(&hlptim1,65535);
		HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1,DAC_ALIGN_12B_R,0);
	HAL_DAC_Start(&hdac1,DAC_CHANNEL_1);
	HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1,DAC_ALIGN_12B_R,0);
		HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2,DAC_ALIGN_12B_R,0);
			HAL_DAC_Start(&hdac1,DAC_CHANNEL_2);
				HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2,DAC_ALIGN_12B_R,0);
	HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
	HAL_ADC_Start_DMA(&hadc1,(uint32_t*)(adc1),48);
HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_4);
htim2.Instance->CCR4=0;
HAL_COMP_Start(&hcomp1);
HAL_DAC_Start(&hdac3,DAC_CHANNEL_1);
    LCD_Init();
LCD_SetSpinAngle(90);
SPI1_Boost();
LCD_FillScreen(RGB565(0,0,0));
htim2.Instance->CCR4=htim2.Instance->ARR-1;
LCD_FillScreen(RGB565(0,0,0));

while(!system_startup_check()){
}
LCD_FillScreen(RGB565(255,0,0));
HAL_Delay(50);
//TouchScreen_Calibrate();
//MX_TouchGFX_Process();
LCD_SetSpinAngle(270);
WriteCmd(0x36);
WriteData(0x40); 
WriteCmd(0x21);

flag_start=1;
  // LCD_Test();  // 依次显示红绿蓝白黑
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

  MX_TouchGFX_Process();
    /* USER CODE BEGIN 3 */
	/*	volt=(float)adc_volt[0]-32845.0f;
		volt/=65536;
		volt*=3*16;
		volt2=volt;
//av12_voltage=measure12V();
char buf[16];
		//snprintf(buf,12,"12V=%.2fV",av12_voltage);
		//WriteString3232(0,80,RGB565(0,255,0),RGB565(0,0,0),buf);
	//	adc1_convert(&adc1_readings);
		//snprintf(buf,12,"%.3fPI",ii);
	//WriteString3232(0,130,RGB565(255,255,255),RGB565(0,0,0),buf);
		//WriteCurrentDAC((233415/2)+((233415/10)*sinf(ii)));
		ii+=(3.14/steps);
		if(ii>6.28)ii=0;
		//WriteCurrentDAC(dacval);
	
		
		
	uint16_t ii,jj;
				if(Touch_Read(&hspi2,GPIOF,GPIO_PIN_0,&ii,&jj)){
					readX=ii;
					readY=jj;
				}
				uint32_t tmpx=(uint32_t)readY*320*2;
				uint32_t tmpy=(uint32_t)readX*240*2;
				tmpx/=65536;
				tmpy/=65536;
			  readX=tmpx;
				readY=tmpy;
				sprintf(buf,"X:%05d\n",readX);
		WriteString3232(0,140,RGB565(0,255,0),RGB565(0,0,0),buf);
				sprintf(buf,"Y:%05d\n",readY);
		WriteString3232(0,180,RGB565(0,255,0),RGB565(0,0,0),buf);
					
	//	ILI9341_FillScreen(0x00ff);*/
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
