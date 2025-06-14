/**
  ******************************************************************************
  * @file    custom_client.c
  * @author  MCD Application Team
  * @brief   Custom Client Sample
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

/* Includes ------------------------------------------------------------------*/
#include "plf_config.h"

#if (USE_CUSTOM_CLIENT == 1)

#include <stdbool.h>
#include <time.h>

#include "custom_client.h"

/* FreeRTOS include */
#include "rtosal.h"

/* Error management */
#include "error_handler.h"

/* Datacache includes */
#include "dc_common.h"
#include "cellular_datacache.h"

#include "com_sockets_net_compat.h"
#include "dc_mems.h"

#include "cellular_service_utils.h"


#define SERVER_LOG_IP                 	((uint32_t)857055654) /*13.60.194.107  222085739*/ /* 0x9B22D734U 52.215.34.155  2478242818 */     /* 52.47.67.227   0x342f43e3    875512803  */
//#define SERVER_LOG_PORT                 ((uint16_t)9999)
#define MUD_LINK_PORT					((uint16_t)4000)
#define MUD_URL 						"13.60.22.32:6000/mud"
#define MUD_DEVICE_ID					((int)1)



typedef struct
{
	char data[1024];
	int data_len;
} logBuffer_t;

static logBuffer_t socketResponse;

static logBuffer_t logBuffer;
static uint16_t SERVER_LOG_PORT = -1;

static bool custom_log_mems()
{
	dc_temperature_info_t   temperature_info;
	dc_humidity_info_t		humidity_info;
	char 					mems_string[64];
	int						mems_string_len;

	char str_tmp[100] = "";
	char str_hum[100] = "";
	char str_dbm[100] = "";
	float temp_value = 0;
	float hum_value = 0;
	int32_t dbm_value = cst_cellular_info.cs_signal_level_db;

	// read the MEM data
	(void)dc_com_read(&dc_com_db, DC_COM_TEMPERATURE, (void *)&temperature_info, sizeof(temperature_info));
    (void)dc_com_read(&dc_com_db, DC_COM_HUMIDITY, (void *)&humidity_info, sizeof(humidity_info));

    temp_value = temperature_info.temperature;
    int tempInt = (int)temp_value;
	int tmpInt1 = temp_value;
	float tmpFrac = temp_value - tmpInt1;
	int tmpInt2 = trunc(tmpFrac * 100);

	hum_value = humidity_info.humidity;
	int humInt = (int)hum_value;
	int humInt1 = hum_value;
	float humFrac = hum_value - humInt1;
	int humInt2 = trunc(humFrac * 100);

	// convert to string the temperature and humidity
    //mems_string_len= sprintf(mems_string,"time=%d;temperature=%f;humidity=%f;acceleration=%li, %li, %li",xTaskGetTickCount(),temperature_info.temperature, humidity_info.humidity, acc_data.AXIS_X, acc_data.AXIS_Y, acc_data.AXIS_Z);
    //mems_string_len= sprintf(mems_string,"time=%lu;temperature=%f;humidity=%f",(unsigned long)time(NULL),temperature_info.temperature, humidity_info.humidity);
    mems_string_len= sprintf(mems_string, "{\"time\": \"%d\",\"dbm\": \"%d\",\"temperature\": \"%f\",\"humidity\": \"%f\"}", xTaskGetTickCount(), dbm_value, temperature_info.temperature, humidity_info.humidity);

    snprintf(str_tmp,100," TEMPERATURE = %d.%02d degree C\n\r", tmpInt1, tmpInt2);
    snprintf(str_hum,100," HUMIDITY = %d.%02d %%\n\r", humInt1, humInt2);
    snprintf(str_dbm, 100," DBM = %d\n\r", dbm_value);

    HAL_UART_Transmit(&huart1,( uint8_t *)str_tmp,sizeof(str_tmp),1000);
	HAL_UART_Transmit(&huart1,( uint8_t *)str_hum,sizeof(str_hum),1000);
	HAL_UART_Transmit(&huart1,( uint8_t *)str_dbm,sizeof(str_dbm),1000);

	HAL_Delay(1000);
	// append in logBuffer as string
	if ((logBuffer.data_len + mems_string_len) <= (sizeof(logBuffer.data)))
	{
		memcpy(&logBuffer.data[logBuffer.data_len], (const void *)mems_string, mems_string_len);
		logBuffer.data_len += mems_string_len;
	}
	else
	{
		return false;
	}
	return true;
}
uint16_t ntohs(uint16_t netshort) {
    return (netshort << 8) | (netshort >> 8);
}

static bool custom_connect_and_send_data(char * buffer_addr, int buffer_len, int send_mud_link)
{
	  bool 		result;
	  int32_t 	id;
	  int32_t 	timeout = 20000;

	  /* socket need to be created */
	  result = false;

	  /* Create a socket */
	  PRINT_INFO("socket creation in progress...\n\r")

	  id = com_socket(COM_AF_INET, COM_SOCK_STREAM, COM_IPPROTO_TCP);
      if (id >= 0) /* no invalid value defined in network library */
      {
        /* Socket created, continue the process */
    	  PRINT_INFO("socket create OK")

		if (com_setsockopt(id, COM_SOL_SOCKET, COM_SO_RCVTIMEO, &timeout, (int32_t)sizeof(timeout)) == COM_SOCKETS_ERR_OK)
		{
	          if (com_setsockopt(id, COM_SOL_SOCKET, COM_SO_SNDTIMEO, &timeout, (int32_t)sizeof(timeout)) == COM_SOCKETS_ERR_OK)
	          {
                com_sockaddr_in_t address;
                address.sin_family      = (uint8_t)COM_AF_INET;
                address.sin_addr.s_addr = COM_HTONL(SERVER_LOG_IP);
                if(send_mud_link == 1){
                    address.sin_port        = COM_HTONS(MUD_LINK_PORT);
                }
                else
                {
                	address.sin_port        = COM_HTONS(SERVER_LOG_PORT);
                }

                if (com_connect(id, (com_sockaddr_t const *)&address, (int32_t)sizeof(com_sockaddr_in_t))
                    == COM_SOCKETS_ERR_OK)
                {
                   	int32_t 	ret;
                	PRINT_INFO("Send data in progress....\n\r");
                	PRINT_INFO("%s\n\r", buffer_addr);

                	ret = com_send(id, (const com_char_t *)buffer_addr, buffer_len, COM_MSG_WAIT);
                	// Data send ok
                	if (ret == buffer_len)
                	{
						result = true;
					}
                	PRINT_INFO("before data size%d", socketResponse.data_len);
                	if (send_mud_link==1)
                	{
						uint8_t buf[2];
						int32_t ret2 = com_recv(id, (com_char_t *)buf, 2, COM_MSG_WAIT);
						if (ret2 == 2) {
							uint16_t net_port;
							memcpy(&net_port, buf, 2);
							uint16_t port = ntohs(net_port);
							PRINT_INFO("Received port = %u\n", port);  // Should print 5000
							SERVER_LOG_PORT = port;
							result = true;
						} else {
							PRINT_INFO("Receive failed, got %d bytes\n", ret2);
							SERVER_LOG_PORT = -1;
							result = false;
						}
                	}
					// close the socket
					if (com_closesocket(id) == COM_SOCKETS_ERR_OK)
					{
						PRINT_INFO("socket close OK\n\r")
					}
					else
					{
						PRINT_INFO("socket close NOK\n\r")
					}
                }
                else
                {
                	PRINT_INFO("socket connect NOK\n\r")
					if (com_closesocket(id) == COM_SOCKETS_ERR_OK)
						{
							PRINT_INFO("socket close OK\n\r")
						}
                }
	          }
	          else
	          {
	        	  PRINT_INFO("socket setsockopt SNDTIMEO NOK\n\r")
	          }
		}
		else
		{
			PRINT_INFO("socket setsockopt RCVTIMEO NOK\n\r")
		}
      }
      else
      {
    	  PRINT_INFO("socket create NOK\n\r")
      }

 exit:
	  return (result);
}


/* To interact with Custom client through a Terminal connected to the target */
#if ((USE_CMD_CONSOLE == 1) && (CUSTOM_CLIENT_CMD != 0U))
#include "cmd.h"
#endif /* (USE_CMD_CONSOLE == 1) && (CUSTOM_CLIENT_CMD != 0U) */

/* Private defines -----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Use Cellular trace system */
#if (USE_TRACE_CUSTOM_CLIENT == 1U)
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_APP(format, args...) \
  TRACE_PRINT_FORCE(DBG_CHAN_CUSTOMCLIENT, DBL_LVL_P0, "" format, ## args)
#define PRINT_INFO(format, args...) \
  TRACE_PRINT(DBG_CHAN_CUSTOMCLIENT, DBL_LVL_P0, "CustClt: " format, ## args)
#define PRINT_DBG(format, args...) \
  TRACE_PRINT(DBG_CHAN_CUSTOMCLIENT, DBL_LVL_P1, "CustClt: " format "\n\r", ## args)
#else
#include <stdio.h>
#define PRINT_APP(format, args...)   (void)printf("" format, ## args);
#define PRINT_INFO(format, args...)  (void)printf("CustClt: " format, ## args);
#define PRINT_DBG(format, args...)   (void)printf("CustClt: " format "\n\r", ## args);
#endif  /* (USE_PRINTF == 0U) */
#else /* USE_TRACE_CUSTOM_CLIENT == 0 */
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_APP(format, args...) \
  TRACE_PRINT_FORCE(DBG_CHAN_CUSTOMCLIENT, DBL_LVL_P0, "" format, ## args)
#else
#include <stdio.h>
#define PRINT_APP(format, args...)   (void)printf("" format, ## args);
#endif  /* (USE_PRINTF == 0U) */
#define PRINT_INFO(...)  __NOP(); /* Nothing to do */
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#endif /* USE_TRACE_CUSTOM_CLIENT == 1U */

/* Private variables ---------------------------------------------------------*/
static osMessageQId custom_client_queue; /* To communicate between datacache callback
                                            and custom client main thread */
/* Example : */
static bool custom_client_modem_is_attached; /* Modem is attached true/false */

#if ((USE_CMD_CONSOLE == 1) && (CUSTOM_CLIENT_CMD != 0U))
static uint8_t *custom_client_cmd_label = ((uint8_t *)"custclt"); /* string used to interact with custom client
                                                                    through a terminal */
#endif /* (USE_CMD_CONSOLE == 1) && (CUSTOM_CLIENT_CMD != 0U) */

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Callback used to treat datacache information */
static void custom_client_notif_cb(dc_com_event_id_t dc_event_id, const void *p_private_gui_data);

/* Declare custom client thread */
static void custom_client_thread(void *p_argument);

#if ((USE_CMD_CONSOLE == 1) && (CUSTOM_CLIENT_CMD != 0U))
/* Functions used in case of module CMD activation */
static cmd_status_t custom_client_cmd_help(uint8_t *cmd_p,
                                           uint8_t *arg_p);
static cmd_status_t custom_client_cmd(uint8_t *cmd_line_p);
#endif /* (USE_CMD_CONSOLE == 1) && (CUSTOM_CLIENT_CMD != 0U) */

/* Private functions ---------------------------------------------------------*/

#if ((USE_CMD_CONSOLE == 1) && (CUSTOM_CLIENT_CMD != 0U))
/**
  * @brief  help cmd management
  * @param  -
  * @retval -
  */
static cmd_status_t custom_client_cmd_help(uint8_t *cmd_p,
                                           uint8_t *arg_p)
{
  cmd_status_t result;

  if ((cmd_p != NULL)
      && (arg_p != NULL))
  {
    result = CMD_SYNTAX_ERROR;
    PRINT_APP("%s bad parameter %s !!!\n\r", cmd_p, arg_p)
  }
  else
  {
    result = CMD_OK;
  }
  /* Always print help even after an error */

  CMD_print_help(custom_client_cmd_label);
  PRINT_APP("%s help       : display all commands supported by %s\n\r",
            (CRC_CHAR_t *)custom_client_cmd_label,
            (CRC_CHAR_t *)custom_client_cmd_label)
  /* Describe below all other custom client command supported */

  return (result);
}

/**
  * @brief  cmd management
  * @param  cmd_line_p - command parameters
  * @retval cmd_status_t - status of cmd management
  */
static cmd_status_t custom_client_cmd(uint8_t *cmd_line_p)
{
  uint32_t argc;
  uint8_t  *argv_p[10];
  uint8_t  *cmd_p;
  cmd_status_t cmd_status;


  cmd_status = CMD_OK;
  PRINT_APP("\n\r")
  cmd_p = (uint8_t *)strtok((CRC_CHAR_t *)cmd_line_p, " \t");

  /* Check CMD is for custom client */
  if (memcmp((CRC_CHAR_t *)cmd_p,
             (CRC_CHAR_t *)custom_client_cmd_label,
             crs_strlen(cmd_p))
      == 0)
  {
    /* parameters parsing */
    for (argc = 0U; argc < 10U; argc++)
    {
      argv_p[argc] = (uint8_t *)strtok(NULL, " \t");
      if (argv_p[argc] == NULL)
      {
        break;
      }
    }

    if (argc == 0U)
    {
      /* No parameter in the command */
      /* Add the treatment below or if it is an error then print help */
      /* Example:
         cmd_status = custom_client_cmd_help(NULL, NULL);
      */
    }
    /* at least one parameter provided in the command */
    /* Compare to valid parameters */
    else if (memcmp((CRC_CHAR_t *)argv_p[0],
                    "send",
                    crs_strlen(argv_p[0])) == 0)
    {
    	// connect to server and send
		if (custom_connect_and_send_data(logBuffer.data, logBuffer.data_len, 0)== true)
		{
			memset(logBuffer.data, 0, sizeof(logBuffer.data));
			logBuffer.data_len=0;
		}
		memset(logBuffer.data, 0, sizeof(logBuffer.data));
		logBuffer.data_len=0;
    }
    //send mud link
    else if (memcmp((CRC_CHAR_t *)argv_p[0],
					"mud",
					crs_strlen(argv_p[0])) == 0)
	{
		// connect to server and send mudlink
		if (custom_connect_and_send_data(logBuffer.data, logBuffer.data_len, 0)== true)
		{
			memset(logBuffer.data, 0, sizeof(logBuffer.data));
			logBuffer.data_len=0;
		}
	}
    else if (memcmp((CRC_CHAR_t *)argv_p[0],
                    "help",
                    crs_strlen(argv_p[0])) == 0)
    {
      cmd_status = custom_client_cmd_help(NULL, NULL);
    }
    else /* Parameter provided is not recognized */
    {
      cmd_status = custom_client_cmd_help(cmd_p, argv_p[0]);
    }
  }

  return cmd_status;
}
#endif /* (USE_CMD_CONSOLE == 1) && (CUSTOM_CLIENT_CMD != 0U) */

/**
  * @brief  Callback called when a value in datacache changed
  * @note   Managed datacache value changed
  * @param  dc_event_id - value changed
  * @note   -
  * @param  p_private_gui_data - value provided at service subscription
  * @note   Unused parameter
  * @retval -
  */
static void custom_client_notif_cb(dc_com_event_id_t dc_event_id, const void *p_private_gui_data)
{
  UNUSED(p_private_gui_data);

  /* remove next lines to use dc_event_id parameter */
  UNUSED(dc_event_id);

  /* According to dc_event_id value, due a specific treatment */
  /* Example: Modem is attached to the network*/
  if (dc_event_id == DC_CELLULAR_NIFMAN_INFO)
  {
    dc_nifman_info_t dc_nifman_info;
    (void)dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO,
                      (void *)&dc_nifman_info,
                      sizeof(dc_nifman_info));
    if (dc_nifman_info.rt_state == DC_SERVICE_ON)
    {
      custom_client_modem_is_attached = true;
      PRINT_APP("CustomClt: Network is UP\n\r")
      (void)rtosalMessageQueuePut(custom_client_queue, (uint32_t)dc_event_id, 0U);
    }
    else
    {
      custom_client_modem_is_attached = false;
      PRINT_APP("CustomClt: Network is DOWN\n\r")
    }
  }

  /* treat other dc_event_id */
  /*
  else if (dc_event_id == xxx)
  {
  }
  else
  {
    _NOP();
  }
  */
}

/**
  * @brief  CustomClient thread
  * @note   Infinite loop body
  * @param  p_argument - parameter osThread
  * @note   Unused parameter
  * @retval -
  */
static bool send_mudfile_link()
{
	int	mems_string_len;
	char mems_string[100];
	mems_string_len = snprintf(mems_string, sizeof(mems_string), "http://%s/%d", MUD_URL, MUD_DEVICE_ID);
	if ((logBuffer.data_len + mems_string_len) <= (sizeof(logBuffer.data)))
		{
			memcpy(&logBuffer.data[logBuffer.data_len], (const void *)mems_string, mems_string_len);
			logBuffer.data_len += mems_string_len;
		}
	do {
		if (custom_connect_and_send_data(logBuffer.data, logBuffer.data_len, 1))
		{
			PRINT_INFO("data port received")
		}
		else
		{
			const TickType_t delay = 5000;
			PRINT_INFO("Mud link failed. Trying again in %d milliseconds.", delay);
			vTaskDelay( delay );
		}
	} while (SERVER_LOG_PORT == -1 || SERVER_LOG_PORT == 65535);


	//custom_connect_and_send_data(logBuffer.data, logBuffer.data_len, 1);
	memset(logBuffer.data, 0, sizeof(logBuffer.data));
	logBuffer.data_len=0;
	return true;
}
static void custom_client_thread(void *p_argument)
{
  UNUSED(p_argument);

    /* Add below code of your application */
    /* Example : */
    /* Wait network is up to do something */
	uint32_t msg_queue = 0U;
	(void)rtosalMessageQueueGet(custom_client_queue, &msg_queue, RTOSAL_WAIT_FOREVER);

    /* Block for 5000ms. */
    const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
    send_mudfile_link();
    for( ;; )
    {
        // Perform action here: log the mems
        custom_log_mems();
        uint8_t cmd[] = "custclt send";
		custom_client_cmd(cmd);

        // Wait for the next cycle.
        vTaskDelay( xDelay );
    }


}

/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Initialization
  * @note   Custom Client initialization
  * @param  -
  * @retval -
  */
void custom_client_init(void)
{
  /* Custom Client static initialization */

  /* Example: */
  custom_client_modem_is_attached = false;

  /* CustomClient queue creation */
  custom_client_queue = rtosalMessageQueueNew(NULL, 1U);
  if (custom_client_queue == NULL)
  {
    ERROR_Handler(DBG_CHAN_CUSTOMCLIENT, 1, ERROR_FATAL);
  }
}

/**
  * @brief  Start
  * @note   Custom Client start
  * @param  -
  * @retval -
  */
void custom_client_start(void)
{
  static osThreadId CustomClient_TaskHandle;

  /* Cellular is now initialized
    Registration to other components is now possible */

  /* Registration to datacache */
  (void)dc_com_register_gen_event_cb(&dc_com_db, custom_client_notif_cb, (void *) NULL);

#if ((USE_CMD_CONSOLE == 1)  && (CUSTOM_CLIENT_CMD != 0U))
  CMD_Declare(custom_client_cmd_label, custom_client_cmd, (uint8_t *)"customclient commands");
#endif /* (USE_CMD_CONSOLE == 1) && (CUSTOM_CLIENT_CMD != 0U) */

  /* Create CustomClient thread  */
  CustomClient_TaskHandle = rtosalThreadNew((const rtosal_char_t *)"CustomCltThread", (os_pthread)custom_client_thread,
                                            CUSTOMCLIENT_THREAD_PRIO, CUSTOMCLIENT_THREAD_STACK_SIZE, NULL);

  if (CustomClient_TaskHandle == NULL)
  {
    ERROR_Handler(DBG_CHAN_CUSTOMCLIENT, 2, ERROR_FATAL);
  }
  else
  {
#if (USE_STACK_ANALYSIS == 1)
    (void)stackAnalysis_addStackSizeByHandle(CustomClient_TaskHandle, CUSTOMCLIENT_THREAD_STACK_SIZE);
#endif /* USE_STACK_ANALYSIS == 1 */
  }
}

#endif /* USE_CUSTOM_CLIENT == 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
