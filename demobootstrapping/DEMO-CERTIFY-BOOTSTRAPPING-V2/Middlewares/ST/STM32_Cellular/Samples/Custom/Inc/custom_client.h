/**
  ******************************************************************************
  * @file    custom_client.h
  * @author  MCD Application Team
  * @brief   Header for custom_client.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef CUSTOM_CLIENT_H
#define CUSTOM_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "plf_config.h"

#if (USE_CUSTOM_CLIENT == 1)

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

/**
  * @brief  Initialization
  * @note   CustomClient initialization - first function called
  * @param  -
  * @retval -
  */
void custom_client_init(void);

/**
  * @brief  Start
  * @note  CustomClient start - function called when platform is initialized
  * @param  -
  * @retval -
  */
void custom_client_start(void);

#endif /* USE_CUSTOM_CLIENT == 1 */

#ifdef __cplusplus
}
#endif

#endif /* CUSTOM_CLIENT_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

