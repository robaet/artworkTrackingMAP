/**
  ******************************************************************************
  * @file    plf_hw_config.h
  * @author  MCD Application Team
  * @brief   This file contains the hardware configuration of the platform
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef PLF_HW_CONFIG_H
#define PLF_HW_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* MISRAC messages linked to HAL include are ignored */
/*cstat -MISRAC2012-* */
#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
/*cstat +MISRAC2012-* */

#include "main.h"
#include "plf_features.h"
//#include "plf_modem_config.h"
//#include "usart.h" /* Modem Uart and Input/Output: Cmd and Trace */
//#include "rng.h"   /* hrng access for ComSockets when USE_SOCKETS_TYPE == USE_SOCKETS_MODEM and ComIcc */

/* Exported constants --------------------------------------------------------*/

/* Platform defines ----------------------------------------------------------*/

/* ST33 configuration */
#define ST33_SPI_HANDLE                 hspi3
#define ST33_SPI_INSTANCE               SPI3
#define ST33_SPI_BAUDRATEPRESCALER      SPI_BAUDRATEPRESCALER_8
#define ST33_SPI_MOSI_PIN               SPI3_MOSI_PIN
#define ST33_SPI_MISO_PIN               SPI3_MISO_PIN
#define ST33_SPI_MOSI_MISO_SCK_PORT     SPI3_PORT
#define ST33_SPI_CS_PIN                 ST33_CS_Pin
#define ST33_SPI_CS_PORT                ST33_CS_GPIO_Port
#define ST33_SPI_ALT_FUNCTION           SPI3_ALT_FUNCTION
#define ST33_SPI_SCK_PIN                SPI3_SCK_PIN

#define MODEM_UART_HANDLE       huart3
#define MODEM_UART_INSTANCE     USART3
#define MODEM_UART_AUTOBAUD     (0)
#define MODEM_UART_IRQN         USART3_IRQn

#define MODEM_UART_BAUDRATE     (CONFIG_MODEM_UART_BAUDRATE)
#define MODEM_UART_WORDLENGTH   UART_WORDLENGTH_8B
#define MODEM_UART_STOPBITS     UART_STOPBITS_1
#define MODEM_UART_PARITY       UART_PARITY_NONE
#define MODEM_UART_MODE         UART_MODE_TX_RX

#if (CONFIG_MODEM_UART_RTS_CTS == 1)
#define MODEM_UART_HWFLOWCTRL   UART_HWCONTROL_RTS_CTS
#else
#define MODEM_UART_HWFLOWCTRL   UART_HWCONTROL_NONE
#endif /* (CONFIG_MODEM_UART_RTS_CTS == 1) */

#define MODEM_TX_GPIO_PORT      ((GPIO_TypeDef *)MODEM_UART_TX_GPIO_Port)
#define MODEM_TX_PIN            MODEM_UART_TX_Pin
#define MODEM_RX_GPIO_PORT      ((GPIO_TypeDef *)MODEM_UART_RX_GPIO_Port)
#define MODEM_RX_PIN            MODEM_UART_RX_Pin
#define MODEM_CTS_GPIO_PORT     ((GPIO_TypeDef *)MODEM_UART_CTS_GPIO_Port)
#define MODEM_CTS_PIN           MODEM_UART_CTS_Pin
#define MODEM_RTS_GPIO_PORT     ((GPIO_TypeDef *)MODEM_UART_RTS_GPIO_Port)
#define MODEM_RTS_PIN           MODEM_UART_RTS_Pin

/* ---- MODEM other pins configuration ---- */
/* output */
#define MODEM_PWR_EN_GPIO_PORT          MODEM_PWR_EN_GPIO_Port
#define MODEM_PWR_EN_PIN                MODEM_PWR_EN_Pin
#define MODEM_DTR_GPIO_PORT             MODEM_DTR_GPIO_Port
#define MODEM_DTR_PIN                   MODEM_DTR_Pin

/* input */
#define MODEM_RING_GPIO_PORT    ((GPIO_TypeDef *)MODEM_RING_GPIO_Port)
#define MODEM_RING_PIN          MODEM_RING_Pin
#define MODEM_RING_IRQN         EXTI15_10_IRQn


/* DEBUG INTERFACE CONFIGURATION */
#define TRACE_INTERFACE_UART_HANDLE     huart1
#define TRACE_INTERFACE_INSTANCE        USART1

/* RANDOM NUMBER GENERATOR CONFIGURATION */
#define RNG_HANDLE                       hrng /* if (RNG_HANDLE) is defined then HAL_RNG_GenerateRandomNumber is used
                                               * else (RNG_HANDLE not defined) rand() is used */

#if ((USE_DISPLAY == 1) || (USE_ST33 == 1))
#define SPI_INTERFACE                    (1)
#endif /* (USE_DISPLAY == 1) || (USE_ST33 == 1) */

#if (USE_DISPLAY == 1)
#include "stm32l462e_cell1.h"
#include "stm32l462e_cell1_lcd.h"
/*cstat -MISRAC2012-* */
#include "stm32_lcd.h"
/*cstat +MISRAC2012-* */
#define DISPLAY_WAIT_MODEM_IS_ON         (1) /* 0: no need to wait modem is on before to use display,
                                                1: need to wait modem is on before to use display   */
#define DISPLAY_DEFAULT_FONT             (Font12)
#define DISPLAY_INTERFACE                SPI_INTERFACE
#endif /* USE_DISPLAY == 1 */

#if (USE_ST33 == 1)
#define NDLC_INTERFACE                   SPI_INTERFACE
#endif /* !defined USE_DISPLAY */

#if (USE_ST33 == 1)
#if (NDLC_INTERFACE == SPI_INTERFACE)
#include "gpio.h"
#endif /* NDLC_INTERFACE == SPI_INTERFACE */
#endif /* USE_ST33 == 1 */

#if (USE_SENSORS == 1)
#include "stm32l462e_cell1.h"
#include "stm32l462e_cell1_env_sensors.h"
#endif /* USE_SENSORS == 1 */

/* Exported types ------------------------------------------------------------*/

/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* PLF_HW_CONFIG_H */
