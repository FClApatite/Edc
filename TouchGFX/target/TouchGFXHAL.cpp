/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : TouchGFXHAL.cpp
  ******************************************************************************
  * This file was created by TouchGFX Generator 4.26.1. This file is only
  * generated once! Delete this file from your project and re-generate code
  * using STM32CubeMX or change this file manually to update it.
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

#include <TouchGFXHAL.hpp>

/* USER CODE BEGIN TouchGFXHAL.cpp */

using namespace touchgfx;

/* ******************************************************
 * Functions required by Partial Frame Buffer Strategy
 * ******************************************************
 *
 *  int touchgfxDisplayDriverTransmitActive() must return whether or not data is currently being transmitted, over e.g. SPI.
 *  void touchgfxDisplayDriverTransmitBlock(const uint8_t* pixels, uint16_t x, uint16_t y, uint16_t w, uint16_t h) will be called
 *  when the framework wants to send a block. The user must then transfer the data represented by the arguments.
 *
 *  A user must call touchgfx::startNewTransfer(); once touchgfxDisplayDriverTransmitBlock() has successfully sent a block.
 *  E.g. if using DMA to transfer the block, this could be called in the "Transfer Completed" interrupt handler.
 *
 */
#warning "A user must call touchgfx::startNewTransfer(); once touchgfxDisplayDriverTransmitBlock() has succesfully sent a block."
#warning "A user must implement C-methods touchgfxDisplayDriverTransmitActive() and touchgfxDisplayDriverTransmitBlock() used by the Partial Framebuffer Strategy."

#include "spi.h"
#include "gpio.h"
#include "stm32g4xx.h"
extern class Semaphores_t{
	public:
	bool isHRTIMidle;
	bool isTIM16idle;
	bool isDACidle;
	bool isSPI1idle;
	bool isSPI2idle;
	bool isADCidle;
	bool isLCDidle;
	void Init(){
		isHRTIMidle=1;
		isTIM16idle=1;
		isDACidle=1;
		isSPI1idle=1;
		isSPI2idle=1;
		isADCidle=1;
		isLCDidle=1;
	}
}occupancy;
int touchgfxDisplayDriverTransmitActive(){
return (!occupancy.isLCDidle);
}

#define LCD_CS_PORT1     GPIOA
#define LCD_CS_PIN1      GPIO_PIN_15

#define LCD_RST_PORT1    GPIOA
#define LCD_RST_PIN1     GPIO_PIN_11

#define LCD_DC_PORT1     GPIOA
#define LCD_DC_PIN1      GPIO_PIN_12

/* 软件SPI引脚 (使用任意GPIO，这里用PA0=SCK, PA1=MOSI) */
#define LCD_SCK_PORT1    GPIOB
#define LCD_SCK_PIN1     GPIO_PIN_3

#define LCD_MOSI_PORT1   GPIOA
#define LCD_MOSI_PIN1    GPIO_PIN_6

/* ==================== 基本操作宏 ==================== */
#define CS_LOW1()    HAL_GPIO_WritePin(LCD_CS_PORT1, LCD_CS_PIN1, GPIO_PIN_RESET)
#define CS_HIGH1()   HAL_GPIO_WritePin(LCD_CS_PORT1 ,LCD_CS_PIN1, GPIO_PIN_SET)

#define RST_LOW1()   HAL_GPIO_WritePin(LCD_RST_PORT1, LCD_RST_PIN1, GPIO_PIN_RESET)
#define RST_HIGH1()  HAL_GPIO_WritePin(LCD_RST_PORT1, LCD_RST_PIN1, GPIO_PIN_SET)

#define DC_LOW1()    HAL_GPIO_WritePin(LCD_DC_PORT1, LCD_DC_PIN1, GPIO_PIN_RESET)
#define DC_HIGH1()   HAL_GPIO_WritePin(LCD_DC_PORT1, LCD_DC_PIN1, GPIO_PIN_SET)

#define SCK_LOW1()   HAL_GPIO_WritePin(LCD_SCK_PORT1, LCD_SCK_PIN1, GPIO_PIN_RESET)
#define SCK_HIGH1()  HAL_GPIO_WritePin(LCD_SCK_PORT1, LCD_SCK_PIN1, GPIO_PIN_SET)

static inline void SPI_WriteByte1(uint8_t data) {
    // 使用HAL库的SPI发送函数
    // HAL_SPI_Transmit(SPI句柄, 数据缓冲区, 数据长度, 超时时间)
    HAL_SPI_Transmit_DMA(&hspi1, &data,1);
}
/* ==================== 命令/数据写入 ==================== */
static inline void WriteCmd1(uint8_t cmd) {
    CS_LOW1();
    DC_LOW1();       // DC=0 表示命令
    SPI_WriteByte1(cmd);
    CS_HIGH1();
}

static inline void WriteData1(uint8_t data) {
    CS_LOW1();
    DC_HIGH1();      // DC=1 表示数据
    SPI_WriteByte1(data);
    CS_HIGH1();
}
static void SPI_WriteWord1(uint16_t data) {
    CS_LOW1();
    DC_HIGH1();
    SPI_WriteByte1(data >> 8);   // 高字节
    SPI_WriteByte1(data & 0xFF); // 低字节
	//HAL_SPI_Transmit_DMA(&hspi1, &data,2);
    CS_HIGH1();
}
namespace touchgfx {
    extern void startNewTransfer();
 
}
void touchgfxDisplayDriverTransmitBlock(const uint8_t* pixels, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	if(x+w>320||y+h>240)return;
	occupancy.isLCDidle=0;
	if(x>320) x=320;
	if(y>240) y=240;

	//x=320-x;
	y=240-y;
//	occupancy.isLCDidle=1;
    // Set the column address range
  WriteCmd1(0x2A);
    WriteData1(x >> 8); WriteData1(x & 0xFF);
    uint16_t x_end = x + w - 1;
    WriteData1(x_end >> 8); WriteData1(x_end & 0xFF);
    
    // Set the page address range
    WriteCmd1(0x2B);
    WriteData1(y >> 8); WriteData1(y & 0xFF);
    uint16_t y_end = y + h - 1;
    WriteData1(y_end >> 8); WriteData1(y_end & 0xFF);
    
    // Start writing pixel data
    WriteCmd1(0x2C);
    
    // Send all pixel data in a burst
    CS_LOW1();
    DC_HIGH1();
    
    uint32_t total_pixels = (uint32_t)w * h;
    for (uint32_t i = 0; i < total_pixels; i++)
    {
        // pixels buffer contains 16-bit RGB565 colors
        uint16_t color = ((uint16_t)pixels[i * 2] << 8) | pixels[i * 2 + 1];
        SPI_WriteWord1(color);
    }
    
    CS_HIGH1();
		touchgfx::startNewTransfer();
	//	occupancy.isLCDidle=0;
		occupancy.isLCDidle=1;
}

   
void TouchGFXHAL::initialize()
{
    // Calling parent implementation of initialize().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.
    // Please note, HAL::initialize() must be called to initialize the framework.

    TouchGFXGeneratedHAL::initialize();
}

/**
 * Gets the frame buffer address used by the TFT controller.
 *
 * @return The address of the frame buffer currently being displayed on the TFT.
 */
uint16_t* TouchGFXHAL::getTFTFrameBuffer() const
{
    // Calling parent implementation of getTFTFrameBuffer().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    return TouchGFXGeneratedHAL::getTFTFrameBuffer();
}

/**
 * Sets the frame buffer address used by the TFT controller.
 *
 * @param [in] address New frame buffer address.
 */
void TouchGFXHAL::setTFTFrameBuffer(uint16_t* address)
{
    // Calling parent implementation of setTFTFrameBuffer(uint16_t* address).
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    TouchGFXGeneratedHAL::setTFTFrameBuffer(address);
}

/**
 * This function is called whenever the framework has performed a partial draw.
 *
 * @param rect The area of the screen that has been drawn, expressed in absolute coordinates.
 *
 * @see flushFrameBuffer().
 */
void TouchGFXHAL::flushFrameBuffer(const touchgfx::Rect& rect)
{
    // Calling parent implementation of flushFrameBuffer(const touchgfx::Rect& rect).
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.
    // Please note, HAL::flushFrameBuffer(const touchgfx::Rect& rect) must
    // be called to notify the touchgfx framework that flush has been performed.
    // To calculate the start address of rect,
    // use advanceFrameBufferToRect(uint8_t* fbPtr, const touchgfx::Rect& rect)
    // defined in TouchGFXGeneratedHAL.cpp

    TouchGFXGeneratedHAL::flushFrameBuffer(rect);
}

bool TouchGFXHAL::blockCopy(void* RESTRICT dest, const void* RESTRICT src, uint32_t numBytes)
{
    return TouchGFXGeneratedHAL::blockCopy(dest, src, numBytes);
}

/**
 * Configures the interrupts relevant for TouchGFX. This primarily entails setting
 * the interrupt priorities for the DMA and LCD interrupts.
 */
void TouchGFXHAL::configureInterrupts()
{
    // Calling parent implementation of configureInterrupts().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    TouchGFXGeneratedHAL::configureInterrupts();
}

/**
 * Used for enabling interrupts set in configureInterrupts()
 */
void TouchGFXHAL::enableInterrupts()
{
    // Calling parent implementation of enableInterrupts().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    TouchGFXGeneratedHAL::enableInterrupts();
}

/**
 * Used for disabling interrupts set in configureInterrupts()
 */
void TouchGFXHAL::disableInterrupts()
{
    // Calling parent implementation of disableInterrupts().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    TouchGFXGeneratedHAL::disableInterrupts();
}

/**
 * Configure the LCD controller to fire interrupts at VSYNC. Called automatically
 * once TouchGFX initialization has completed.
 */
void TouchGFXHAL::enableLCDControllerInterrupt()
{
    // Calling parent implementation of enableLCDControllerInterrupt().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    TouchGFXGeneratedHAL::enableLCDControllerInterrupt();
}

bool TouchGFXHAL::beginFrame()
{
    return TouchGFXGeneratedHAL::beginFrame();
}

void TouchGFXHAL::endFrame()
{
    TouchGFXGeneratedHAL::endFrame();
}

/* USER CODE END TouchGFXHAL.cpp */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
