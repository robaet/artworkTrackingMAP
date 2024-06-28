/**
  ******************************************************************************
  * @file    cellular_app_sensors.c
  * @author  MCD Application Team
  * @brief   Implements functions for sensors actions.
  *          Supported sensors : humidity, pressure, temperature
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

/* Includes ------------------------------------------------------------------*/
#include "cellular_app_sensors.h"

#if (USE_SENSORS == 1)

/* Private typedef -----------------------------------------------------------*/
/* Cellular App Sensor descriptor */
typedef struct
{
  bool status;/* Sensor Status: false: not initialized, true: initialized */
} cellular_app_sensor_desc_t;

/* Private defines -----------------------------------------------------------*/
/* Used to size array of sensors */
#define CELLULAR_APP_SENSOR_TYPE_MAX (CELLULAR_APP_SENSOR_TYPE_TEMPERATURE + 1U)

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Array of sensors */
static cellular_app_sensor_desc_t cellular_app_sensor[CELLULAR_APP_SENSOR_TYPE_MAX];

/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private Functions Definition ----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Initialize a sensor
  * @retval bool - false/true - sensor init NOK / sensor init OK
  */
bool cellular_app_sensors_initialize(cellular_app_sensor_type_t type)
{
  bool result = false; /* false: sensor init NOK, true: sensor init OK */

  switch (type)
  {
    case CELLULAR_APP_SENSOR_TYPE_HUMIDITY :
      if (BSP_ENV_SENSOR_Init_Humidity() == BSP_ERROR_NONE)
      {
        result = true; /* Humidity sensor init OK */
      }
      break;
    case CELLULAR_APP_SENSOR_TYPE_PRESSURE :
      if (BSP_ENV_SENSOR_Init_Pressure() == BSP_ERROR_NONE)
      {
        result = true; /* Pressure sensor init OK */
      }
      break;
    case CELLULAR_APP_SENSOR_TYPE_TEMPERATURE :
      if (BSP_ENV_SENSOR_Init_Temperature() == BSP_ERROR_NONE)
      {
        result = true; /* Temperature sensor init OK */
      }
      break;
    default :
      __NOP(); /* because result already set to false */
      break;
  }

  /* Update internal sensor status */
  if (type < CELLULAR_APP_SENSOR_TYPE_MAX)
  {
    cellular_app_sensor[type].status = result;
  }

  return (result);
}

/**
  * @brief  Read a sensor
  * @retval bool - false/true - sensor read NOK/ sensor read OK
  */
bool cellular_app_sensors_read(cellular_app_sensor_type_t type, cellular_app_sensors_data_t *const p_data)
{
  bool result = false; /* false: sensor read NOK, true: sensor read OK */

  if (p_data != NULL)
  {
    switch (type)
    {
      case CELLULAR_APP_SENSOR_TYPE_HUMIDITY :
        /* Before to read sensor, is Humidity sensor initialize ? */
        if (cellular_app_sensor[CELLULAR_APP_SENSOR_TYPE_HUMIDITY].status == true)
        {
          if (BSP_ENV_SENSOR_ReadT_Humidity(&(p_data->float_data)) == BSP_ERROR_NONE)
          {
            result = true; /* Humidity sensor read OK */
          }
          /* else __NOP(); because result already set to false */
        }
        /* else __NOP(); because result already set to false */
        break;
      case CELLULAR_APP_SENSOR_TYPE_PRESSURE :
        /* Before to read sensor, is Pressure sensor initialize ? */
        if (cellular_app_sensor[CELLULAR_APP_SENSOR_TYPE_PRESSURE].status == true)
        {
          if (BSP_ENV_SENSOR_Read_Pressure(&(p_data->float_data)) == BSP_ERROR_NONE)
          {
            result = true; /* Pressure sensor read OK */
          }
          /* else __NOP(); because result already set to false */
        }
        /* else __NOP(); because result already set to false */
        break;
      case CELLULAR_APP_SENSOR_TYPE_TEMPERATURE :
        /* Before to read sensor, is Temperature sensor initialize ? */
        if (cellular_app_sensor[CELLULAR_APP_SENSOR_TYPE_TEMPERATURE].status == true)
        {
          if (BSP_ENV_SENSOR_Read_Temperature(&(p_data->float_data)) == BSP_ERROR_NONE)
          {
            result = true; /* Temperature sensor read OK */
          }
          /* else __NOP(); because result already set to false */
        }
        /* else __NOP(); because result already set to false */
        break;
      default :
        __NOP(); /* because result already set to false */
        break;
    }
  }

  return (result);
}

/**
  * @brief  Initialize sensor module
  * @retval -
  */
void cellular_app_sensors_init(void)
{
  /* Initialize sensor status to false: not initialize */
  for (uint8_t i = 0; i < CELLULAR_APP_SENSOR_TYPE_MAX; i++)
  {
    cellular_app_sensor[i].status = false;
  }
}

#endif /* USE_SENSORS == 1 */
