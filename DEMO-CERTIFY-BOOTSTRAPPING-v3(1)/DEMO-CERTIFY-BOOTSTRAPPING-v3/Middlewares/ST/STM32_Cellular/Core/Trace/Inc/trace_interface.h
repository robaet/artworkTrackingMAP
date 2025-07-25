/**
  ******************************************************************************
  * @file    trace_interface.h
  * @author  MCD Application Team
  * @brief   Header for trace_interface.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef TRACE_INTERFACE_H
#define TRACE_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "plf_config.h"
#include "cellular_runtime_standard.h"
#include "cellular_runtime_custom.h"

/* define debug levels (bitmap) */
typedef uint8_t dbg_levels_t;

#define DBL_LVL_P0     (dbg_levels_t)0x01U
#define DBL_LVL_P1     (dbg_levels_t)0x02U
#define DBL_LVL_P2     (dbg_levels_t)0x04U
#define DBL_LVL_WARN   (dbg_levels_t)0x08U
#define DBL_LVL_ERR    (dbg_levels_t)0x10U
#define DBL_LVL_VALID  (dbg_levels_t)0x20U

/* following flags select debug interface(s) to use : to be defined in plf_sw_config.h
  #define TRACE_IF_TRACES_ITM     (1)
  #define TRACE_IF_TRACES_UART    (1)
*/

/* DEBUG MASK defines the allowed traces : to be defined in plf_sw_config.h */
/* Full traces */
/* #define TRACE_IF_MASK    (uint16_t)(DBL_LVL_P0 | DBL_LVL_P1 | DBL_LVL_P2 | DBL_LVL_WARN | DBL_LVL_ERR) */
/* Warn and Error traces only */
/* #define TRACE_IF_MASK    (uint16_t)(DBL_LVL_WARN | DBL_LVL_ERR) */


/* Maximum buffer size (per channel) */
#define DBG_IF_MAX_BUFFER_SIZE  (uint16_t)(256)

/* Exported types ------------------------------------------------------------*/

/* Define here the list of 32 ITM channels (0 to 31) */
typedef enum
{
  DBG_CHAN_GENERIC = 0,
  DBG_CHAN_MAIN,
  DBG_CHAN_ATCMD,
  DBG_CHAN_COMLIB,
#if (USE_CUSTOM_CLIENT == 1)
  DBG_CHAN_CUSTOMCLIENT,
#endif /* USE_CUSTOM_CLIENT == 1 */
#if (USE_ECHO_CLIENT == 1)
  DBG_CHAN_ECHOCLIENT,
#endif /* USE_ECHO_CLIENT == 1 */
#if (USE_HTTP_CLIENT == 1)
  DBG_CHAN_HTTP,
#endif /* USE_HTTP_CLIENT == 1 */
#if (USE_PING_CLIENT == 1)
  DBG_CHAN_PING,
#endif /* USE_PING_CLIENT == 1 */
#if (USE_COM_CLIENT == 1)
  DBG_CHAN_COMCLIENT,
#endif /* USE_COM_CLIENT == 1  */
#if (USE_MQTT_CLIENT == 1)
  DBG_CHAN_MQTTCLIENT,
#endif /* USE_MQTT_CLIENT == 1 */
#if (USE_UI_CLIENT == 1)
  DBG_CHAN_UICLIENT,
#endif /* USE_UI_CLIENT == 1   */
  DBG_CHAN_IPC,
  DBG_CHAN_PPPOSIF,
  DBG_CHAN_CELLULAR_SERVICE,
  DBG_CHAN_NIFMAN,
  DBG_CHAN_DATA_CACHE,
  DBG_CHAN_UTILITIES,
  DBG_CHAN_ERROR_LOGGER,
  DBG_CHAN_VALID,
  DBG_CHAN_TEST,
  DBG_CHAN_MAX_VALUE        /* keep last */
} dbg_channels_t;


/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
extern uint8_t dbgIF_buf[DBG_CHAN_MAX_VALUE][DBG_IF_MAX_BUFFER_SIZE];
extern uint8_t *traceIF_UartBusyFlag;

/* Exported functions ------------------------------------------------------- */
/**
  * @brief  Trace off - Set trace to disable
  * @param  -
  * @retval -
  */
void traceIF_trace_off(void);

/**
  * @brief  Trace on - Set trace to enable
  * @param  -
  * @retval -
  */
void traceIF_trace_on(void);

/**
  * @brief  Print a trace on ITM
  * @param  port - component channel
  * @param  lvl - trace level
  * @param  pptr - pointer on the trace
  * @param  len - length of the trace
  * @retval -
  */
void traceIF_itmPrint(uint8_t port, uint8_t lvl, uint8_t *pptr, uint16_t len);

/**
  * @brief  Print a trace on UART
  * @param  port - component channel
  * @param  lvl - trace level
  * @param  pptr - pointer on the trace
  * @param  len - length of the trace
  * @retval -
  */
void traceIF_uartPrint(uint8_t port, uint8_t lvl, uint8_t *pptr, uint16_t len);

/**
  * @brief  Print a trace on ITM even if global or component trace is disable
  * @param  port - component channel
  * @param  pptr - pointer on the trace
  * @param  len - length of the trace
  * @retval -
  */
void traceIF_itmPrintForce(uint8_t port, uint8_t *pptr, uint16_t len);

/**
  * @brief  Print a trace on ITM even if global or component trace is disable
  * @param  port - component channel
  * @note   port is an unused parameter
  * @param  pptr - pointer on the trace
  * @param  len - length of the trace
  * @retval -
  */
void traceIF_uartPrintForce(uint8_t port, uint8_t *pptr, uint16_t len);

/**
  * @brief  Print a trace in hexadecimal format
  * @note   Available for ITM or UART trace And NOT for printf
  * @param  chan - component channel
  * @param  lvl  - trace level
  * @param  buff - pointer on the trace
  * @param  len  - length of the trace
  * @retval -
  */
void traceIF_hexPrint(dbg_channels_t chan, dbg_levels_t level, uint8_t *buff, uint16_t len);

/**
  * @brief  Print a trace in character format - treat special character e.g
  *  '\0' replace by string <NULL CHAR>
  *  '\r' replace by string <CR>
  *  '\n' replace by string <LF>
  * @param  chan  - component channel
  * @param  level - trace level
  * @param  buf   - pointer on the trace
  * @param  size  - size of the trace
  * @retval -
  */
void traceIF_BufCharPrint(dbg_channels_t chan, dbg_levels_t level, const CRC_CHAR_t *buf, uint16_t size);

/**
  * @brief  Print a trace in hexadecimal format
  * @param  chan  - component channel
  * @param  level - trace level
  * @param  buf   - pointer on the trace
  * @param  size  - size of the trace
  * @retval -
  */
void traceIF_BufHexPrint(dbg_channels_t chan, dbg_levels_t level, const CRC_CHAR_t *buf, uint16_t size);

#if ((TRACE_IF_TRACES_ITM == 1U) && (TRACE_IF_TRACES_UART == 1U))
#define TRACE_PRINT(chan, lvl, format, args...) \
  (void)sprintf((CRC_CHAR_t *)dbgIF_buf[(chan)], format "", ## args);\
  traceIF_itmPrint((uint8_t)(chan), (uint8_t)lvl, (uint8_t *)dbgIF_buf[(chan)],\
                   (uint16_t)crs_strlen(dbgIF_buf[(chan)]));\
  traceIF_uartPrint( (uint8_t)(chan), (uint8_t)lvl, (uint8_t *)dbgIF_buf[(chan)],\
                     (uint16_t)crs_strlen(dbgIF_buf[(chan)]));
#elif (TRACE_IF_TRACES_ITM == 1U)
#define TRACE_PRINT(chan, lvl, format, args...) \
  (void)sprintf((CRC_CHAR_t *)dbgIF_buf[(chan)], format "", ## args);\
  traceIF_itmPrint((uint8_t)(chan), (uint8_t)lvl, (uint8_t *)dbgIF_buf[(chan)],\
                   (uint16_t)crs_strlen(dbgIF_buf[(chan)]));
#elif (TRACE_IF_TRACES_UART == 1U)
#define TRACE_PRINT(chan, lvl, format, args...) \
  (void)sprintf((CRC_CHAR_t *)dbgIF_buf[(chan)], format "", ## args);\
  traceIF_uartPrint((uint8_t)(chan), (uint8_t)lvl, (uint8_t *)dbgIF_buf[(chan)],\
                    (uint16_t)crs_strlen(dbgIF_buf[(chan)]));
#else
#define TRACE_PRINT(...)      __NOP(); /* Nothing to do */
#endif  /* ((TRACE_IF_TRACES_ITM == 1U) && (TRACE_IF_TRACES_UART == 1U)) */

/* To force traces even if they are deactivated (used in Boot Menu for example) */
#define TRACE_PRINT_FORCE(chan, lvl, format, args...) \
  (void)sprintf((CRC_CHAR_t *)dbgIF_buf[(chan)], format "", ## args);\
  traceIF_itmPrint((uint8_t)(chan), 1, (uint8_t *)dbgIF_buf[(chan)],\
                   (uint16_t)crs_strlen(dbgIF_buf[(chan)]));\
  traceIF_uartPrintForce((uint8_t)(chan), (uint8_t *)dbgIF_buf[(chan)],\
                         (uint16_t)crs_strlen(dbgIF_buf[(chan)]));

#define TRACE_VALID(format, args...) \
  (void)sprintf((CRC_CHAR_t *)dbgIF_buf[(DBG_CHAN_VALID)], format "", ## args);\
  traceIF_uartPrintForce((uint8_t)(DBG_CHAN_VALID), (uint8_t *)dbgIF_buf[(DBG_CHAN_VALID)],\
                         (uint16_t)crs_strlen(dbgIF_buf[(DBG_CHAN_VALID)]));

#define TRACE_PRINT_BUF_CHAR(chan, lvl, pbuf, size) traceIF_BufCharPrint((chan), (lvl), (pbuf), (size))
#define TRACE_PRINT_BUF_HEX(chan, lvl, pbuf, size)  traceIF_BufHexPrint((chan), (lvl), (pbuf), (size))


/*** Component Initialization/Start *******************************************/
/*** Internal use only - Not an Application Interface *************************/

/**
  * @brief  Component initialization
  * @note   must be called only one time :
  *         - before using any other functions of traceIF_*
  * @param  -
  * @retval -
  */
void traceIF_init(void);

/**
  * @brief  Component start
  * @note   must be called only one time but
            after traceIF_init
            and before using any other functions of traceIF_*
  * @param  -
  * @retval -
  */
void traceIF_start(void);

#ifdef __cplusplus
}
#endif

#endif /* TRACE_INTERFACE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
