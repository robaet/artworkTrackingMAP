/**
  ******************************************************************************
  * @file    comclient.h
  * @author  MCD Application Team
  * @brief   Header for comclient.c module
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
#ifndef COM_CLIENT_H
#define COM_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "plf_config.h"

#if (USE_COM_CLIENT == 1)

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

/**
  * @brief  Initialization
  * @note   COMClient initialization - first function to call
  * @param  -
  * @retval -
  */
void comclient_init(void);

/**
  * @brief  Start
  * @note   COMClient start - must be called after comclient_init
  * @param  -
  * @retval -
  */
void comclient_start(void);

#endif /* USE_COM_CLIENT == 1 */

#ifdef __cplusplus
}
#endif

#endif /* COM_CLIENT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
