/**
  ******************************************************************************
  * @file    cellular_service_utils.c
  * @author  MCD Application Team
  * @brief   This file defines utilities functions for Cellular Service Task
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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
#include "cellular_service_utils.h"
#include "cellular_service_task.h"
#if (USE_LOW_POWER == 1)
#include "cellular_service_power.h"
#endif  /* (USE_LOW_POWER == 1) */
#include "error_handler.h"

#include "at_util.h"
#include "cellular_datacache.h"
#include "cellular_service_os.h"
#include "cellular_service_config.h"

#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
#include "ppposif_client.h"
#endif /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */

#if (USE_CMD_CONSOLE == 1)
#include "cellular_service_cmd.h"
#include "cmd.h"
#endif  /* (USE_CMD_CONSOLE == 1) */

#if (USE_PRINTF == 0U)
/* Trace macro definition */
#include "trace_interface.h"
#define PRINT_FORCE(format, args...) \
  TRACE_PRINT_FORCE(DBG_CHAN_CELLULAR_SERVICE, DBL_LVL_P0, "" format "", ## args)
#else
#include <stdio.h>
#define PRINT_FORCE(format, args...)                (void)printf(format, ## args);
#endif  /* (USE_PRINTF == 1) */

/* SIM slot polling period */
#define CST_SIM_POLL_COUNT     200U    /* 20s */

/* Maximum length of APN string */
#define APN_LENGTH_MAX 20

/* Maximum length of username and password for APN access string */
#define USR_PASS_LENGTH_MAX 20

/* Array structure for MMN/MNC - APN association */
typedef struct
{
  uint8_t mmcmnc[6];
  uint8_t apn[APN_LENGTH_MAX];
  uint8_t username[USR_PASS_LENGTH_MAX];
  uint8_t password[USR_PASS_LENGTH_MAX];
} mmcmnc_apn_t;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  convert sim slot DC enum value to CS enum value
  * @param  sim_slot_value - sim slot DC enum value
  * @retval CS_SimSlot_t  - sim slot CS enum value
  */
static CS_SimSlot_t  cst_convert_sim_socket_type(dc_cs_sim_slot_type_t sim_slot_value);

/**
  * @brief  64bits modulo calculation
  * @param  div   - divisor
  * @param  val_m  - high 32bits value
  * @param  val_l  - low  32bits value
  * @retval uint32_t - result of modulo calculation
  */
static uint32_t cst_modulo64(uint32_t div, uint32_t val_m, uint32_t val_l);

/**
  * @brief  subscribes to network events
  * @param  -
  * @retval -
  */
static void CST_subscribe_all_net_events(void);

/**
  * @brief  sets modem infos in data cache
  * @param  target_state  - modem target state
  * @retval -
  */
static void CST_get_device_all_infos(dc_cs_target_state_t  target_state);

/**
  * @brief  URC callback (Unsollicited Result Code from modem)
  * @param  -
  * @retval -
  */
static void CST_network_reg_callback(void);

/**
  * @brief  location info callback callback
  * @param  -
  * @retval -
  */
static void CST_location_info_callback(void);

/**
  * @brief  modem event callback
  * @param  event - modem event
  * @retval -
  */
static void CST_modem_event_callback(CS_ModemEvent_t event);

/**
  * @brief  calculates NFMC tempos and sets them in DataCache
  * @param  imsi_high  - high IMSI 32bits value
  * @param  imsi_low   - low IMSI  32bits value
  * @retval -
  */
static void CST_fill_nfmc_tempo(uint32_t imsi_high, uint32_t imsi_low);

/**
  * @brief  PDN definition management
  * @param  -
  * @retval -
  */
static void CST_modem_define_pdn(uint8_t *apn, uint8_t *username, uint8_t *password);

/**
  * @brief  setting FAIL mode in DC
  * @param  -
  * @retval -
  */
static void CST_fail_setting(void);

/* Private function Definition -----------------------------------------------*/

/* ===================================================================
   Tools functions  BEGIN
   =================================================================== */

/**
  * @brief  convert sim slot DC enum value to CS enum value
  * @param  sim_slot_value - sim slot DC enum value
  * @retval CS_SimSlot_t  - sim slot CS enum value
  */
static CS_SimSlot_t  cst_convert_sim_socket_type(dc_cs_sim_slot_type_t sim_slot_value)
{
  CS_SimSlot_t enum_value;
  switch (sim_slot_value)
  {
    case DC_SIM_SLOT_MODEM_SOCKET:
      enum_value = CS_MODEM_SIM_SOCKET_0;
      break;
    case DC_SIM_SLOT_MODEM_EMBEDDED_SIM:
      enum_value = CS_MODEM_SIM_ESIM_1;
      break;
    case DC_SIM_SLOT_STM32_EMBEDDED_SIM:
      enum_value = CS_STM32_SIM_2;
      break;
    default:
      enum_value = CS_MODEM_SIM_SOCKET_0;
      break;
  }
  return enum_value;
}


/**
  * @brief  64bits modulo calculation
  * @param  div   - divisor
  * @param  val_m  - high 32bits value
  * @param  val_l  - low  32bits value
  * @retval uint32_t - result of modulo calculation
  */
static uint32_t cst_modulo64(uint32_t div, uint32_t val_m, uint32_t val_l)
{
  uint32_t div_m;
  uint32_t div_l;
  uint32_t tmp_m;
  uint32_t tmp_l;

  tmp_l = 0U;
  div_m = div;
  div_l = 0U;

  if (div_m != 0U)
  {
    tmp_m = val_m % div_m;

    tmp_l = val_l;

    while (tmp_m > 0U)
    {
      if ((div_m > tmp_m) || ((div_m == tmp_m) && (div_l > tmp_l)))
      {
        /* Nothing to do */
      }
      else if (div_l > tmp_l)
      {
        tmp_l = tmp_l - div_l;
        tmp_m--;
        tmp_m = tmp_m - div_m;
      }
      else
      {
        tmp_m = tmp_m - div_m;
        tmp_l = tmp_l - div_l;
      }

      div_l = div_l >> 1;
      if ((div_m & 1U) == 1U)
      {
        div_l = div_l | 0x80000000U;
      }
      div_m = div_m >> 1U;
    }
    tmp_l = tmp_l % div;
  }
  return tmp_l;
}

/* ===================================================================
   Tools functions  END
   =================================================================== */

/* ===================================================================
   UTility functions  BEGIN
   =================================================================== */

/**
  * @brief  URC callback (Unsolicited Result Code from modem)
  * @param  -
  * @retval -
  */
static void CST_network_reg_callback(void)
{
  PRINT_CELLULAR_SERVICE("==================================CST_network_reg_callback\n\r")
  /* sends a message to automaton */
  CST_send_message(CST_MESSAGE_CS_EVENT, CST_NETWORK_CALLBACK_EVENT);
}

/**
  * @brief  location info callback callback
  * @param  -
  * @retval -
  */
static void CST_location_info_callback(void)
{
  /* Not used yet: reserved for future usage */
  PRINT_CELLULAR_SERVICE("CST_location_info_callback\n\r")
}

/**
  * @brief  modem event callback
  * @param  event - modem event
  * @retval -
  */
static void CST_modem_event_callback(CS_ModemEvent_t event)
{
  /* event is a bitmask, we can have more than one evt reported at the same time */
  if (((uint16_t)event & (uint16_t)CS_MDMEVENT_BOOT) != 0U)
  {
    /* reboot modem has occurred: sends a message to automaton */
    CST_send_message(CST_MESSAGE_CS_EVENT, CST_REBOOT_MODEM_EVENT);
#if (USE_LOW_POWER == 1)
    (void)CSP_DataWakeup(MODEM_WAKEUP);
#endif /* (USE_LOW_POWER == 1) */
  }
  if (((uint16_t)event & (uint16_t)CS_MDMEVENT_POWER_DOWN) != 0U)
  {
    /* Modem power down event: sends a message to automaton */
    PRINT_CELLULAR_SERVICE("Modem event received:  CS_MDMEVENT_POWER_DOWN\n\r")
    /* We reboot the Modem if it power down */
    CST_send_message(CST_MESSAGE_CS_EVENT, CST_MODEM_POWER_DOWN_EVENT);

    /* Nothing to do */
  }
  if (((uint16_t)event & (uint16_t)CS_MDMEVENT_FOTA_START) != 0U)
  {
    /* FOTA programmation start : send a message to automaton */
    CST_send_message(CST_MESSAGE_CS_EVENT, CST_FOTA_START_EVENT);
  }
  if (((uint16_t)event & (uint16_t)CS_MDMEVENT_FOTA_END) != 0U)
  {
    /* FOTA programmation end : sends a message to automaton */
    CST_send_message(CST_MESSAGE_CS_EVENT, CST_FOTA_END_EVENT);
  }

#if (USE_LOW_POWER == 1)
  /* ---------------- */
  /* Low Power events */
  /* ---------------- */

  if (((uint16_t)event & (uint16_t)CS_MDMEVENT_LP_ENTER) != 0U)
  {
    /* Enter Low power: sends a message to automaton  */
    PRINT_CELLULAR_SERVICE("Modem event received:  CS_MDMEVENT_LP_ENTER\n\r")
    CST_send_message(CST_MESSAGE_CMD, CST_POWER_SLEEP_COMPLETE_EVENT);
  }

  /* Modem requests to leave Low Power  */
  if (((uint16_t)event & (uint16_t)CS_MDMEVENT_WAKEUP_REQ) != 0U)
  {
    /* Modem requests to leave Low Power  */
    (void)CSP_DataWakeup(MODEM_WAKEUP);
    PRINT_CELLULAR_SERVICE("Modem event received:  CS_MDMEVENT_WAKEUP_REQ\n\r")
  }

  if (((uint16_t)event & (uint16_t)CS_MDMEVENT_LP_LEAVE) != 0U)
  {
    /* Modem Leave Low Power state  */
    PRINT_CELLULAR_SERVICE("Modem event received:  CS_MDMEVENT_LP_LEAVE\n\r")
    /* Nothing to do */
  }
#endif /* (USE_LOW_POWER == 1) */
}




/**
  * @brief  init modem management
  * @param  -
  * @retval -
  */
void  CST_modem_sim_init(void)
{
  /* When updating this array, do not forget to update the value of MMCMNC_APN_MAX */
  static mmcmnc_apn_t mmcmnc_apn_a[MMCMNC_APN_MAX] =
  {
    /* { MMC/MNC, APN, username, password } */
    {"00101", "default", "", ""},
    {"20810", "iotinternet", "", ""}, /* SFR 20810 -> iotinternet*/
    {"29510", "soracom.io", "sora", "sora"},
    {"23425", "iot.truphone.com", "", ""},
    {"29505", "EM", "", ""},
  };

  dc_apn_config_t apn_config;
  CS_Status_t cs_status;
  cs_status = CELLULAR_OK;
  PRINT_CELLULAR_SERVICE("*********** CST_modem_sim_init ********\n\r")
#if (USE_LOW_POWER == 1)  /* Power init must be done before starting the MOdem RF */
  CSP_InitPowerConfig();
#endif  /* (USE_LOW_POWER == 1) */


  /* sim slot select */
  (void)osCS_sim_select(cst_convert_sim_socket_type(cst_sim_info.active_slot));

  (void)rtosalDelay(10);  /* waiting for 10ms after sim selection */
  if (cst_cellular_params.target_state == DC_TARGET_STATE_SIM_ONLY)
  {
    /* Modem Target state : SIM ONLY */
    cs_status = osCDS_init_modem(CS_CMI_SIM_ONLY, CELLULAR_FALSE, CST_SIM_PINCODE);
  }
  else if (cst_cellular_params.target_state == DC_TARGET_STATE_FULL)
  {
    /* if automatic APN selection mode is enabled then perform following steps
        first check the SIM presence by reading the IMSI
        when IMSI is present then select the APN according to the IMSI
        using a IMSI configuration table
        MCC      MNC     apnname auth_prot_pres username password
        3digits  2digits string  1 or 0
        23425  iot.truphone.com auth username password
        29510  soracom.io auth username password
        00101  default (Amarisoft)
        second retrieve the APN name and parameters
        if apn has been found then configure the related APN
        if apn cannot be found then use the APN from the configuration file.
        third set pdn
    */
    /* Modem Target state : FULL */
    PRINT_CELLULAR_SERVICE("CST_modem_sim_init : check SIM First \n\r")
    /* to do  call a function to check SIM presence and get SIM info, need to move here the SIM verification */
    /* CST_get_device_all_infos */
    /* first check SIM presence */
    CS_DeviceInfo_t cst_imsi_info;
    cs_status = osCDS_init_modem(CS_CMI_SIM_ONLY, CELLULAR_FALSE, CST_SIM_PINCODE);
    if (cs_status == CELLULAR_OK)
    {
      /* if SIM Present then read IMSI */
      cst_imsi_info.field_requested = CS_DIF_IMSI_PRESENT;
      cs_status = osCDS_get_device_info(&cst_imsi_info);
      /* IMSI available => SIM is present on this slot */
      uint32_t cst_imsi_high;
      uint32_t cst_imsi_low;
      uint8_t mmc_mnc[6];
      (void)ATutil_convertHexaStringToInt64(cst_imsi_info.u.imsi, 15U, &cst_imsi_high, &cst_imsi_low);
      /* The 5 firsts digit of IMEI are MMC + MNC */
      /* copy the five first digit of the IMSI to the mmc_mnc variable */
      (void)memcpy(mmc_mnc, cst_imsi_info.u.imsi, 5);
      mmc_mnc[5] = 0;
      PRINT_CELLULAR_SERVICE(" -IMSI: %lx%lx\n\r -MMC/MNC: %s\n\r", cst_imsi_high, cst_imsi_low, mmc_mnc)

      if (cst_cellular_params.set_pdn_mode != 0U)
      {
        uint8_t idx_apn_found;
        bool    apn_found;

        /* In no APN is defines in EEPROM configuration, then, check if MMC/MNC is present in the */
        /* MMC/MNC <-> APN table */
        if (crs_strlen(cst_cellular_params.sim_slot[cst_context.sim_slot_index].apn) == 0U)
        {

          /* the function CST_lut_pdn() retrieve the PDN descriptor from a data base
          if an APN is found then this PDN must be used call osCDS_define_pdn()
          if no APN can be found then a PDN from configuration must be used (call CST_modem_define_pdn() */
          /* we must first define Cellular context before activating the RF because
            * modem will immediately attach to network once RF is enabled
          */
          idx_apn_found = 0U;
          apn_found = false;
          /* Parse the MMC/MNC <-> APN table to search for the actual MMC/MNC read in the SIM */
          for (uint8_t i = 0U; i < MMCMNC_APN_MAX; i++)
          {
            if (memcmp(mmc_mnc, mmcmnc_apn_a[i].mmcmnc, sizeof(mmc_mnc)) == 0)
            {
              /* MMC/MNC found, store the corresponding APN index for future use */
              idx_apn_found = i;
              apn_found = true;
              PRINT_CELLULAR_SERVICE(" -Found corresponding APN: \"%s\" in table\n\r", mmcmnc_apn_a[i].apn)
            }
          }
          /* if an APN has been found in the MMC/MNC <-> APN table, then use it with it's username and password */
          /* to configure the modem */
          if (apn_found)
          {
            PRINT_CELLULAR_SERVICE("CST_modem_sim_init : CST_modem_define_pdn with APN: \"%s\"\n\r",

                                   /* Set APN in modem data structure */
                                   mmcmnc_apn_a[idx_apn_found].apn)
            CST_modem_define_pdn(mmcmnc_apn_a[idx_apn_found].apn,
                                 mmcmnc_apn_a[idx_apn_found].username,
                                 mmcmnc_apn_a[idx_apn_found].password);

            /* Update data cache with used APN information */
            (void)dc_com_read(&dc_com_db, DC_CELLULAR_APN_CONFIG, (void *)&apn_config, sizeof(apn_config));
            /* Copy APN */
            (void)memcpy(apn_config.apn, mmcmnc_apn_a[idx_apn_found].apn,
                         crs_strlen(mmcmnc_apn_a[idx_apn_found].apn) + 1U);
            /* Copy APN user name */
            (void)memcpy(apn_config.apn, mmcmnc_apn_a[idx_apn_found].username,
                         crs_strlen(mmcmnc_apn_a[idx_apn_found].username) + 1U);
            /* Copy APN password */
            (void)memcpy(apn_config.apn, mmcmnc_apn_a[idx_apn_found].password,
                         crs_strlen(mmcmnc_apn_a[idx_apn_found].password) + 1U);
            /* Write APN information to data cache */
            (void)dc_com_write(&dc_com_db, DC_CELLULAR_APN_CONFIG, (void *)&apn_config, sizeof(apn_config));
          }
          else
            /* if an APN has NOT been found in the MMC/MNC <-> APN table, then use the default values stored in */
            /* the EEPROM */
          {
            PRINT_CELLULAR_SERVICE("CST_modem_sim_init : CST_modem_define_pdn with Flash stored APN\n\r")
            CST_modem_define_pdn((uint8_t *)"", (uint8_t *)"", (uint8_t *)"");
          }
        }
        else
          /* if an APN is stored in the EEPROM, let's use it */
        {
          PRINT_CELLULAR_SERVICE("CST_modem_sim_init : CST_modem_define_pdn with Flash stored APN\n\r")
          CST_modem_define_pdn((uint8_t *)"", (uint8_t *)"", (uint8_t *)"");
        }
        cs_status = osCDS_init_modem(CS_CMI_FULL, CELLULAR_FALSE, CST_SIM_PINCODE);
      }
    }
  }
  else
  {
    __NOP(); /* Nothing to do */
  }

  if (cs_status == CELLULAR_SIM_INCORRECT_PASSWORD)
  {
    PRINT_FORCE("==================================\n\r")
    PRINT_FORCE(" WARNING: WRONG PIN CODE !!!\n\r")
    PRINT_FORCE(" DO NOT RESTART THE BOARD WITHOUT SETTING A CORRECT PIN CODE\n\r")
    PRINT_FORCE(" TO AVOID LOCKING THE SIM ! \n\r")
    PRINT_FORCE("==================================\n\r")
    for (;;)
    {
      /* Infinite loop to avoid to restart the board */
      __NOP(); /* Nothing to do */
    }
  }
  else if (cs_status == CELLULAR_SIM_PIN_OR_PUK_LOCKED)
  {
    PRINT_FORCE("==================================\n\r")
    PRINT_FORCE(" WARNING: PIN OK PUK LOCKED !!!  \n\r")
    PRINT_FORCE(" PROCESSING STOPPED\n\r")
    PRINT_FORCE("==================================\n\r")
    for (;;)
    {
      /* Infinite loop to avoid to restart the board */
      __NOP(); /* Nothing to do */
    }
  }
  else
  {
    __NOP(); /* Nothing to do */
  }

  if ((cs_status == CELLULAR_SIM_NOT_INSERTED) || (cs_status == CELLULAR_ERROR) || (cs_status == CELLULAR_SIM_ERROR))
  {
    /* SIM Error: FAIL */
    (void)dc_com_read(&dc_com_db, DC_CELLULAR_SIM_INFO, (void *)&cst_sim_info, sizeof(cst_sim_info));
    cst_sim_info.sim_status[cst_context.sim_slot_index] = DC_SIM_ERROR;
    cst_sim_info.rt_state   = DC_SERVICE_ON;
    cst_context.sim_slot_index++;

    if (cst_context.sim_slot_index  >= cst_cellular_params.sim_slot_nb)
    {
      cst_context.sim_slot_index = 0U; /* No available SIM found: set default SIM socket (Modem socket) */
      PRINT_CELLULAR_SERVICE("CST_modem_powered_on_state : No SIM found\n\r")
    }

    cst_sim_info.active_slot = cst_cellular_params.sim_slot[cst_context.sim_slot_index].sim_slot_type;
    cst_sim_info.index_slot  = cst_context.sim_slot_index;
    (void)dc_com_write(&dc_com_db, DC_CELLULAR_SIM_INFO, (void *)&cst_sim_info, sizeof(cst_sim_info));

    CST_config_fail(((uint8_t *)"CST_modem_sim_init"),
                    CST_MODEM_SIM_FAIL,
                    &cst_context.sim_reset_count,
                    CST_SIM_RETRY_MAX);

  }
  else
  {
    /* SIM OK */
    /* Init Power config after Modem Power On and before subscribe modem events  */

    CST_subscribe_all_net_events();

    CST_get_device_all_infos(cst_cellular_params.target_state);
    if (cst_cellular_params.target_state != DC_TARGET_STATE_SIM_ONLY)
    {
      CST_set_state(CST_MODEM_READY_STATE);
      CST_send_message(CST_MESSAGE_CS_EVENT, CST_MODEM_READY_EVENT);
    }
    else
    {
      /* Modem Target State == SIM ONLY : end of processing */
      /* DC Update */
      (void)dc_com_read(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(dc_cellular_info_t));
      cst_cellular_info.rt_state    = DC_SERVICE_ON;
      cst_cellular_info.modem_state = DC_MODEM_STATE_SIM_CONNECTED;
      (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(dc_cellular_info_t));

      CST_set_state(CST_MODEM_SIM_ONLY_STATE);
    }
  }
}

/**
  * @brief  subscribes to network events
  * @param  -
  * @retval -
  */
static void CST_subscribe_all_net_events(void)
{
  PRINT_CELLULAR_SERVICE("Subscribe URC events: Network registration\n\r")
  (void)osCDS_subscribe_net_event(CS_URCEVENT_CS_NETWORK_REG_STAT, CST_network_reg_callback);
  (void)osCDS_subscribe_net_event(CS_URCEVENT_GPRS_NETWORK_REG_STAT, CST_network_reg_callback);
  (void)osCDS_subscribe_net_event(CS_URCEVENT_EPS_NETWORK_REG_STAT, CST_network_reg_callback);
  PRINT_CELLULAR_SERVICE("Subscribe URC events: Location info\n\r")
  (void)osCDS_subscribe_net_event(CS_URCEVENT_EPS_LOCATION_INFO, CST_location_info_callback);
  (void)osCDS_subscribe_net_event(CS_URCEVENT_GPRS_LOCATION_INFO, CST_location_info_callback);
  (void)osCDS_subscribe_net_event(CS_URCEVENT_CS_LOCATION_INFO, CST_location_info_callback);
}

/**
  * @brief  calculates NFMC tempos and sets them in DataCache
  * @param  imsi_high  - high IMSI 32bits value
  * @param  imsi_low   - low IMSI  32bits value
  * @retval -
  */
static void CST_fill_nfmc_tempo(uint32_t imsi_high, uint32_t imsi_low)
{
  uint32_t i;
  dc_nfmc_info_t nfmc_info;

  if (cst_cellular_params.nfmc_active != 0U)
  {
    /* NFMC active : NFMC tempos calculation */
    cst_nfmc_context.active = true;
    nfmc_info.activate = 1U;
    for (i = 0U; i < CST_NFMC_TEMPO_NB; i++)
    {
      uint32_t temp_value32;

      /* calculation of NFMC tempo */
      if (cst_cellular_params.nfmc_value[i] != 0U)
      {
        temp_value32 = cst_modulo64(cst_cellular_params.nfmc_value[i], imsi_high, imsi_low);
      }
      else
      {
        temp_value32 = imsi_low;  /* parameter value == 0 => value set to imsi_low */
      }
      temp_value32 = temp_value32 + cst_cellular_params.nfmc_value[i];
      cst_nfmc_context.tempo[i] = (0xffffffffU & temp_value32);

      nfmc_info.tempo[i] = cst_nfmc_context.tempo[i];
      PRINT_CELLULAR_SERVICE("VALUE/TEMPO %ld/%ld\n\r",  cst_cellular_params.nfmc_value[i], cst_nfmc_context.tempo[i])
    }
    nfmc_info.rt_state = DC_SERVICE_ON;
  }
  else
  {
    /* NFMC not active */
    nfmc_info.activate = 0U;
    nfmc_info.rt_state = DC_SERVICE_OFF;
    cst_nfmc_context.active = false;
  }
  (void)dc_com_write(&dc_com_db, DC_CELLULAR_NFMC_INFO, (void *)&nfmc_info, sizeof(nfmc_info));
}

/**
  * @brief  update Cellular Info entry of Data Cache
  * @param  dc_service_state - new entry state to set
  * @param  ip_addr - new IP address (null if not defined)
  * @retval -
  */
void  CST_data_cache_cellular_info_set(dc_service_rt_state_t dc_service_state, dc_network_addr_t *ip_addr)
{
  dc_nifman_info_t nifman_info;

  (void)dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));

  if (nifman_info.rt_state != dc_service_state)
  {
    nifman_info.rt_state   =  dc_service_state;
#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
    nifman_info.network    =  DC_CELLULAR_SOCKETS_LWIP;
#else
    nifman_info.network    =  DC_CELLULAR_SOCKET_MODEM;
#endif /*  (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)*/
    if (ip_addr != NULL)
    {
      (void)memcpy(&nifman_info.ip_addr, ip_addr, sizeof(dc_network_addr_t));
    }
    (void)dc_com_write(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
  }


  (void)dc_com_read(&dc_com_db, DC_CELLULAR_DATA_INFO, (void *)&cst_cellular_data_info,
                    sizeof(cst_cellular_data_info));

  if (cst_cellular_data_info.rt_state != dc_service_state)
  {
    cst_cellular_data_info.rt_state   =  dc_service_state;
#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
    cst_cellular_data_info.network    =  DC_NETWORK_SOCKETS_LWIP;
#else
    cst_cellular_data_info.network    =  DC_NETWORK_SOCKET_MODEM;
#endif /*  (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)*/
    if (ip_addr != NULL)
    {
      (void)memcpy(&cst_cellular_data_info.ip_addr, ip_addr, sizeof(dc_network_addr_t));
    }

    (void)dc_com_write(&dc_com_db, DC_CELLULAR_DATA_INFO, (void *)&cst_cellular_data_info,
                       sizeof(cst_cellular_data_info));
  }
}

/**
  * @brief  sets modem infos in data cache
  * @param  target_state  - modem target state
  * @retval -
  */
static void CST_get_device_all_infos(dc_cs_target_state_t  target_state)
{
  static CS_DeviceInfo_t cst_device_info;
  CS_Status_t            cs_status;
  uint16_t               sim_poll_count;
  bool                   end_of_loop;
  uint32_t               cst_imsi_high;
  uint32_t               cst_imsi_low;

  sim_poll_count = 0U;

  (void)memset((void *)&cst_device_info, 0, sizeof(CS_DeviceInfo_t));

  /* read current device info in Data Cache */
  (void)dc_com_read(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));


  /* gets IMEI */
  cst_device_info.field_requested = CS_DIF_IMEI_PRESENT;
  if (osCDS_get_device_info(&cst_device_info) == CELLULAR_OK)
  {
    (void)memcpy(cst_cellular_info.imei, cst_device_info.u.imei, DC_MAX_SIZE_IMEI - 1U);
    cst_cellular_info.imei[DC_MAX_SIZE_IMEI - 1U] = 0U;     /* to avoid a non null terminated string */
    PRINT_CELLULAR_SERVICE(" -IMEI: %s\n\r", cst_device_info.u.imei)
  }
  else
  {
    cst_cellular_info.imei[0] = 0U;
    PRINT_CELLULAR_SERVICE("IMEI error\n\r")
  }


  /* gets Manufacturer Name  of modem*/
  cst_device_info.field_requested = CS_DIF_MANUF_NAME_PRESENT;
  if (osCDS_get_device_info(&cst_device_info) == CELLULAR_OK)
  {
    (void)memcpy((CRC_CHAR_t *)cst_cellular_info.manufacturer_name,
                 (CRC_CHAR_t *)cst_device_info.u.manufacturer_name,
                 DC_MAX_SIZE_MANUFACT_NAME - 1U);
    /* to avoid a non null terminated string */
    cst_cellular_info.manufacturer_name[DC_MAX_SIZE_MANUFACT_NAME - 1U] = 0U;
    PRINT_CELLULAR_SERVICE(" -MANUFACTURER: %s\n\r", cst_device_info.u.manufacturer_name)
  }
  else
  {
    cst_cellular_info.manufacturer_name[0] = 0U;
    PRINT_CELLULAR_SERVICE("Manufacturer Name error\n\r")
  }

  /* gets Model modem  */
  cst_device_info.field_requested = CS_DIF_MODEL_PRESENT;
  if (osCDS_get_device_info(&cst_device_info) == CELLULAR_OK)
  {
    (void)memcpy((CRC_CHAR_t *)cst_cellular_info.model,
                 (CRC_CHAR_t *)cst_device_info.u.model,
                 DC_MAX_SIZE_MODEL - 1U);
    cst_cellular_info.model[DC_MAX_SIZE_MODEL - 1U] = 0U; /* to avoid a non null terminated string */
    PRINT_CELLULAR_SERVICE(" -MODEL: %s\n\r", cst_device_info.u.model)
  }
  else
  {
    cst_cellular_info.model[0] = 0U;
    PRINT_CELLULAR_SERVICE("Model error\n\r")
  }

  /* gets revision of modem  */
  cst_device_info.field_requested = CS_DIF_REV_PRESENT;
  if (osCDS_get_device_info(&cst_device_info) == CELLULAR_OK)
  {
    (void)memcpy((CRC_CHAR_t *)cst_cellular_info.revision,
                 (CRC_CHAR_t *)cst_device_info.u.revision,
                 DC_MAX_SIZE_REV - 1U);
    cst_cellular_info.revision[DC_MAX_SIZE_REV - 1U] = 0U; /* to avoid a non null terminated string */
    PRINT_CELLULAR_SERVICE(" -REVISION: %s\n\r", cst_device_info.u.revision)
  }
  else
  {
    cst_cellular_info.revision[0] = 0U;
    PRINT_CELLULAR_SERVICE("Revision error\n\r")
  }

  /* gets serial number of modem  */
  cst_device_info.field_requested = CS_DIF_SN_PRESENT;
  if (osCDS_get_device_info(&cst_device_info) == CELLULAR_OK)
  {
    (void)memcpy((CRC_CHAR_t *)cst_cellular_info.serial_number,
                 (CRC_CHAR_t *)cst_device_info.u.serial_number,
                 DC_MAX_SIZE_SN - 1U);
    cst_cellular_info.serial_number[DC_MAX_SIZE_SN - 1U] = 0U; /* to avoid a non null terminated string */
    PRINT_CELLULAR_SERVICE(" -SERIAL NBR: %s\n\r", cst_device_info.u.serial_number)
  }
  else
  {
    cst_cellular_info.serial_number[0] = 0U;
    PRINT_CELLULAR_SERVICE("Serial Number error\n\r")
  }

  /* gets CCCID  */
  cst_device_info.field_requested = CS_DIF_ICCID_PRESENT;
  if (osCDS_get_device_info(&cst_device_info) == CELLULAR_OK)
  {
    (void)memcpy((CRC_CHAR_t *)cst_cellular_info.iccid,
                 (CRC_CHAR_t *)cst_device_info.u.iccid,
                 DC_MAX_SIZE_ICCID - 1U);
    cst_cellular_info.iccid[DC_MAX_SIZE_ICCID - 1U] = 0U; /* to avoid a non null terminated string */
    PRINT_CELLULAR_SERVICE(" -ICCID: %s\n\r", cst_device_info.u.iccid)
  }
  else
  {
    cst_cellular_info.serial_number[0] = 0U;
    PRINT_CELLULAR_SERVICE("Serial Number error\n\r")
  }

  /* writes updated cellular info in Data Cache */
  (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));

  end_of_loop = true;
  if (target_state == DC_TARGET_STATE_FULL)
  {
    /* modem target state: FULL */

    /* SIM info set to 'on going' in Data Cache during SIM connection */
    (void)dc_com_read(&dc_com_db, DC_CELLULAR_SIM_INFO, (void *)&cst_sim_info, sizeof(cst_sim_info));
    cst_sim_info.rt_state   = DC_SERVICE_ON;
    cst_sim_info.sim_status[cst_context.sim_slot_index] = DC_SIM_CONNECTION_ON_GOING;
    (void)dc_com_write(&dc_com_db, DC_CELLULAR_SIM_INFO, (void *)&cst_sim_info, sizeof(cst_sim_info));

    /* loop: waiting for SIM status */
    while (end_of_loop != false)
    {
      /* try to get IMSI to determine if SIM is present on this slot */
      cst_device_info.field_requested = CS_DIF_IMSI_PRESENT;
      cs_status = osCDS_get_device_info(&cst_device_info);
      if (cs_status == CELLULAR_OK)
      {
        /* IMSI available => SIM is present on this slot */

        /* NFMC tempos calculation (using IMSI)  (15 is size of imsi value) */
        (void)ATutil_convertHexaStringToInt64(cst_device_info.u.imsi, 15U, &cst_imsi_high, &cst_imsi_low);
        PRINT_CELLULAR_SERVICE(" -IMSI: %s\n\r", cst_device_info.u.imsi)
        CST_fill_nfmc_tempo(cst_imsi_high, cst_imsi_low);

        (void)memcpy((CRC_CHAR_t *)cst_sim_info.imsi,
                     (CRC_CHAR_t *)cst_device_info.u.imsi,
                     DC_MAX_SIZE_IMSI - 1U);
        cst_sim_info.imsi[DC_MAX_SIZE_IMSI - 1U] = 0;  /* to avoid a non null terminated string */
        cst_sim_info.sim_status[cst_context.sim_slot_index] = DC_SIM_OK;
        end_of_loop = false;
      }
      else if ((cs_status == CELLULAR_SIM_BUSY)
               || (cs_status == CELLULAR_SIM_ERROR))
      {
        /* SIM presently not available: poll it until available or polling time exceed */
        (void)rtosalDelay(100U);
        sim_poll_count++;
        if (sim_poll_count > CST_SIM_POLL_COUNT)
        {
          /* polling time exceed: SIM not available on this slot */
          if (cs_status == CELLULAR_SIM_BUSY)
          {
            cst_sim_info.sim_status[cst_context.sim_slot_index] = DC_SIM_BUSY;
          }
          else
          {
            cst_sim_info.sim_status[cst_context.sim_slot_index] = DC_SIM_ERROR;
          }
          end_of_loop = false;
        }
      }
      else
      {
        /* error returned => SIM not available. Getting SIM error cause */
        if (cs_status == CELLULAR_SIM_NOT_INSERTED)
        {
          cst_sim_info.sim_status[cst_context.sim_slot_index] = DC_SIM_NOT_INSERTED;
        }
        else if (cs_status == CELLULAR_SIM_PIN_OR_PUK_LOCKED)
        {
          cst_sim_info.sim_status[cst_context.sim_slot_index] = DC_SIM_PIN_OR_PUK_LOCKED;
        }
        else if (cs_status == CELLULAR_SIM_INCORRECT_PASSWORD)
        {
          cst_sim_info.sim_status[cst_context.sim_slot_index] = DC_SIM_INCORRECT_PASSWORD;
        }
        else
        {
          cst_sim_info.sim_status[cst_context.sim_slot_index] = DC_SIM_ERROR;
        }
        end_of_loop = false;
      }
    }
    /* Set SIM state in Data Cache */
    (void)dc_com_write(&dc_com_db, DC_CELLULAR_SIM_INFO, (void *)&cst_sim_info, sizeof(cst_sim_info));
  }
}

/**
  * @brief  PDN definition management
  * @param  -
  * @retval -
  */
static void CST_modem_define_pdn(uint8_t *apn, uint8_t *username, uint8_t *password)
{
  CS_PDN_configuration_t pdn_conf;
  uint32_t size;

  CS_Status_t cs_status;
  /* define user PDN configurations */

  /* common user name and password */
  (void)memset((void *)&pdn_conf, 0, sizeof(CS_PDN_configuration_t));

  if (crs_strlen(apn) == 0U)
    /* No apn provided, use the one username and password stored in EEPROM, and present */
    /* in data cache */
  {
    size =  crs_strlen(cst_cellular_params.sim_slot[cst_context.sim_slot_index].username) + 1U;
    if (size <= MAX_SIZE_USERNAME)
    {
      (void)memcpy((CRC_CHAR_t *)pdn_conf.username,
                   (CRC_CHAR_t *)cst_cellular_params.sim_slot[cst_context.sim_slot_index].username,
                   size);

      size =  crs_strlen(cst_cellular_params.sim_slot[cst_context.sim_slot_index].password) + 1U;
      if (size <= MAX_SIZE_USERNAME)
      {
        (void)memcpy((CRC_CHAR_t *)pdn_conf.password,
                     (CRC_CHAR_t *)cst_cellular_params.sim_slot[cst_context.sim_slot_index].password,
                     size);
      }
    }
  }
  else
    /* Username provided, use this one, and also the corresponding password */
  {
    (void)memcpy((CRC_CHAR_t *)pdn_conf.username,
                 (CRC_CHAR_t *)username,
                 crs_strlen(username) + 1U);
    (void)memcpy((CRC_CHAR_t *)pdn_conf.password,
                 (CRC_CHAR_t *)password,
                 crs_strlen(password) + 1U);
  }


  /* example for CS_PDN_USER_CONFIG_1 with access point name =  "PDN CONFIG 1" */
  if (crs_strlen(apn) == 0U)
  {
    cs_status = osCDS_define_pdn(cst_get_cid_value(cst_cellular_params.sim_slot[cst_context.sim_slot_index].cid),
                                 (const uint8_t *)cst_cellular_params.sim_slot[cst_context.sim_slot_index].apn,
                                 &pdn_conf);
  }
  else
  {
    cs_status = osCDS_define_pdn(cst_get_cid_value(cst_cellular_params.sim_slot[cst_context.sim_slot_index].cid),
                                 (const uint8_t *)apn,
                                 &pdn_conf);
  }


  if (cs_status != CELLULAR_OK)
  {
    CST_config_fail(((uint8_t *)"CST_modem_define_pdn"),
                    CST_MODEM_PDP_DEFINE_FAIL,
                    &cst_context.activate_pdn_reset_count,
                    CST_DEFINE_PDN_RESET_MAX);
  }
}

/**
  * @brief  configuration failure management
  * @param  msg_fail   - failure message (only for trace)
  * @param  fail_cause - failure cause
  * @param  fail_count - count of failures
  * @param  fail_max   - max of allowed failures
  * @retval -
  */
void CST_config_fail(const uint8_t *msg_fail, cst_fail_cause_t fail_cause, uint8_t *fail_count,
                     uint8_t fail_max)
{
#if (USE_TRACE_CELLULAR_SERVICE == 0)
  UNUSED(msg_fail);
#endif  /* (USE_TRACE_CELLULAR_SERVICE == 0) */

  PRINT_CELLULAR_SERVICE("=== %s Fail !!! === \r\n", msg_fail)
  ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 1, ERROR_WARNING);

  *fail_count = *fail_count + 1U;
  cst_context.global_retry_count++;
  cst_context.reset_count++;

  CST_data_cache_cellular_info_set(DC_SERVICE_OFF, NULL);
  if ((*fail_count <= fail_max) && (cst_context.global_retry_count <= CST_GLOBAL_RETRY_MAX))
  {
    /* maximal fail count not reached => restart automation */
    CST_set_state(CST_MODEM_RESET_STATE);
    CST_send_message(CST_MESSAGE_CS_EVENT, CST_MODEM_INIT_EVENT);
    cst_context.fail_cause    = fail_cause;
  }
  else
  {
    /* maximal fail count reached => stop cellular processing */
    CST_set_state(CST_MODEM_FAIL_STATE);
    CST_fail_setting();
    cst_context.fail_cause    = CST_MODEM_POWER_ON_FAIL;

    PRINT_CELLULAR_SERVICE_ERR("=== CST_set_fail_state %d - count %d/%d FATAL !!! ===\n\r",
                               fail_cause,
                               cst_context.global_retry_count,
                               *fail_count)
    ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 2, ERROR_FATAL);
  }
}

/**
  * @brief  sets current signal quality values in DC
  * @param  -
  * @retval CS_Status_t - error code
  */
CS_Status_t CST_set_signal_quality(void)
{
  CS_Status_t cs_status;
  CS_SignalQuality_t sig_quality;
  cs_status = CELLULAR_OK;

  if (osCS_get_signal_quality(&sig_quality) == CELLULAR_OK)
  {
    /* signal quality service available */
    cst_context.csq_count_fail = 0U;
    if ((sig_quality.rssi != cst_context.signal_quality.rssi) || (sig_quality.ber != cst_context.signal_quality.ber))
    {
      /* signal quality value has changed => update DC values */
      cst_context.signal_quality.rssi = sig_quality.rssi;
      cst_context.signal_quality.ber  = sig_quality.ber;

      (void)dc_com_read(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));

      if (sig_quality.rssi == CST_BAD_SIG_RSSI)
      {
        /* Wrong signal quality : not attached to network */
        cs_status = CELLULAR_ERROR;
        cst_cellular_info.cs_signal_level    = DC_NO_ATTACHED;
        cst_cellular_info.cs_signal_level_db = (int32_t)DC_NO_ATTACHED;
      }
      else
      {
        /* signal quality OK  */
        cs_status = CELLULAR_OK;
        cst_cellular_info.cs_signal_level     = sig_quality.rssi;                         /* range 0..99 */
        cst_cellular_info.cs_signal_level_db  = (-113 + (2 * (int32_t)sig_quality.rssi)); /* dBm value   */
      }
      (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));
    }

    PRINT_CELLULAR_SERVICE(" -Sig quality rssi : %d\n\r", sig_quality.rssi)
    PRINT_CELLULAR_SERVICE(" -Sig quality ber  : %d\n\r", sig_quality.ber)
  }
  else
  {
    /* signal quality service not available */
    cs_status = CELLULAR_ERROR;
    cst_context.csq_count_fail++;
    PRINT_CELLULAR_SERVICE("Modem signal quality error\n\r")
    if (cst_context.csq_count_fail >= CST_COUNT_FAIL_MAX)
    {
      PRINT_CELLULAR_SERVICE("Modem signal quality error max\n\r")
      cst_context.csq_count_fail = 0U;
      CST_config_fail(((uint8_t *)"CS_get_signal_quality"),
                      CST_MODEM_CSQ_FAIL,
                      &cst_context.csq_reset_count,
                      CST_CSQ_MODEM_RESET_MAX);
    }
  }
  return cs_status;
}

/**
  * @brief  subscribes to modem events
  * @param  -
  * @retval -
  */

void CST_subscribe_modem_events(void)
{
  PRINT_CELLULAR_SERVICE("Subscribe modems events\n\r")
  CS_ModemEvent_t events_mask = (CS_ModemEvent_t)((uint16_t)CS_MDMEVENT_BOOT       |
                                                  (uint16_t)CS_MDMEVENT_POWER_DOWN |
                                                  (uint16_t)CS_MDMEVENT_FOTA_START |
#if (USE_LOW_POWER == 1)
                                                  (uint16_t)CS_MDMEVENT_LP_ENTER   |
                                                  (uint16_t)CS_MDMEVENT_WAKEUP_REQ |
#endif  /* (USE_LOW_POWER == 1) */
                                                  (uint16_t)CS_MDMEVENT_FOTA_END);
  (void)osCDS_subscribe_modem_event(events_mask, CST_modem_event_callback);
}

/**
  * @brief  gets automaton event from message event
  * @param  event  - message event
  * @retval cst_autom_event_t - automaton event
  */
cst_autom_event_t CST_get_autom_event(cst_message_t event)
{
  static dc_cellular_target_state_t cst_target_state;            /* new target state requested   */

  cst_autom_event_t autom_event;
  autom_event = CST_NO_EVENT;

  /*  types of messages:
       -> CS automaton event
       -> CS CMD
       -> DC EVENT  (DC_CELLULAR_DATA_INFO: / FAIL)
  */
  if (GET_AUTOMATON_MSG_TYPE(event) == CST_MESSAGE_CS_EVENT)
  {
    /* Cellular Event */
    autom_event = (cst_autom_event_t)GET_AUTOMATON_MSG_ID(event);
  }
  else if (GET_AUTOMATON_MSG_TYPE(event) == CST_MESSAGE_CMD)
  {
    /* Command Event */
    autom_event = (cst_autom_event_t)GET_AUTOMATON_MSG_ID(event);
  }
  else if (GET_AUTOMATON_MSG_TYPE(event) == CST_MESSAGE_DC_EVENT)
  {
    /* Data Cache Event */
    if (GET_AUTOMATON_MSG_ID(event) == (uint16_t)DC_CELLULAR_DATA_INFO)
    {
      /* DC_CELLULAR_DATA_INFO entry event */
      (void)dc_com_read(&dc_com_db, DC_CELLULAR_DATA_INFO, (void *)&cst_cellular_data_info,
                        sizeof(cst_cellular_data_info));
      if (cst_cellular_data_info.rt_state == DC_SERVICE_FAIL)
      {
        autom_event = CST_CELLULAR_DATA_FAIL_EVENT;
      }
    }
    else if (GET_AUTOMATON_MSG_ID(event) == (uint16_t)DC_CELLULAR_TARGET_STATE_CMD)
    {
      /* New modem target state  request */
      (void)dc_com_read(&dc_com_db, DC_CELLULAR_TARGET_STATE_CMD, (void *)&cst_target_state, sizeof(cst_target_state));
      if (cst_target_state.rt_state == DC_SERVICE_ON)
      {
        cst_cellular_params.target_state = cst_target_state.target_state;
        autom_event = CST_TARGET_STATE_CMD_EVENT;
      }
    }
#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
    else if (GET_AUTOMATON_MSG_ID(event) == (uint16_t) DC_PPP_CLIENT_INFO)
    {
      /* PPP Event */
      dc_ppp_client_info_t ppp_client_info;
      (void)dc_com_read(&dc_com_db, DC_PPP_CLIENT_INFO, (void *)&ppp_client_info, sizeof(ppp_client_info));
      if (ppp_client_info.rt_state == DC_SERVICE_ON)
      {
        autom_event = CST_PPP_OPENED_EVENT;
      }
      else if (ppp_client_info.rt_state == DC_SERVICE_OFF)
      {
        autom_event = CST_PPP_CLOSED_EVENT;
      }
      else
      {
        CST_config_fail(((uint8_t *)"CST_get_autom_event"),
                        CST_PPP_FAIL,
                        &cst_context.ppp_fail_count,
                        CST_PPP_FAIL_MAX);
      }
    }
#endif /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */
    else
    {
      __NOP(); /* Nothing to do */
    }
  }
  else
  {
    __NOP(); /* Nothing to do */
  }
  return autom_event;
}

/**
  * @brief  gets network status
  * @param  -
  * @retval cst_network_status_t - network status
  */
cst_network_status_t  CST_get_network_status(void)
{
  CS_Status_t cs_status;
  CS_RegistrationStatus_t reg_status;
  cst_network_status_t ret;

  (void)memset((void *)&reg_status, 0, sizeof(reg_status));

  cs_status = osCDS_get_net_status(&reg_status);
  if (cs_status == CELLULAR_OK)
  {
    /* service available */
    cst_context.current_EPS_NetworkRegState  = reg_status.EPS_NetworkRegState;
    cst_context.current_GPRS_NetworkRegState = reg_status.GPRS_NetworkRegState;
    cst_context.current_CS_NetworkRegState   = reg_status.CS_NetworkRegState;

    if ((cst_context.current_EPS_NetworkRegState  != CS_NRS_REGISTERED_HOME_NETWORK)
        && (cst_context.current_EPS_NetworkRegState  != CS_NRS_REGISTERED_ROAMING)
        && (cst_context.current_GPRS_NetworkRegState != CS_NRS_REGISTERED_HOME_NETWORK)
        && (cst_context.current_GPRS_NetworkRegState != CS_NRS_REGISTERED_ROAMING))
    {
      if (cst_context.current_CS_NetworkRegState == CS_NRS_UNKNOWN)
      {
        ret = CST_NET_UNKNOWN ;
      }
      else
      {
        /* network not registered */
        ret = CST_NOT_REGISTERED;
      }
    }
    else /* device registered to network */
    {
      /* network registered */
      ret = CST_NET_REGISTERED;
      if (((uint16_t)reg_status.optional_fields_presence & (uint16_t)CS_RSF_FORMAT_PRESENT) != 0U)
      {
        /* update DC */
        (void)dc_com_read(&dc_com_db,  DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));
        (void)memcpy(cst_cellular_info.mno_name, reg_status.operator_name, DC_MAX_SIZE_MNO_NAME - 1U);
        cst_cellular_info.mno_name[DC_MAX_SIZE_MNO_NAME - 1U] = 0;  /* to avoid a non null terminated string */
        cst_cellular_info.rt_state = DC_SERVICE_ON;
        cst_cellular_info.access_techno = (dc_access_techno_t)reg_status.AcT;
        (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));
        PRINT_CELLULAR_SERVICE(" ->operator_name = %s", reg_status.operator_name)
      }
    }
  }
  else
  {
    /* service not available */
    cst_context.current_EPS_NetworkRegState  = CS_NRS_NOT_REGISTERED_SEARCHING;
    cst_context.current_GPRS_NetworkRegState = CS_NRS_NOT_REGISTERED_SEARCHING;
    cst_context.current_CS_NetworkRegState   = CS_NRS_NOT_REGISTERED_SEARCHING;
    ret = CST_NET_STATUS_ERROR;
  }
  return ret;
}





/**
  * @brief  setting FAIL mode in DC
  * @param  -
  * @retval -
  */
static void CST_fail_setting(void)
{
  (void)dc_com_read(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(dc_cellular_info_t));
  (void)dc_com_read(&dc_com_db, DC_CELLULAR_DATA_INFO, (void *)&cst_cellular_data_info,
                    sizeof(dc_cellular_data_info_t));
  cst_cellular_info.rt_state      = DC_SERVICE_FAIL;
  cst_cellular_data_info.rt_state = DC_SERVICE_FAIL;
  (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(dc_cellular_info_t));
  (void)dc_com_write(&dc_com_db, DC_CELLULAR_DATA_INFO, (void *)&cst_cellular_data_info,
                     sizeof(dc_cellular_data_info_t));
}

/* ===================================================================
   UTility functions  BEGIN
   =================================================================== */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
