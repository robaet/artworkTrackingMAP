/**
  ******************************************************************************
  * @file    cellular_app_uiclient.c
  * @author  MCD Application Team
  * @brief   UIClt Cellular Application :
  *          - Manage UI (display)
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
#include "../Target/plf_config.h"

#if (USE_DISPLAY == 1)
#include <string.h>
#include <stdbool.h>

#if (USE_SENSORS ==1)
#include <math.h>
#endif /* USE_SENSORS ==1 */

#include "cellular_app_uiclient.h"

#include "cellular_app.h"
#include "cellular_app_trace.h"

#if (USE_RTC == 1)
#include "cellular_app_datetime.h"
#endif /* USE_RTC == 1 */

#include "cellular_app_display.h"
#if defined(APPLICATION_IMAGES_FILE)
#include APPLICATION_IMAGES_FILE
#endif /* defined(APPLICATION_IMAGES_FILE) */

#if (USE_SENSORS == 1)
#include "cellular_app_sensors.h"
#endif /* USE_SENSORS == 1 */

#include "cellular_control_api.h"

#include "rtosal.h"

/* Private typedef -----------------------------------------------------------*/
/* Timers are used to update periodically :
 * - the DateTime
 * - the Sensors value
 */
#if ((USE_RTC == 1) || (USE_SENSORS == 1))
typedef uint8_t cellular_app_uiclient_timer_state_t; /* Timer state type definition */
#define CELLULAR_APP_UICLIENT_TIMER_INVALID       (cellular_app_uiclient_timer_state_t)0
#define CELLULAR_APP_UICLIENT_TIMER_IDLE          (cellular_app_uiclient_timer_state_t)1
#define CELLULAR_APP_UICLIENT_TIMER_RUN           (cellular_app_uiclient_timer_state_t)2
#endif /*(USE_RTC == 1) || (USE_SENSORS == 1) */

/* Private defines -----------------------------------------------------------*/
typedef uint8_t uiclient_screen_state_t; /* Screen state type definition */
#define UICLIENT_SCREEN_OFF                       (uiclient_screen_state_t)0
#define UICLIENT_SCREEN_WELCOME                   (uiclient_screen_state_t)1
#define UICLIENT_SCREEN_CELLULAR_INFO             (uiclient_screen_state_t)2
#define UICLIENT_SCREEN_DATETIME_INFO             (uiclient_screen_state_t)3
#if (USE_SENSORS == 1)
#define UICLIENT_SCREEN_SENSORS_INFO              (uiclient_screen_state_t)4
#endif /* USE_SENSORS == 1 */

#define UICLIENT_STRING_LENGTH_MAX                (uint8_t)40
#define UICLIENT_STRING_TMP_LENGTH_MAX            (uint8_t)20
#define UICLIENT_STRING_SPACE_LENGTH_MAX          (uint8_t)10

#define UICLIENT_CELLULAR_MSG                      ((CELLULAR_APP_VALUE_MAX_MSG) + (cellular_app_msg_type_t)1)
#define UICLIENT_TIMER_MSG                         ((CELLULAR_APP_VALUE_MAX_MSG) + (cellular_app_msg_type_t)2)
/* MSG id when MSG is UICLIENT_CELLULAR_MSG */
/* MSG id is Cellular info                  */
#define UICLIENT_CELLULAR_INFO_CHANGE_ID           ((CELLULAR_APP_VALUE_MAX_ID) + (cellular_app_msg_id_t)1)
/* MSG id is DateTime info                  */
#define UICLIENT_CELLULAR_DATETIME_CHANGE_ID       ((CELLULAR_APP_VALUE_MAX_ID) + (cellular_app_msg_id_t)3)
/* MSG id when MSG is UICLIENT_TIMER_MSG    */
#if (USE_RTC == 1)
/* MSG id is DateTime increase 1s.          */
#define UICLIENT_DATETIME_READ_ID                  ((CELLULAR_APP_VALUE_MAX_ID) + (cellular_app_msg_id_t)4)
#endif /* USE_RTC == 1 */
#if (USE_SENSORS == 1)
/* MSG id is Sensors read                   */
#define UICLIENT_SENSORS_READ_ID                   ((CELLULAR_APP_VALUE_MAX_ID) + (cellular_app_msg_id_t)5)
#endif /* USE_SENSORS == 1 */

#if (USE_RTC == 1)
#define UICLIENT_DATETIME_READ_TIMER               (uint32_t)60000 /* Unit: in ms. - read every minute DateTime */
#endif /* USE_RTC == 1 */
#if (USE_SENSORS == 1)
#define UICLIENT_SENSORS_READ_TIMER                (uint32_t)5000  /* Unit: in ms. - read every 5s. the sensors */
#endif /* USE_SENSORS == 1 */

/* Private variables ---------------------------------------------------------*/
/* Trace shortcut */
static const uint8_t *p_cellular_app_uiclient_trace;

/* UIClt application descriptor */
static cellular_app_desc_t     cellular_app_uiclient;
/* UIClt screen state           */
static uiclient_screen_state_t cellular_app_uiclient_screen_state;

#if (DISPLAY_WAIT_MODEM_IS_ON == 1U)
static bool cellular_app_uiclient_modem_is_on;
#endif /* DISPLAY_WAIT_MODEM_IS_ON == 1U */
static bool cellular_app_uiclient_display_init_ok;

static uint8_t cellular_app_uiclient_string[UICLIENT_STRING_LENGTH_MAX];             /* to format a line to display */
static uint8_t cellular_app_uiclient_string_tmp[UICLIENT_STRING_TMP_LENGTH_MAX];     /* to format temporary string  */
static uint8_t cellular_app_uiclient_string_space[UICLIENT_STRING_SPACE_LENGTH_MAX]; /* to add space in a string    */

#if (USE_RTC == 1)
/* Timer to read periodically the DateTime see UICLIENT_DATETIME_READ_TIMER for the period */
static osTimerId cellular_app_uiclient_datetime_timer_id;
static cellular_app_uiclient_timer_state_t cellular_app_uiclient_datetime_timer_state;
#endif /* USE_RTC == 1 */

#if (USE_SENSORS == 1)
/* Timer to read periodically the Sensors see UICLIENT_SENSORS_READ_TIMER for the period */
static osTimerId cellular_app_uiclient_sensors_timer_id;
static cellular_app_uiclient_timer_state_t cellular_app_uiclient_sensors_timer_state;
#endif /* USE_SENSORS == 1 */

static cellular_info_t cellular_app_uiclient_cellular_info; /* To read cellular information */

/* Private macro -------------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/

/* Private functions prototypes ----------------------------------------------*/
/* BEGIN callbacks declaration */
/* Callback called by Cellular when a cellular value changed */
static void cellular_app_uiclient_cellular_info_status_cb(ca_event_type_t event_type,
                                                          const cellular_info_t *const p_cellular_info,
                                                          void *const p_callback_ctx);

/* Callback called by Cellular when a signal value changed */
static void cellular_app_uiclient_signal_info_status_cb(ca_event_type_t event_type,
                                                        const cellular_signal_info_t *const p_sigal_info,
                                                        void *const p_callback_ctx);

#if (USE_RTC == 1)
/* CCallback called when DateTime is set */
static void cellular_app_uiclient_datetime_set_cb(void const *p_argument);
/* Callback called when DateTime Timer is raised */
static void cellular_app_uiclient_datetime_timer_cb(void *p_argument);
#endif /* USE_RTC == 1*/

#if (USE_SENSORS == 1)
/* Callback called when Sensors Timer is raised */
static void cellular_app_uiclient_sensors_timer_cb(void *p_argument);
#endif /* USE_SENSORS == 1 */
/* END   callbacks declaration */

/* Format a line taking into account number of characters available on a LCD line */
static void uiclient_format_line(uint8_t *p_string1, uint32_t length1, uint8_t *p_string2, uint32_t length2,
                                 uint8_t *p_string_res);
/* Update display and status according to welcome screen */
static bool uiclient_update_welcome(void);
/* Update display and status according to new cellular info received */
static bool uiclient_update_cellular_info(void);
/* Update display and status according to new cellular ip info received */
static bool uiclient_update_cellular_ip_info(void);

#if (USE_RTC == 1)
/* Update display and status according to new date time info received */
static bool uiclient_update_datetime_info(void);
#endif /* USE_RTC == 1 */

#if (USE_SENSORS == 1)
/* Update display and status according to new sensors info read */
static bool uiclient_update_sensors_info(void);
#endif /* USE_SENSORS == 1 */

/* UIClt thread */
static void uiclient_thread(void *p_argument);

/* Public  functions  prototypes ---------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Callback called when a value in cellular changed
  * @param  event_type      - Event that happened: CA_CELLULAR_INFO_EVENT.
  * @param  p_cellular_info - The new cellular information.
  * @param  p_callback_ctx  - The p_callback_ctx parameter in cellular_info_changed_registration function.
  * @retval -
  */
static void cellular_app_uiclient_cellular_info_status_cb(ca_event_type_t event_type,
                                                          const cellular_info_t *const p_cellular_info,
                                                          void *const p_callback_ctx)
{
  UNUSED(p_callback_ctx);
  cellular_app_msg_type_t msg_type = UICLIENT_CELLULAR_MSG;
  cellular_app_msg_id_t   msg_id   = UICLIENT_CELLULAR_INFO_CHANGE_ID;
  cellular_app_msg_data_t msg_data = 0U;
  uint32_t msg = 0U;
  rtosalStatus status; /* send message status */

  /* Event to know Modem state ? */
  if ((event_type == CA_CELLULAR_INFO_EVENT) && (p_cellular_info != NULL))
  {
    SET_CELLULAR_APP_MSG_TYPE(msg, msg_type);
    SET_CELLULAR_APP_MSG_ID(msg, msg_id);
    SET_CELLULAR_APP_MSG_DATA(msg, msg_data);
    /* Send the message */
    status = rtosalMessageQueuePut(cellular_app_uiclient.queue_id, msg, 0U);
    if (status != osOK)
    {
      /* Message send failed */
      PRINT_FORCE("%s: ERROR CellularInfo MsgPut Type:%d Id:%d Data:%d- status:%d!", p_cellular_app_uiclient_trace,
                  msg_type, msg_id, msg_data, status)
    }
  }
}

/**
  * @brief  Callback called by Cellular when a signal value changed
  * @param  event_type     - Event that happened: CA_CELLULAR_SIGNAL_INFO_EVENT.
  * @param  p_signal_info  - The new signal information.
  * @param  p_callback_ctx - The p_callback_ctx parameter in cellular_signal_info_changed_registration function.
  * @retval -
  */
static void cellular_app_uiclient_signal_info_status_cb(ca_event_type_t event_type,
                                                        const cellular_signal_info_t *const p_signal_info,
                                                        void *const p_callback_ctx)
{
  UNUSED(p_callback_ctx);
  cellular_app_msg_type_t msg_type = UICLIENT_CELLULAR_MSG;
  cellular_app_msg_id_t   msg_id   = UICLIENT_CELLULAR_INFO_CHANGE_ID;
  cellular_app_msg_data_t msg_data = 0U;
  uint32_t msg = 0U;
  rtosalStatus status; /* send message status */

  /* Event to know Modem state ? */
  if ((event_type == CA_SIGNAL_INFO_EVENT) && (p_signal_info != NULL))
  {
    SET_CELLULAR_APP_MSG_TYPE(msg, msg_type);
    SET_CELLULAR_APP_MSG_ID(msg, msg_id);
    SET_CELLULAR_APP_MSG_DATA(msg, msg_data);
    /* Send the message */
    status = rtosalMessageQueuePut(cellular_app_uiclient.queue_id, msg, 0U);
    if (status != osOK)
    {
      /* Message send failed */
      PRINT_FORCE("%s: ERROR SignalInfo MsgPut Type:%d Id:%d data:%d- status:%d!", p_cellular_app_uiclient_trace,
                  msg_type, msg_id, msg_data, status)
    }
  }
}

#if (USE_RTC == 1)
/**
  * @brief  Callback called when DateTime is set
  * @param  p_argument - UNUSED
  * @retval -
  */
static void cellular_app_uiclient_datetime_set_cb(void const *p_argument)
{
  UNUSED(p_argument);
  cellular_app_msg_type_t msg_type = UICLIENT_CELLULAR_MSG;
  cellular_app_msg_id_t   msg_id   = UICLIENT_CELLULAR_DATETIME_CHANGE_ID;
  cellular_app_msg_data_t msg_data = 0U;
  uint32_t msg = 0U;
  rtosalStatus status; /* send message status */

  SET_CELLULAR_APP_MSG_TYPE(msg, msg_type);
  SET_CELLULAR_APP_MSG_ID(msg, msg_id);
  SET_CELLULAR_APP_MSG_DATA(msg, msg_data);
  /* Send the message */
  status = rtosalMessageQueuePut(cellular_app_uiclient.queue_id, msg, 0U);
  if (status != osOK)
  {
    /* Message send failed */
    PRINT_FORCE("%s: ERROR DateTime MsgPut Type:%d Id:%d Data:%d - status:%d!", p_cellular_app_uiclient_trace,
                msg_type, msg_id, msg_data, status)
  }
}

/**
  * @brief  Callback called when DateTime Timer is raised
  * @param  p_argument - UNUSED
  * @retval -
  */
static void cellular_app_uiclient_datetime_timer_cb(void *p_argument)
{
  UNUSED(p_argument);
  rtosalStatus status; /* send message status */
  cellular_app_msg_type_t msg_type = UICLIENT_TIMER_MSG;
  cellular_app_msg_id_t   msg_id   = UICLIENT_DATETIME_READ_ID;
  cellular_app_msg_data_t msg_data = 0U;
  uint32_t msg = 0U;

  if (cellular_app_uiclient_datetime_timer_state == CELLULAR_APP_UICLIENT_TIMER_RUN)
  {
    SET_CELLULAR_APP_MSG_TYPE(msg, msg_type);
    SET_CELLULAR_APP_MSG_ID(msg, msg_id);
    SET_CELLULAR_APP_MSG_DATA(msg, msg_data);
    /* Send the message */
    status = rtosalMessageQueuePut(cellular_app_uiclient.queue_id, msg, 0U);
    if (status != osOK)
    {
      /* Message send failed */
      PRINT_FORCE("%s: ERROR DateTime MsgPut Type:%d Id:%d Data:%d - status:%d!r", p_cellular_app_uiclient_trace,
                  msg_type, msg_id, msg_data, status)
    }
  }
}
#endif /* USE_RTC == 1 */

#if (USE_SENSORS == 1)
/**
  * @brief  Callback called when Sensors Timer is raised
  * @param  p_argument - UNUSED
  * @retval -
  */
static void cellular_app_uiclient_sensors_timer_cb(void *p_argument)
{
  UNUSED(p_argument);
  rtosalStatus status; /* send message status */
  cellular_app_msg_type_t msg_type = UICLIENT_TIMER_MSG;
  cellular_app_msg_id_t   msg_id   = UICLIENT_SENSORS_READ_ID;
  cellular_app_msg_data_t msg_data = 0U;
  uint32_t msg = 0U;

  if (cellular_app_uiclient_sensors_timer_state == CELLULAR_APP_UICLIENT_TIMER_RUN)
  {
    SET_CELLULAR_APP_MSG_TYPE(msg, msg_type);
    SET_CELLULAR_APP_MSG_ID(msg, msg_id);
    SET_CELLULAR_APP_MSG_DATA(msg, msg_data);
    /* Send the message */
    status = rtosalMessageQueuePut(cellular_app_uiclient.queue_id, msg, 0U);
    if (status != osOK)
    {
      /* Message send failed */
      PRINT_FORCE("%s: ERROR Sensors MsgPut Type:%d Id:%d Data:%d - status:%d!", p_cellular_app_uiclient_trace,
                  msg_type, msg_id, msg_data, status)
    }
  }
}
#endif /* USE_SENSORS == 1 */

/**
  * @brief  Format a line taking into account number of characters available on a LCD line
  *         Spaces are added between string1 and string2 to have string2 end always at last possible position on LCD
  * @param  p_string1    - string to display at left
  * @param  length1      - string1 length
  * @param  p_string2    - string to display at right
  * @param  length2      - string2 length
  * @param  p_string_res - string result to display
  * @retval -
  */
static void uiclient_format_line(uint8_t *p_string1, uint32_t length1, uint8_t *p_string2, uint32_t length2,
                                 uint8_t *p_string_res)
{
  uint32_t  nb_spaces;   /* number of spaces to add */
  uint32_t  length1_tmp; /* number of characters string1 */
  uint32_t  length2_tmp; /* number of characters string2 */
  /* Get number of maximum possible characters on LCD line with the actual Font */
  uint32_t nb_character = cellular_app_display_characters_per_line();

  if (nb_character > 0U)
  {
    /* 'string1' on left, then spaces, 'string2' on right */
    /* Some spaces to add between string1 and string2 ? */
    if ((length1 + length2) < nb_character) /* at minimum one space between the two strings */
    {
      /* It is possible to display string1 completely */
      length1_tmp = length1;
      /* Calculate the number of spaces between string1 and string2 */
      nb_spaces   = nb_character - (length1 + length2);
      /* It is possible to display string2 completely */
      length2_tmp = length2;
    }
    else /* Too many characters to display */
    {
      /* Ensure maximum number of characters for string1 */
      length1_tmp = CELLULAR_APP_MIN(length1, nb_character);
      /* If possible and if not already available add one space */
      /* static function length1 > 0 and p_string2 != NULL */
      if ((p_string1[length1_tmp - 1U] != (uint8_t)' ') && (p_string2[0] != (uint8_t)' '))
      {
        nb_spaces = (length1_tmp != nb_character) ? 1U : 0U;
      }
      else
      {
        nb_spaces = 0U;
      }
      /* Rest for string2 */
      length2_tmp = ((length1_tmp + nb_spaces) < nb_character) ? (nb_character - length1_tmp - nb_spaces) : 0U;
    }
    /* 'string1' on left, then spaces, 'string2' on right */
    (void)sprintf((CRC_CHAR_t *)p_string_res, "%.*s%.*s%.*s", (int16_t)length1_tmp, p_string1,
                  (int16_t)nb_spaces, cellular_app_uiclient_string_space, (int16_t)length2_tmp, p_string2);
  }
  /* nb_character == 0U (should not happen) - No string can be displayed */
}

/**
  * @brief  Update display and status according to welcome screen
  * @retval true/false - refresh has to be done or not
  */
static bool uiclient_update_welcome(void)
{
  bool result = false; /* false: refresh has not to be done, true: refresh has to be done */

  /* Display welcome screen only if UI comes from screen off */
  if (cellular_app_uiclient_screen_state == UICLIENT_SCREEN_OFF)
  {
    /* Board display init to be done only if modem is on ? */
#if (DISPLAY_WAIT_MODEM_IS_ON == 1U)
    if (cellular_app_uiclient_modem_is_on == true)
    {
#endif /* DISPLAY_WAIT_MODEM_IS_ON == 1U */
      cellular_app_uiclient_display_init_ok = cellular_app_display_init();
#if (DISPLAY_WAIT_MODEM_IS_ON == 1U)
    }
#endif /* DISPLAY_WAIT_MODEM_IS_ON == 1U */

    if (cellular_app_uiclient_display_init_ok == true)
    {
      /* Display a welcome image if defined */
#if defined(APPLICATION_IMAGES_FILE)
#if defined(CELLULAR_APP_BMP_WELCOME_SIZE)
      uint32_t XPos = 0U;
      uint32_t YPos = 0U;
      if (CELLULAR_APP_BMP_WELCOME_HEIGHT < cellular_app_display_get_YSize())
      {
        YPos = (cellular_app_display_get_YSize() - CELLULAR_APP_BMP_WELCOME_HEIGHT) / 2U;
      }
      if (CELLULAR_APP_BMP_WELCOME_WIDTH < cellular_app_display_get_XSize())
      {
        XPos = (cellular_app_display_get_XSize() - CELLULAR_APP_BMP_WELCOME_WIDTH) / 2U;
      }
      cellular_app_display_set_BackColor(LCD_COLOR_WHITE);
      cellular_app_display_draw_Bitmap((uint16_t)XPos, (uint16_t)YPos, (uint8_t *)cellular_app_bmp_welcome);
      /* Update screen state */
#else
      cellular_app_display_set_BackColor(LCD_COLOR_BLACK);
#endif /* defined(CELLULAR_APP_BMP_WELCOME_SIZE) */
#endif /* defined(APPLICATION_IMAGES_FILE) */
      /* Update screen state */
      cellular_app_uiclient_screen_state = UICLIENT_SCREEN_WELCOME;
      result = true; /* refresh has to be done */
    }
  }

  return (result);
}

/**
  * @brief  Update display and status according to new cellular info received
  * @retval bool         - false/true - refresh has not to do/refresh has to be done
  */
static bool uiclient_update_cellular_info(void)
{
  bool result = false; /* false: refresh has not to be done, true: refresh has to be done */
  uint8_t  operator_name_start; /* To ensure to suppress potential '"' at begin of operator name string */
  uint32_t line = 0U; /* start at first line of LCD */
  uint32_t string1_length; /* p_string1 length */
  uint32_t string2_length; /* p_string2 length */
  uint32_t nb_character; /* number of characters in a line */
  uint8_t  *p_string1;
  uint8_t  *p_string2;
  /* SIM string state */
#define UICLIENT_SIM_STRING_LENGTH       7U /* rather than to calculate length sim_string */
  static uint8_t sim_string[4][UICLIENT_SIM_STRING_LENGTH] =
  {
    ("SIM    "), \
    ("SIMCard"), \
    ("SIMSold"), \
    ("SIM Nok")
  };

  /* Cellular/Modem string state */
#define UICLIENT_STATE_STRING_LENGTH    9U /* rather than to calculate length state_string */
  static uint8_t state_string[10][UICLIENT_STATE_STRING_LENGTH] =
  {
    ("  Unknown"), \
    ("     Init"), \
    ("SimConctd"), \
    ("NwkSearch"), \
    ("NwkAttach"), \
    ("DataReady"), \
    ("   Flight"), \
    ("   Reboot"), \
    (" Updating"), \
    (" ModemOff")
  };

  /* Operator string state */
#define UICLIENT_OPERATOR_STRING_LENGTH  10U /* rather than to calculate length operator_string */
  static uint8_t operator_string[1][UICLIENT_OPERATOR_STRING_LENGTH] =
  {
    ("Operator ?")
    /* other value is: <operator name value> */
  };

  /* ICCID string state */
#define UICLIENT_ICCID_STRING_LENGTH     5U /* rather than to calculate length iccid_string */
  static uint8_t iccid_string[1][UICLIENT_ICCID_STRING_LENGTH] =
  {
    ("ICCID")
    /* other value is: ICCID <ICCID value> */
  };

  cellular_sim_info_t sim_info;       /* To read SIM information      */
  cellular_signal_info_t signal_info; /* To read Signal information   */

  /* Read Cellular information to know cellular/modem state */
  cellular_get_cellular_info(&cellular_app_uiclient_cellular_info);

  /* Is display already initialized ? */
  if (cellular_app_uiclient_display_init_ok == false)
  {
    /* Must wait modem is power on before to initialize the display ? */
#if (DISPLAY_WAIT_MODEM_IS_ON == 1U)
    /* Must wait modem is power on before to initialize the display - Is modem power on ? */
    if (cellular_app_uiclient_cellular_info.modem_state != CA_MODEM_POWER_OFF)
    {
      cellular_app_uiclient_modem_is_on = true; /* update modem is on status */
#endif /* DISPLAY_WAIT_MODEM_IS_ON == 1U */
      /* Initialize display */
      cellular_app_uiclient_display_init_ok = cellular_app_display_init();
      /* If must wait modem is power on before to initialize the display
       * then if modem is OFF update display and modem status */
#if (DISPLAY_WAIT_MODEM_IS_ON == 1U)
    }
    else /* Modem is OFF */
    {
      /* Update display status */
      cellular_app_uiclient_display_init_ok = false;
      /* Update modem status */
      cellular_app_uiclient_modem_is_on = false;
    }
#endif /* DISPLAY_WAIT_MODEM_IS_ON == 1U */
  }

  if (cellular_app_uiclient_display_init_ok == true)
  {
    /* Display welcome screen ? */
    if (cellular_app_uiclient_screen_state == UICLIENT_SCREEN_OFF)
    {
      /* Display a welcome bitmap */
      result = uiclient_update_welcome();
    }
    else
    {
      /* If comes from Screen Welcome - Erase welcome screen */
      if (cellular_app_uiclient_screen_state == UICLIENT_SCREEN_WELCOME)
      {
        /* Exit from Welcome screen only if something about modem has to be displayed */
        if ((cellular_app_uiclient_cellular_info.modem_state != CA_MODEM_POWER_OFF)
            && (cellular_app_uiclient_cellular_info.identity.manufacturer_id.len != 0U))
        {
          cellular_app_display_set_BackColor(LCD_COLOR_BLACK);
          cellular_app_display_set_TextColor(LCD_COLOR_WHITE);
          cellular_app_display_clear(LCD_COLOR_BLACK);
          /* And go to Screen Cellular Info */
          cellular_app_uiclient_screen_state = UICLIENT_SCREEN_CELLULAR_INFO;
#if (USE_SENSORS == 1)
          /* Now that cellular info is displayed, start Sensors Timer */
          if (cellular_app_uiclient_sensors_timer_state == CELLULAR_APP_UICLIENT_TIMER_IDLE)
          {
            if (rtosalTimerStart(cellular_app_uiclient_sensors_timer_id, UICLIENT_SENSORS_READ_TIMER) == osOK)
            {
              cellular_app_uiclient_sensors_timer_state = CELLULAR_APP_UICLIENT_TIMER_RUN;
            }
          }
#endif /* USE_SENSORS == 1 */
        }
      }

      /* Update screen only if screen is Cellular Info */
      if (cellular_app_uiclient_screen_state == UICLIENT_SCREEN_CELLULAR_INFO)
      {
        /* Set font to default font */
        cellular_app_display_font_set(0U);
        /* Get the maximum number of characters per line */
        nb_character = cellular_app_display_characters_per_line();

        /* Initialize strings */
        (void)memset(cellular_app_uiclient_string, 0, UICLIENT_STRING_LENGTH_MAX);

#if (USE_RTC == 1) /* Is DateTime known ? */
        /* Next line is string1:'DateTime' string2:''*/
        (void)uiclient_update_datetime_info();
#endif /* USE_RTC == 1 */
        /* Even if DateTime is not displayed, go to next line taking into account Font height */
        line += cellular_app_display_font_get_height();

        /* Next line is: string1:'SIM' and string2:'Cellular/Modem status' */
        /* Read SIM information to format string1 */
        cellular_get_sim_info(&sim_info);
        switch (sim_info.sim_status)
        {
          case CA_SIM_READY:
            /* ToDo: Make the difference between SIM and eSIM */
            switch (sim_info.sim_slot_type)
            {
              case CA_SIM_REMOVABLE_SLOT :
                p_string1 = &sim_string[1][0];
                break;
              case CA_SIM_EXTERNAL_MODEM_SLOT :
              case CA_SIM_INTERNAL_MODEM_SLOT :
                p_string1 = &sim_string[2][0];
                break;
              default :
                p_string1 = &sim_string[0][0];
                break;
            }
            break;

          case CA_SIM_STATUS_UNKNOWN:
          case CA_SIM_CONNECTION_ONGOING:
            p_string1 = &sim_string[0][0]; /* "SIM    " */
            p_string1[4] = (uint8_t)'?';   /* "SIM ?  " */
            break;
          case CA_SIM_PIN_OR_PUK_LOCKED:
          case CA_SIM_INCORRECT_PIN:
          case CA_SIM_BUSY:
          case CA_SIM_ERROR:
          case CA_SIM_NOT_INSERTED:
          case CA_SIM_NOT_USED:
          case CA_SIM_NOT_IMPLEMENTED:
          default:
            p_string1 = &sim_string[3][0];
            break;
        }
        /* Update string1 length */
        string1_length = UICLIENT_SIM_STRING_LENGTH;

        /* Check modem state to format string2 */
        switch (cellular_app_uiclient_cellular_info.modem_state)
        {
          case CA_MODEM_STATE_POWERED_ON:
            p_string2 = &state_string[1][0];
            break;
          case CA_MODEM_STATE_SIM_CONNECTED:
            p_string2 = &state_string[2][0];
            break;
          case CA_MODEM_NETWORK_SEARCHING:
            p_string2 = &state_string[3][0];
            break;
          case CA_MODEM_NETWORK_REGISTERED:
            p_string2 = &state_string[4][0];
            break;
          case CA_MODEM_STATE_DATAREADY:
            p_string2 = &state_string[5][0];
            break;
          case CA_MODEM_IN_FLIGHTMODE:
            p_string2 = &state_string[6][0];
            break;
          case CA_MODEM_REBOOTING:
            p_string2 = &state_string[7][0];
            break;
          case CA_MODEM_FOTA_INPROGRESS:
            p_string2 = &state_string[8][0];
            break;
          case CA_MODEM_POWER_OFF:
            p_string2 = &state_string[9][0];
            break;
          default:
            p_string2 = &state_string[0][0];
            PRINT_INFO("%s: Modem state Unknown: %d!!!", p_cellular_app_uiclient_trace,
                       cellular_app_uiclient_cellular_info.modem_state)
            break;
        }
        /* Update string2 length */
        string2_length = UICLIENT_STATE_STRING_LENGTH;

        /* Format the line */
        uiclient_format_line(p_string1, string1_length, p_string2, string2_length,
                             (uint8_t *)&cellular_app_uiclient_string);
        cellular_app_display_string(1U, (uint16_t)line, cellular_app_uiclient_string);
        /* Go to next line taking into account Font height */
        line += cellular_app_display_font_get_height();

        /* Next line is: string1:'Operator name' and string2:'Signal level' */
        /* Update string1 length with operator name length */
        string1_length = cellular_app_uiclient_cellular_info.mno_name.len;
        if (string1_length > 0U)
        {
          /* Suppress potential '"' at begin and end of mobile network operator name */
          if (cellular_app_uiclient_cellular_info.mno_name.value[0] == (uint8_t)'"')
          {
            operator_name_start = 1U;
            if ((string1_length > 1U)
                && (cellular_app_uiclient_cellular_info.mno_name.value[string1_length - 1U] == (uint8_t)'"'))
            {
              string1_length = string1_length - 2U; /* by-pass '"' at string begin and end */
            }
            else
            {
              string1_length = string1_length - 1U; /* by-pass '"' at string begin */
            }
          }
          else
          {
            operator_name_start = 0U;
          }
          /* Point on mobile network mobile operator name */
          p_string1 = &cellular_app_uiclient_cellular_info.mno_name.value[operator_name_start];
        }
        else
        {
          /* Point on mobile network mobile operator unknown*/
          p_string1 = &operator_string[0][0];
          string1_length = UICLIENT_OPERATOR_STRING_LENGTH;
        }

        /* Read Cellular signal information to format string2 */
        cellular_get_signal_info(&signal_info);

        (void)sprintf((CRC_CHAR_t *)&cellular_app_uiclient_string_tmp[0], "%3lddB",
                      signal_info.signal_strength.db_value);
        p_string2 = &cellular_app_uiclient_string_tmp[0];
        string2_length = 5U;

        /* Check enough space to write string2 */
        if (nb_character > string2_length)
        {
          /* Format the line to ensure string2 is displayed */
          uiclient_format_line(p_string1, CELLULAR_APP_MIN(string1_length, (nb_character - string2_length - 1U)),
                               p_string2, string2_length, (uint8_t *)&cellular_app_uiclient_string);
        }
        else
        {
          /* Impossible case : format the line with what we have */
          uiclient_format_line(p_string1, string1_length,
                               p_string2, string2_length, (uint8_t *)&cellular_app_uiclient_string);
        }

        cellular_app_display_string(1U, (uint16_t)line, cellular_app_uiclient_string);
        /* Go to next line taking into account Font height */
        line += cellular_app_display_font_get_height();

        /* Continue cellular info screen with cellular ip info */
        (void)uiclient_update_cellular_ip_info();
        /* Go to next line taking into account Font height */
        line += cellular_app_display_font_get_height();

        /* Next line is: string1:'ICCID' */
        /* ICCID is displayed with reduced Font */
        cellular_app_display_font_decrease();
        if ((sim_info.sim_status == CA_SIM_READY) && (sim_info.iccid.len != 0U))
        {
          /* ICCID is known */
          p_string1 = &iccid_string[0][0];
          string1_length = UICLIENT_ICCID_STRING_LENGTH;
          p_string2 = &sim_info.iccid.value[0];
          string2_length = sim_info.iccid.len;
          /* Format the line */
          uiclient_format_line(p_string1, string1_length, p_string2, string2_length,
                               (uint8_t *)&cellular_app_uiclient_string);
        }
        else
        {
          /* ICCID is unknown */
          (void)sprintf((CRC_CHAR_t *)cellular_app_uiclient_string, "%.*s ?",
                        UICLIENT_ICCID_STRING_LENGTH, &iccid_string[0][0]);
        }
        cellular_app_display_string(1U, (uint16_t)line, cellular_app_uiclient_string);
        /* Restore font to default font */
        cellular_app_display_font_set(0U);

#if (USE_SENSORS == 1)
        /* Continue cellular info screen with sensors info */
        (void)uiclient_update_sensors_info();
#endif /* USE_SENSORS == 1 */

        result = true; /* refresh has to be done */
      }
    }
  }

  return (result);
}

/**
  * @brief  Update display and status according to new cellular ip info received
  * @retval bool         - false/true - refresh has not to be done/refresh has to be done
  */
static bool uiclient_update_cellular_ip_info(void)
{
  bool result = false; /* false: refresh has not to be done, true: refresh has to be done */
  uint16_t line;
  uint8_t *p_string1;
  uint8_t *p_string2;
#define UICLIENT_IP_STRING_LENGTH     2U /* rather than to calculate length ip_string */
  static uint8_t ip_string[1][UICLIENT_IP_STRING_LENGTH] =
  {
    ("IP")
    /* other value is: IP <IP value> */
  };

  /* Update screen only if display is initialized and screen is Cellular info */
  if ((cellular_app_uiclient_display_init_ok == true)
      && (cellular_app_uiclient_screen_state == UICLIENT_SCREEN_CELLULAR_INFO))
  {
    /* Next line is: string1:'IP header' and string2:'IP address if known' */

    /* Read Cellular information to format string1 */
    cellular_get_cellular_info(&cellular_app_uiclient_cellular_info);

    p_string1 = &ip_string[0][0];
    /* Is IP known ? */
    if (cellular_app_uiclient_cellular_info.ip_addr.addr != 0U)
    {
      /* IP is known */
      (void)sprintf((CRC_CHAR_t *)&cellular_app_uiclient_string_tmp[0], "%u.%u.%u.%u",
                    COM_IP4_ADDR1_VAL(cellular_app_uiclient_cellular_info.ip_addr),
                    COM_IP4_ADDR2_VAL(cellular_app_uiclient_cellular_info.ip_addr),
                    COM_IP4_ADDR3_VAL(cellular_app_uiclient_cellular_info.ip_addr),
                    COM_IP4_ADDR4_VAL(cellular_app_uiclient_cellular_info.ip_addr));
      p_string2 = &cellular_app_uiclient_string_tmp[0];
      /* Format the line */
      uiclient_format_line(p_string1, UICLIENT_IP_STRING_LENGTH, p_string2, crs_strlen((const uint8_t *)p_string2),
                           (uint8_t *)&cellular_app_uiclient_string);
    }
    else
    {
      /* IP is unknown */
      (void)sprintf((CRC_CHAR_t *)cellular_app_uiclient_string, "%.*s ?", UICLIENT_IP_STRING_LENGTH, &ip_string[0][0]);
    }
    /* Go to the IP line on LCD */
    line = 3U * (uint16_t)cellular_app_display_font_get_height();
    cellular_app_display_string(1U, line, cellular_app_uiclient_string);

    result = true; /* refresh has to be done */
  }

  return (result);
}

#if (USE_RTC == 1)
/**
  * @brief  Update display and status according to new date time info received
  * @retval bool - false/true - refresh has not to be done/refresh has to be done
  */
static bool uiclient_update_datetime_info(void)
{
  bool result = false; /* false: refresh has not to be done, true: refresh has to be done */
  uint32_t nb_character; /* number of characters in a line */
  cellular_app_datetime_t datetime; /* To read DateTime information */

  /* Update screen only if display is initialized and screen is Cellular info */
  if ((cellular_app_uiclient_display_init_ok == true)
      && (cellular_app_uiclient_screen_state == UICLIENT_SCREEN_CELLULAR_INFO))
  {
    /* Next line is: hh:mm yyyy/mm/dd */
    /* Set font to default font */
    cellular_app_display_font_set(0U);
    /* Get the maximum number of characters per line */
    nb_character = cellular_app_display_characters_per_line();

    if (nb_character >= 16U)
    {
      /* Next line is: Time Date */
      /* Is DateTime known ? */
      if (cellular_app_datetime_get(&datetime) == true)
      {
        /* DateTime is known */
        (void)sprintf((CRC_CHAR_t *)cellular_app_uiclient_string, "%02u:%02u %.*s%04u/%02u/%02u",
                      datetime.time.hour, datetime.time.min,
                      (int16_t)(nb_character - 16U), cellular_app_uiclient_string_space,
                      ((uint16_t)datetime.date.year + datetime.date.year_start),
                      datetime.date.month, datetime.date.month_day);
      }
      else
      {
        /* DateTime is unknown */
        (void)sprintf((CRC_CHAR_t *)cellular_app_uiclient_string, "--:-- %.*s----/--/--",
                      (int16_t)(nb_character - 16U), cellular_app_uiclient_string_space);
      }
      /* DateTime line is first line: 0U */
      cellular_app_display_string(1U, 0U, cellular_app_uiclient_string);

      result = true; /* refresh has to be done */
    }
    /* else nb_character < 16U - display date and time should be reviewed */
  }

  return (result);
}
#endif /* USE_RTC == 1 */

#if (USE_SENSORS ==1)
/**
  * @brief  Update display and status according to new sensors info read
  * @retval bool         - false/true - refresh has not to be done/refresh has to be done
  */
static bool uiclient_update_sensors_info(void)
{
  bool result = false; /* false: refresh has not to be done, true: refresh has to be done */
  uint16_t line;
  cellular_app_sensors_data_t sensor_humidity;    /* To read Humidity sensor information    */
  cellular_app_sensors_data_t sensor_pressure;    /* To read Pressure sensor information    */
  cellular_app_sensors_data_t sensor_temperature; /* To read Temperature sensor information */

  /* Update screen only if display is initialized and screen is Cellular info */
  if ((cellular_app_uiclient_display_init_ok == true)
      && (cellular_app_uiclient_screen_state == UICLIENT_SCREEN_CELLULAR_INFO))
  {
    /* Next line is: 'T:''Temperature value''C' 'H:''Humidity value' 'P:''Pressure value''P' */
    /* Get Humidity sensor value */
    if (cellular_app_sensors_read(CELLULAR_APP_SENSOR_TYPE_HUMIDITY, &sensor_humidity) == false)
    {
      sensor_humidity.float_data = (float_t)0;
    }
    /* Get Pressure sensor value */
    if (cellular_app_sensors_read(CELLULAR_APP_SENSOR_TYPE_PRESSURE, &sensor_pressure) == false)
    {
      sensor_pressure.float_data = (float_t)0;
    }
    /* Get Temperature sensor value */
    if (cellular_app_sensors_read(CELLULAR_APP_SENSOR_TYPE_TEMPERATURE, &sensor_temperature) == false)
    {
      sensor_temperature.float_data = (float_t)0;
    }

    /* Format the line */
    (void)sprintf((CRC_CHAR_t *)cellular_app_uiclient_string, "T:%4.1fC H:%4.1f P:%6.1fP",
                  sensor_temperature.float_data, sensor_humidity.float_data, sensor_pressure.float_data);

    /* Go to Sensors line: DateTime, SIM, Operator, IP */
    line = 4U * (uint16_t)cellular_app_display_font_get_height();

    /* ICCID is displayed with reduced Font */
    cellular_app_display_font_decrease();
    line += (uint16_t)cellular_app_display_font_get_height();

    /* Display sensors info */
    /* Too many information to display on same line: stay in reduced Font */
    cellular_app_display_string(1U, line, cellular_app_uiclient_string);

    /* Restore font to default font */
    cellular_app_display_font_set(0U);

    result = true; /* refresh has to be done */
  }

  return (result);
}
#endif /* USE_SENSORS == 1 */

/**
  * @brief  Update information according to new one received
  * @param  info - information received
  * @retval -
  */
static void uiclient_update_info(uiclient_screen_state_t info)
{
  bool refresh_to_do = false; /* false: refresh has not to be done, true: refresh has to be done */

  switch (info)
  {
    case UICLIENT_SCREEN_WELCOME :
      refresh_to_do = uiclient_update_welcome();
      break;
    case UICLIENT_SCREEN_CELLULAR_INFO :
      refresh_to_do = uiclient_update_cellular_info();
      break;
    case UICLIENT_SCREEN_DATETIME_INFO :
#if (USE_RTC == 1)
      refresh_to_do = uiclient_update_datetime_info();
#endif /* USE_RTC == 1 */
      break;
#if (USE_SENSORS == 1)
    case UICLIENT_SCREEN_SENSORS_INFO :
      refresh_to_do = uiclient_update_sensors_info();
      break;
#endif /* USE_SENSORS == 1 */
    default:
      __NOP(); /* because refresh_to_do already set to false */
      break;
  }

  if (refresh_to_do == true)
  {
    /* Refresh Display */
    cellular_app_display_refresh();
  }
}
/**
  * @brief  UIClt thread
  * @note   Infinite loop UIClt body
  * @param  p_argument - unused
  * @retval -
  */
static void uiclient_thread(void *p_argument)
{
  UNUSED(p_argument);

  uint32_t msg;
  cellular_app_msg_type_t msg_type;  /* Msg type received from the queue */
  cellular_app_msg_id_t   msg_id;    /* Msg id received from the queue   */

  /* Display Screen Welcome as soon as possible */
  uiclient_update_info(UICLIENT_SCREEN_WELCOME);

  for (;;)
  {
    msg = 0U; /* Re-initialize msg_queue to impossible value */
    /* Wait a notification to do something */
    (void)rtosalMessageQueueGet(cellular_app_uiclient.queue_id, &msg, RTOSAL_WAIT_FOREVER);
    /* Analyze message */
    if (msg != 0U)
    {
      msg_type = GET_CELLULAR_APP_MSG_TYPE(msg);
      msg_id   = GET_CELLULAR_APP_MSG_ID(msg);

      switch (msg_type)
      {
        case UICLIENT_CELLULAR_MSG :
          if (msg_id == UICLIENT_CELLULAR_INFO_CHANGE_ID)
          {
            uiclient_update_info(UICLIENT_SCREEN_CELLULAR_INFO);
          }
          else if (msg_id == UICLIENT_CELLULAR_DATETIME_CHANGE_ID)
          {
            uiclient_update_info(UICLIENT_SCREEN_DATETIME_INFO);
            /* Start periodical DateTime Timer ? */
            if (cellular_app_uiclient_datetime_timer_state == CELLULAR_APP_UICLIENT_TIMER_IDLE)
            {
              if (rtosalTimerStart(cellular_app_uiclient_datetime_timer_id, UICLIENT_DATETIME_READ_TIMER) == osOK)
              {
                cellular_app_uiclient_datetime_timer_state = CELLULAR_APP_UICLIENT_TIMER_RUN;
              }
            }
          }
          else /* Should not happen */
          {
            __NOP();
          }
          break;

        case UICLIENT_TIMER_MSG :
#if (USE_RTC == 1)
          if (msg_id == UICLIENT_DATETIME_READ_ID)
          {
            uiclient_update_info(UICLIENT_SCREEN_DATETIME_INFO);
          }
#endif /* USE_RTC == 1 */
#if (USE_SENSORS == 1)
          if (msg_id == UICLIENT_SENSORS_READ_ID)
          {
            uiclient_update_info(UICLIENT_SCREEN_SENSORS_INFO);
            /* Rearm Sensors Timer */
            if (cellular_app_uiclient_sensors_timer_state == CELLULAR_APP_UICLIENT_TIMER_RUN)
            {
              if (rtosalTimerStart(cellular_app_uiclient_sensors_timer_id, UICLIENT_SENSORS_READ_TIMER)
                  == osOK)
              {
                PRINT_INFO("%s: Sensors timer rearmed", p_cellular_app_uiclient_trace)
              }
              else /* Error to rearm Sensors Timer */
              {
                PRINT_FORCE("%s: Sensors timer rearmed NOK !", p_cellular_app_uiclient_trace)
              }
            }
          }
#endif /* USE_SENSORS == 1 */
          break;

        default : /* Should not happen */
          __NOP();
          break;
      }
    }
  }
}

/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  Initialize all needed structures to support UIClt feature
  * @param  -
  * @retval -
  */
void cellular_app_uiclient_init(void)
{
  /* Initialize trace shortcut */
  p_cellular_app_uiclient_trace = cellular_app_type_string[CELLULAR_APP_TYPE_UICLIENT];

  /* Application Initialization */
  cellular_app_uiclient.app_id = 0U;
  /* Process Status Initialization */
  cellular_app_uiclient.process_status = true;
  /* Process Period Initialization */
  cellular_app_uiclient.process_period = 0U; /* Unused */
  /* Thread Id Initialization */
  cellular_app_uiclient.thread_id = NULL;
  /* Queue Id Initialization/Creation */
  cellular_app_uiclient.queue_id = rtosalMessageQueueNew((const rtosal_char_t *)"CELLAPP_QUE_UI_CMD",
                                                         CELLULAR_APP_QUEUE_SIZE);

  /* Specific Initialization */
#if (DISPLAY_WAIT_MODEM_IS_ON == 1)
  /* Modem is on Initialization */
  cellular_app_uiclient_modem_is_on = false;
#endif /* DISPLAY_WAIT_MODEM_IS_ON == 1 */
  /* Display is Initialized */
  cellular_app_uiclient_display_init_ok = false;
  /* Screen State Initialization */
  cellular_app_uiclient_screen_state = UICLIENT_SCREEN_OFF;

  /* Initialize cellular_app_uiclient_string_space */
  (void)memset(cellular_app_uiclient_string_space, 0x20, UICLIENT_STRING_SPACE_LENGTH_MAX);

#if (USE_RTC == 1)
  /* Need a timer to read periodically the DateTime */
  cellular_app_uiclient_datetime_timer_id = rtosalTimerNew((const rtosal_char_t *)"CELLAPP_TIM_UI_DATETIME",
                                                           (os_ptimer)cellular_app_uiclient_datetime_timer_cb,
                                                           osTimerPeriodic, NULL);
  cellular_app_uiclient_datetime_timer_state = CELLULAR_APP_UICLIENT_TIMER_IDLE;
#endif /* USE_RTC == 1 */

#if (USE_SENSORS == 1)
  /* Need a timer to read periodically the Sensors - Only one 'sensors read' request at a time */
  cellular_app_uiclient_sensors_timer_id = rtosalTimerNew((const rtosal_char_t *)"CELLAPP_TIM_UI_SENSOR",
                                                          (os_ptimer)cellular_app_uiclient_sensors_timer_cb,
                                                          osTimerOnce, NULL);
  cellular_app_uiclient_sensors_timer_state = CELLULAR_APP_UICLIENT_TIMER_IDLE;
#endif /* USE_SENSORS == 1 */

  /* Check Initialization is ok */
  if (cellular_app_uiclient.queue_id == NULL)
  {
    CELLULAR_APP_ERROR(CELLULAR_APP_ERROR_UICLIENT, ERROR_FATAL)
  }
#if (USE_SENSORS == 1)
  if (cellular_app_uiclient_sensors_timer_id == NULL)
  {
    cellular_app_uiclient_sensors_timer_state = CELLULAR_APP_UICLIENT_TIMER_INVALID;
    CELLULAR_APP_ERROR((CELLULAR_APP_ERROR_UICLIENT + 1), ERROR_FATAL)
  }
#endif /* USE_SENSORS == 1 */
}

/**
  * @brief  Start UIClt thread
  * @param  -
  * @retval -
  */
void cellular_app_uiclient_start(void)
{
  uint8_t thread_name[CELLULAR_APP_THREAD_NAME_MAX];
  uint32_t len;

  /* Cellular initialization already done - Registration to services is OK */
  /* Registration to CellularInfo: needs to know all cellular status modification to update the display */
  if (cellular_info_cb_registration(cellular_app_uiclient_cellular_info_status_cb, (void *) NULL) != CELLULAR_SUCCESS)
  {
    CELLULAR_APP_ERROR((CELLULAR_APP_ERROR_UICLIENT + 2), ERROR_FATAL)
  }
  /* Registration to CellularSignalInfo: needs to know all signal status modification to update the display */
  if (cellular_signal_info_cb_registration(cellular_app_uiclient_signal_info_status_cb, (void *) NULL)
      != CELLULAR_SUCCESS)
  {
    CELLULAR_APP_ERROR((CELLULAR_APP_ERROR_UICLIENT + 3), ERROR_FATAL)
  }

#if (USE_RTC == 1)
  /* Registration to cellular_app_datetime_set_cb() to know when datetime is set */
  cellular_app_datetime_set_cb_registration(cellular_app_uiclient_datetime_set_cb, NULL);
#endif /* USE_DISPLAY == 1 */

#if (USE_SENSORS == 1)
  if (cellular_app_uiclient_sensors_timer_state == CELLULAR_APP_UICLIENT_TIMER_IDLE)
  {
    /* Initialization Sensors modules */
    cellular_app_sensors_init();
    /* Initialization Sensor Humidity */
    if (cellular_app_sensors_initialize(CELLULAR_APP_SENSOR_TYPE_HUMIDITY) == false)
    {
      PRINT_FORCE("%s: Humidity sensors init NOK!", p_cellular_app_uiclient_trace)
    }
    /* Initialization Sensor Pressure */
    if (cellular_app_sensors_initialize(CELLULAR_APP_SENSOR_TYPE_PRESSURE) == false)
    {
      PRINT_FORCE("%s: Pressure sensor init NOK!", p_cellular_app_uiclient_trace)
    }
    /* Initialization Sensors Temperature */
    if (cellular_app_sensors_initialize(CELLULAR_APP_SENSOR_TYPE_TEMPERATURE) == false)
    {
      PRINT_FORCE("%s: Temperature sensor init NOK!", p_cellular_app_uiclient_trace)
    }
  }
#endif /* USE_SENSORS == 1 */
  /* Thread Name Generation */
  len = crs_strlen((const uint8_t *)"UIClt");
  /* '+1' to copy '\0' */
  (void)memcpy(thread_name, "UIClt", CELLULAR_APP_MIN((len + 1U), CELLULAR_APP_THREAD_NAME_MAX));

  /* Thread Creation */
  cellular_app_uiclient.thread_id = rtosalThreadNew((const rtosal_char_t *)thread_name, (os_pthread)uiclient_thread,
                                                    UICLIENT_THREAD_PRIO, UICLIENT_THREAD_STACK_SIZE, NULL);
  /* Check Creation is ok */
  if (cellular_app_uiclient.thread_id == NULL)
  {
    CELLULAR_APP_ERROR((CELLULAR_APP_ERROR_UICLIENT + 4), ERROR_FATAL)
  }
}

#endif /* USE_DISPLAY == 1 */
