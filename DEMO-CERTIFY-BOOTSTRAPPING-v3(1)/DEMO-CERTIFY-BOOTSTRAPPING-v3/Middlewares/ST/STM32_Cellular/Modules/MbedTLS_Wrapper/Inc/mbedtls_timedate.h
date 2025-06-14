/**
  ******************************************************************************
  * @file    mbedtls_timedate.h
  * @author  MCD Application Team
  * @brief   Header file for timingSystem.c
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */


#ifndef MBEDTLS_TIMEDATE_H
#define MBEDTLS_TIMEDATE_H

#include "plf_config.h"

#include <time.h>

#define TD_OK             0
#define TD_ERR_CONNECT   -1   /**< Could not connect to the network and join the web server. */
#define TD_ERR_HTTP      -2   /**< Could not parse the time and date from the web server response. */
#define TD_ERR_RTC       -3   /**< Could not set the RTC. */
#define TD_ERR_TLS_CERT  -4   /**< The server certificate verification failed.
                                      Applicable only when force_apply is false. */

int32_t mbedtls_timedate_set_from_network(void);

#endif  /* MBEDTLS_TIMEDATE_H */

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/

