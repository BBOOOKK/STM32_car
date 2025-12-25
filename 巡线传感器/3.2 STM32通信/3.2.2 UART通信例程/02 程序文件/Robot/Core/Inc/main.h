/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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
#define KEY_Pin GPIO_PIN_0
#define KEY_GPIO_Port GPIOC
#define BT_EN_Pin GPIO_PIN_1
#define BT_EN_GPIO_Port GPIOC
#define BAT_Pin GPIO_PIN_3
#define BAT_GPIO_Port GPIOC
#define BUS_EN_Pin GPIO_PIN_1
#define BUS_EN_GPIO_Port GPIOA
#define BUS_TX_Pin GPIO_PIN_2
#define BUS_TX_GPIO_Port GPIOA
#define BUS_RX_Pin GPIO_PIN_3
#define BUS_RX_GPIO_Port GPIOA
#define FLASH_CLK_Pin GPIO_PIN_5
#define FLASH_CLK_GPIO_Port GPIOA
#define FLASH_DO_Pin GPIO_PIN_6
#define FLASH_DO_GPIO_Port GPIOA
#define FLASH_DI_Pin GPIO_PIN_7
#define FLASH_DI_GPIO_Port GPIOA
#define FLASH_CS_Pin GPIO_PIN_14
#define FLASH_CS_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_15
#define LED1_GPIO_Port GPIOB
#define PWM_SERVO6_Pin GPIO_PIN_11
#define PWM_SERVO6_GPIO_Port GPIOC
#define PWM_SERVO5_Pin GPIO_PIN_12
#define PWM_SERVO5_GPIO_Port GPIOC
#define PWM_SERVO4_Pin GPIO_PIN_2
#define PWM_SERVO4_GPIO_Port GPIOD
#define PWM_SERVO3_Pin GPIO_PIN_3
#define PWM_SERVO3_GPIO_Port GPIOB
#define PWM_SERVO2_Pin GPIO_PIN_4
#define PWM_SERVO2_GPIO_Port GPIOB
#define PWM_SERVO1_Pin GPIO_PIN_5
#define PWM_SERVO1_GPIO_Port GPIOB
#define BUZZER_Pin GPIO_PIN_8
#define BUZZER_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_9
#define LED_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
