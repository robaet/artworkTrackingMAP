/**
  ******************************************************************************
  * @file    cellular_app_uiclient.h
  * @author  MCD Application Team
  * @brief   Header for cellular_app_uiclient.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef CELLULAR_APP_UICLIENT_H
#define CELLULAR_APP_UICLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "plf_config.h"

#if (USE_DISPLAY == 1)

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/**
  * @brief  Initialize all needed structures to support UIClt feature
  * @param  -
  * @retval -
  */
void cellular_app_uiclient_init(void);

/**
  * @brief  Start UIClt thread
  * @param  -
  * @retval -
  */
void cellular_app_uiclient_start(void);

#endif /* USE_DISPLAY == 1 */

#ifdef __cplusplus
}
#endif

#endif /* CELLULAR_APP_UICLIENT_H */
