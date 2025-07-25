/**
  ******************************************************************************
  * @file    com_sockets_statistic.c
  * @author  MCD Application Team
  * @brief   This file implements Socket statistic
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "com_sockets_statistic.h"

#if (COM_SOCKETS_STATISTIC == 1U)

#include <string.h>
#include <stdio.h>

#include "rtosal.h"

/* Private defines -----------------------------------------------------------*/
#if ((USE_TRACE_COM_SOCKETS == 1U) || (USE_CMD_CONSOLE == 1U))
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_STAT(format, args...) \
  TRACE_PRINT(DBG_CHAN_COMLIB, DBL_LVL_P0, "" format "\n\r", ## args)
#else /* USE_PRINTF == 1U */
#define PRINT_STAT(format, args...) \
  (void)printf("" format "\n\r", ## args);
#endif /* USE_PRINTF == 0U */

#else /* (USE_TRACE_COM_SOCKETS == 0U) && (USE_CMD_CONSOLE == 0U) */
#define PRINT_STAT(...)             \
  __NOP(); /* Nothing to do */

#endif /* (USE_TRACE_COM_SOCKETS == 1U) || (USE_CMD_CONSOLE == 1U) */

#if (USE_DATACACHE == 1)
#include "dc_common.h"
#endif /* USE_DATACACHE == 1 */

#if (USE_RTC == 1)
#include "time_date.h"
#endif /* USE_RTC == 1 */

/* Private typedef -----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
/* Socket statistics definition */
typedef struct
{
  uint16_t sock_cre_ok;
  uint16_t sock_cre_nok;
  uint16_t sock_cnt_ok;
  uint16_t sock_cnt_nok;
  uint16_t sock_snd_ok;
  uint16_t sock_snd_nok;
  uint16_t sock_rcv_ok;
  uint16_t sock_rcv_nok;
  uint16_t sock_cls_ok;
  uint16_t sock_cls_nok;
#if (USE_DATACACHE == 1)
  uint16_t nwk_up;
  uint16_t nwk_dwn;
#endif /* USE_DATACACHE == 1 */
} com_socket_statistic_t;

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

#if 0
#if ((USE_TRACE_COM_SOCKETS == 1U) || (USE_CMD_CONSOLE == 1U))
/* Statistic socket state readable print */
static const uint8_t *com_socket_state_string[] =
{
  "Invalid",
  "Creating",
  "Created",
  "Connected",
  "Sending",
  "Receiving",
  "Closing"
};
#endif /* (USE_TRACE_COM_SOCKETS == 1U) || (USE_CMD_CONSOLE == 1U) */
#endif /* not yet supported */

/* Statistic socket variable */
static com_socket_statistic_t com_socket_statistic;

/* Private typedef -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Callback prototype */
/* Callback called when timer expires */
#if (COM_SOCKETS_STATISTIC_PERIOD != 0U)
static void com_socket_statistic_timer_cb(void *argument);
#endif /* COM_SOCKETS_STATISTIC_PERIOD != 0U */

/* Private function Definition -----------------------------------------------*/

#if (COM_SOCKETS_STATISTIC_PERIOD != 0U)
/**
  * @brief  Called when statistic timer raised
  * @note   Request com socket statistic print
  * @param  argument - parameter passed at creation of osTimer
  * @note   Unused
  * @retval -
  */
static void com_socket_statistic_timer_cb(void *argument)
{
  UNUSED(argument);
  com_sockets_statistic_display();
}
#endif /* COM_SOCKETS_STATISTIC_PERIOD != 0U */

/* Functions Definition ------------------------------------------------------*/

/*** Used by com_sockets module - Not an User Interface ***********************/
/**
  * @brief  Component initialization
  * @note   must be called only one time and
  *         before using any other functions of com_*
  * @param  -
  * @retval -
  */
void com_sockets_statistic_init(void)
{
#if (COM_SOCKETS_STATISTIC_PERIOD != 0U)
  /* Statistic display timer */
  static osTimerId com_socket_statistic_timer_handle;
#endif /* COM_SOCKETS_STATISTIC_PERIOD != 0U */

  /* Initialize socket statistics structure to 0U */
  (void)memset(&com_socket_statistic, 0, sizeof(com_socket_statistic_t));

#if (COM_SOCKETS_STATISTIC_PERIOD != 0U)
  /* Timer creation */
  com_socket_statistic_timer_handle = rtosalTimerNew(NULL, (os_ptimer)com_socket_statistic_timer_cb, osTimerPeriodic,
                                                     NULL);
  if (com_socket_statistic_timer_handle != NULL)
  {
    (void)rtosalTimerStart(com_socket_statistic_timer_handle, (uint32_t)(COM_SOCKETS_STATISTIC_PERIOD * 60000U));
  }
  else
  {
    PRINT_STAT("ComLibStat: Timer creation NOK")
  }

#endif /* COM_SOCKETS_STATISTIC_PERIOD != 0U */
}

/*** Used by com_sockets_* modules - Not an User Interface ********************/
/**
  * @brief  Managed com sockets statistic update
  * @note   -
  * @param  stat - to know what the function has to do
  * @note   statistic update
  * @retval -
  */
void com_sockets_statistic_update(com_sockets_stat_update_t stat)
{
  switch (stat)
  {
#if (USE_DATACACHE == 1)
    /* Network status managed through Datacache */
    case COM_SOCKET_STAT_NWK_UP:
    {
      com_socket_statistic.nwk_up++;
      break;
    }
    case COM_SOCKET_STAT_NWK_DWN:
    {
      com_socket_statistic.nwk_dwn++;
      break;
    }
#endif /* USE_DATACACHE == 1 */
    case COM_SOCKET_STAT_CRE_OK:
    {
      com_socket_statistic.sock_cre_ok++;
      break;
    }
    case COM_SOCKET_STAT_CRE_NOK:
    {
      com_socket_statistic.sock_cre_nok++;
      break;
    }
    case COM_SOCKET_STAT_CNT_OK:
    {
      com_socket_statistic.sock_cnt_ok++;
      break;
    }
    case COM_SOCKET_STAT_CNT_NOK:
    {
      com_socket_statistic.sock_cnt_nok++;
      break;
    }
    case COM_SOCKET_STAT_SND_OK:
    {
      com_socket_statistic.sock_snd_ok++;
      break;
    }
    case COM_SOCKET_STAT_SND_NOK:
    {
      com_socket_statistic.sock_snd_nok++;
      break;
    }
    case COM_SOCKET_STAT_RCV_OK:
    {
      com_socket_statistic.sock_rcv_ok++;
      break;
    }
    case COM_SOCKET_STAT_RCV_NOK:
    {
      com_socket_statistic.sock_rcv_nok++;
      break;
    }
    case COM_SOCKET_STAT_CLS_OK:
    {
      com_socket_statistic.sock_cls_ok++;
      break;
    }
    case COM_SOCKET_STAT_CLS_NOK:
    {
      com_socket_statistic.sock_cls_nok++;
      break;
    }
    default:
    {
      /* Nothing to do */
      break;
    }
  }
}

/**
  * @brief  Display com sockets statistics
  * @note   COM_SOCKETS_STATISTIC and USE_TRACE_COM_SOCKETS must be set to 1
  * @param  -
  * @retval -
  */
void com_sockets_statistic_display(void)
{
#if 0
  socket_desc_t *socket_desc;
  socket_desc = socket_desc_list;
#endif /* not yet supported */

  /* Check that at least one socket has run */
  if (com_socket_statistic.sock_cre_ok != 0U)
  {
    PRINT_STAT("*** Socket Stat Begin ***")
#if (USE_RTC == 1)
    /* Date and Time managed through Datacache */
    timedate_t time;

    (void)timedate_get(&time, TIMEDATE_DATE_AND_TIME);

    PRINT_STAT("%02d/%02d/%04d - %02d:%02d:%02d",
               time.mday,
               time.month,
               time.year,
               time.hour,
               time.min,
               time.sec)
#endif /* (USE_RTC == 1) */

#if (USE_DATACACHE == 1)
    /* Network status managed through Datacache */
    PRINT_STAT("Nwk: up:%5d dwn:%5d tot:%6d",
               com_socket_statistic.nwk_up,
               com_socket_statistic.nwk_dwn,
               (com_socket_statistic.nwk_up + com_socket_statistic.nwk_dwn))
#endif /* USE_DATACACHE == 1 */

    PRINT_STAT("Cre: ok:%5d nok:%5d tot:%6d",
               com_socket_statistic.sock_cre_ok,
               com_socket_statistic.sock_cre_nok,
               (com_socket_statistic.sock_cre_ok + com_socket_statistic.sock_cre_nok))
    PRINT_STAT("Con: ok:%5d nok:%5d tot:%6d",
               com_socket_statistic.sock_cnt_ok,
               com_socket_statistic.sock_cnt_nok,
               (com_socket_statistic.sock_cnt_ok + com_socket_statistic.sock_cnt_nok))
    PRINT_STAT("Snd: ok:%5d nok:%5d tot:%6d",
               com_socket_statistic.sock_snd_ok,
               com_socket_statistic.sock_snd_nok,
               (com_socket_statistic.sock_snd_ok + com_socket_statistic.sock_snd_nok))
    PRINT_STAT("Rcv: ok:%5d nok:%5d tot:%6d",
               com_socket_statistic.sock_rcv_ok,
               com_socket_statistic.sock_rcv_nok,
               (com_socket_statistic.sock_rcv_ok + com_socket_statistic.sock_rcv_nok))
    PRINT_STAT("Cls: ok:%5d nok:%5d tot:%6d",
               com_socket_statistic.sock_cls_ok,
               com_socket_statistic.sock_cls_nok,
               (com_socket_statistic.sock_cls_ok + com_socket_statistic.sock_cls_nok))
#if 0
    /* Socket status displayed */
    while (socket_desc != NULL)
    {
      if (socket_desc->local == COM_SOCKETS_FALSE)
      {
        PRINT_STAT("Sock: id:%3d-State:%s-Err:%d",
                   socket_desc->id,
                   com_socket_state_string[socket_desc->state],
                   socket_desc->error)
      }
      else
      {
        PRINT_STAT("Ping: id:%3d-State:%s-Err:%d",
                   socket_desc->id,
                   com_socket_state_string[socket_desc->state],
                   socket_desc->error)
      }
      socket_desc = socket_desc->next;
    }
#endif /* not yet supported */
    PRINT_STAT("*** Socket Stat End ***")
  }
}

#else /* COM_SOCKETS_STATISTIC == 0U */
/**
  * @brief  Component initialization
  * @note   must be called only one time and
  *         before using any other functions of com_*
  * @param  -
  * @retval -
  */
void com_sockets_statistic_init(void)
{
  /* Nothing to do */
  __NOP();
}

/**
  * @brief  Managed com sockets statistic update and print
  * @note   -
  * @param  stat - to know what the function has to do
  * @note   statistic init, update or print
  * @retval -
  */
void com_sockets_statistic_update(com_sockets_stat_update_t stat)
{
  UNUSED(stat);
  /* Nothing to do */
}

/**
  * @brief  Display com sockets statistics
  * @note   COM_SOCKETS_STATISTIC and USE_TRACE_COM_SOCKETS must be set to 1
  * @param  -
  * @retval -
  */
void com_sockets_statistic_display(void)
{
  /* Nothing to do */
}

#endif /* COM_SOCKET_STATISTIC == 1U */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
