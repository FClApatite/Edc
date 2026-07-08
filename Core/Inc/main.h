/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TMP_MEAS_Pin GPIO_PIN_0
#define TMP_MEAS_GPIO_Port GPIOA
#define AMP_MEAS1_Pin GPIO_PIN_1
#define AMP_MEAS1_GPIO_Port GPIOA
#define DAC_H_Pin GPIO_PIN_4
#define DAC_H_GPIO_Port GPIOA
#define DAC_L_Pin GPIO_PIN_5
#define DAC_L_GPIO_Port GPIOA
#define VOLT_MEAS_Pin GPIO_PIN_0
#define VOLT_MEAS_GPIO_Port GPIOB
#define CMP_12V_MEAS_Pin GPIO_PIN_1
#define CMP_12V_MEAS_GPIO_Port GPIOB
#define TOUCH_IRQ2_Pin GPIO_PIN_2
#define TOUCH_IRQ2_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_11
#define LED_GPIO_Port GPIOB
#define AMP_MEAS2_Pin GPIO_PIN_12
#define AMP_MEAS2_GPIO_Port GPIOB
#define LCD_RST_Pin GPIO_PIN_11
#define LCD_RST_GPIO_Port GPIOA
#define LCD_DC_Pin GPIO_PIN_12
#define LCD_DC_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
