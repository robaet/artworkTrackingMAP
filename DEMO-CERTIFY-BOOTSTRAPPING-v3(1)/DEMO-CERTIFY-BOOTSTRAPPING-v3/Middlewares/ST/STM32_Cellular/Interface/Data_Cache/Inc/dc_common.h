/**
  ******************************************************************************
  * @file    dc_common.h
  * @author  MCD Application Team
  * @brief   Header for dc_common.c module
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
#ifndef DC_COMMON_H
#define DC_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/**
  ******************************************************************************
  @verbatim
  ==============================================================================
                    ##### How to use Data Cache module #####
  ==============================================================================

  Data Cache allows the sharing of data and events by X-Cube-Cellular components.

  Data Cache contains Entries. Each Entry is a set of data associated to a service.

  A software component (producer) creates a data entry and writes data in it.
  Each data entry is associated to an identifier.
  Creation of a data entry is done by calling dc_com_register_serv() service.
  Update of data entry value is done by calling dc_com_write() service.

  The other components (consumers) can read the data by means of the identifier.
  A component can subscribe a callback in order to be informed
  when a Data Cache data entry has been updated.
  Subscription is done through dc_com_register_gen_event_cb() service
  Read of data entry value is done by calling dc_com_read() service.

  The Data Cache structure includes the rt_state field.
  This field contains the state of service and the validity of entry data.
  e.g:
    - DC_SERVICE_UNAVAIL: field values of structure not significant
    - DC_SERVICE_ON: service started (field values of structure are significant)
    - Other values are entry dependent (for possible values refer to dc_service_rt_state_t)

  ---------------------------
  Cellular Data Cache Entries
  ---------------------------
  Several Data Cache entries allows an application to get cellular information/notifications
     (cellular configuration, cellular status, sim configuration and status,...)
     See Cellular Management module to find the complete description of Cellular Data Cache Entries

  --------
  Example:
  --------
  Find below a code example of Data Cache produced and consumer.
  This example show how
   - a Data Cache producer must initialize its Data Cache entry and how to produce it
   - a Data Cache consumer must register to the service and to read an entry

  -------------------------------
  Example of Data Cache producer:
  -------------------------------

    * Data Dache Entry definition of publisher example (initialized at invalid state)
    dc_com_res_id_t DC_PRODUCER_EXAMPLE_ENTRY = DC_COM_INVALID_ENTRY;

    * Structure definition associated to DC_PRODUCER_EXAMPLE_ENTRY Data Cache entry.
    * This structure contains produced values

    typedef struct
    {
      * the header structure  must be inserted at the top of the data cache structure
      dc_service_rt_header_t header;

      * state of the entry:
      * - DC_SERVICE_UNAVAIL: service not initialized (the field values of structure are not significant)
      * - DC_SERVICE_ON: service available (field values of structure are significant)
      * - Other state values not used in this example.
      dc_service_rt_state_t rt_state;

      * producer values
      uint32_t     example_value_1;
      uint32_t     example_value_2;
    } dc_producer_example_struct_t;

    * Initialization of data cache entry (of producer example)
    static void dc_produced_example_init(void)
    {
      * Data Cache internal structure declaration
      static dc_producer_example_struct_t     dc_producer_example_struct;

      * structure set to 0
      (void)memset((void *)&dc_producer_example_struct,         0, sizeof(dc_producer_example_struct_t));

      * Data Cache entry structure registration
      DC_PRODUCER_EXAMPLE_ENTRY = dc_com_register_serv(&dc_com_db, (void *)&dc_producer_example_struct,
                                                        (uint16_t)sizeof(dc_producer_example_struct_t));
    }

    * Production of data cache entry (of producer example)
    static void dc_produced_example_production(uint32_t value1, uint32_t value2)
    {
      static dc_producer_example_struct_t producer_example_struct;
      producer_example_struct.rt_state       = DC_SERVICE_ON;
      producer_example_struct.example_value_1 = value1;
      producer_example_struct.example_value_2 = value2;
      (void)dc_com_write(&dc_com_db, DC_PRODUCER_EXAMPLE_ENTRY, (void *)&producer_example_struct,
                         sizeof(dc_producer_example_struct_t));
    }

  -------------------------------
  Example of Data Cache customer:
  -------------------------------
    * data cache consumer example initialization
    static void dc_consumer_example_start(void)
    {
      * Data Cache entry structure registration
      * registration allows consumer to be notified when an entry is produced in Data Cache
      (void)dc_com_register_gen_event_cb(&dc_com_db, dc_consumer_example_notif_callback, (void *) NULL);
    }

    ---------------------------------------------------------------------------------------
    NOTE: if a Data Cache consumer read periodically an Entry it has no need to be notified
          and has no need to be registered
    ---------------------------------------------------------------------------------------

    * data cache consumer example notification callback
    * this callback is called when a producer write data in Data Cache
    static void dc_consumer_example_notif_callback(dc_com_event_id_t dc_event_id,
                                                   const void *p_private_gui_data)
    {
      UNUSED(p_private_gui_data);

      * test the Data Cache Id of produced Entry
      if (dc_event_id == DC_PRODUCER_EXAMPLE_ENTRY)
      {
        * DC_PRODUCER_EXAMPLE_ENTRY updated
        * read the entry in Data Cache
        dc_producer_example_struct_t  producer_example_struct;
        (void)dc_com_read(&dc_com_db, DC_PRODUCER_EXAMPLE_ENTRY,
                          (void *)&producer_example_struct,
                          sizeof(dc_producer_example_struct_t));
        if (producer_example_struct.rt_state == DC_SERVICE_ON)
        {
          static uint32_t     example_value_1;
          static uint32_t     example_value_2;

          * Data available
          value1 = producer_example_struct.example_value_1;
          value2 = producer_example_struct.example_value_2;
          * value processing
          * NOTE: this processing is executed in producer thread context
          *       For an heavy processing it is better to post values in a queue to wakeup the consumer thread
          ...
        }
    }

  @endverbatim
  */

/** @defgroup DC DC: Data Cache module
  * @{
  */

/**
  * @}
  */


/** @addtogroup DC
  * @{
  */

/** @defgroup DC_COMMON Common services
  * @{
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup DC_COMMON_Constants Constants
  * @{
  */

/** @brief Number max of Data Cache subscriber. */
/* According to dc_com_reg_id_t type, and DC_COM_INVALID_ENTRY error code, should not exceed 254 */
#define DC_COM_MAX_NB_SUBSCRIBER 10U

#if USE_BOARD_BUTTONS == 1
#define DC_BOARD_ENTRIES 7U
#else  /* #if USE_BOARD_BUTTONS == 1 */
#define DC_BOARD_ENTRIES 0U
#endif /* #if USE_BOARD_BUTTONS == 1 */

#if USE_DC_MEMS == 1
#define DC_MEMS_ENTRIES 8U
#else  /* #if USE_DC_MEMS == 1 */
#define DC_MEMS_ENTRIES 0U
#endif /* #if USE_DC_MEMS == 1 */

#if USE_SIMU_MEMS == 1
#define DC_SIMU_MEMS_ENTRIES 8U
#else  /* #if USE_SIMU_MEMS == 1 */
#define DC_SIMU_MEMS_ENTRIES 0U
#endif /* #if USE_SIMU_MEMS == 1 */

#if USE_DC_GENERIC == 1
#define DC_GENERIC_ENTRIES 12U
#else  /* #if USE_DC_GENERIC == 1 */
#define DC_GENERIC_ENTRIES 0U
#endif /* #if USE_DC_GENERIC == 1 */

#define DC_CELLULAR_CORE_ENTRIES 12U

/** @brief Number max of Data Cache entries */
#define DC_COM_ENTRY_MAX_NB   (DC_BOARD_ENTRIES + \
                               DC_MEMS_ENTRIES + \
                               DC_SIMU_MEMS_ENTRIES + \
                               DC_GENERIC_ENTRIES + \
                               DC_CELLULAR_CORE_ENTRIES)

/** @brief Invalid entry: at creation, the Data Cache entries must be initialized with this value  */
#define DC_COM_INVALID_ENTRY  0xFFU

/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup DC_COMMON_Types Types
  * @{
  */

typedef uint8_t dc_com_reg_id_t;   /*!< type of identifier of registered consumer */

typedef uint16_t dc_com_res_id_t;   /*!< type of identifier of the registered Data Cache entry */

typedef uint8_t dc_com_event_id_t;  /*!< type of Data Cache event */

typedef bool  dc_com_status_t;   /*!< type of Data Cache return code  */
#define   DC_COM_OK     (dc_com_status_t)true   /*!< Ok  */
#define   DC_COM_ERROR  (dc_com_status_t)false  /*!< Error  */

/** @brief type of entry/service state */
typedef enum
{
  DC_SERVICE_UNAVAIL = 0x00, /*!< Service is unavailable. HW and/or SW driver are not present. */
  DC_SERVICE_RESET,          /*!< Service is resetting.
                                  When reset is complete, the Service enters in ON or READY state */
  DC_SERVICE_CALIB,          /*!< Service is under calibration */
  DC_SERVICE_OFF,            /*!< Service is OFF */
  DC_SERVICE_SHUTTING_DOWN,  /*!<  Service is shutting down */
  DC_SERVICE_STARTING,       /*!< Service is starting but not fully operational */
  DC_SERVICE_RUN,            /*!< Service is ON (functional) but not Calibrated or not Initialized */
  DC_SERVICE_ON,             /*!< Service is ON and fully operational and calibrated */
  DC_SERVICE_FAIL            /*!< Service is Failed */
} dc_service_rt_state_t;
/**
  * @}
  */

/** @brief type of Data Cache structure header */
typedef struct
{
  dc_com_res_id_t res_id;
  uint32_t size;
} dc_service_rt_header_t;


/** @brief type of Data Cache notification callback */
typedef void (*dc_com_gen_event_callback_t)(
  const dc_com_event_id_t event_id,      /*!< Event ID */
  const void *private_consumer_data);        /*!< private consumer Data (p_private_data parameter
                                                           of dc_com_register_gen_event_cb)  */

/** @brief type of Data Cache notification callback (Data Cache internal use)  */
typedef struct
{
  dc_com_reg_id_t consumer_reg_id;
  dc_com_gen_event_callback_t notif_cb;
  const void *private_consumer_data;
} dc_com_consumer_info_t;

/** @brief type of Data Cache global structure (Data Cache internal use) */
typedef struct
{
  dc_com_reg_id_t   consumer_number;
  dc_com_res_id_t   serv_number;
  dc_com_consumer_info_t consumer_info[DC_COM_MAX_NB_SUBSCRIBER];
  void *p_dc_db[DC_COM_ENTRY_MAX_NB];
  uint16_t dc_db_len[DC_COM_ENTRY_MAX_NB];
} dc_com_db_t;

/**
  * @}
  */

/* External variables --------------------------------------------------------*/
/** @defgroup DC_COMMON_Variables Variables
  * @{
  */

extern dc_com_db_t dc_com_db; /*!< Data Cache Database Identifier */

/**
  * @}
  */


/* Exported macros -----------------------------------------------------------*/
/** @defgroup DC_COMMON_Macros Macros
  * @{
  */

/**
  * @}
  */

/* Exported functions ------------------------------------------------------- */
/** @defgroup DC_COMMON_Functions Functions
  * @brief    Data cache entry management (Register, Read, Write)
  * @{
  */

/**
  * @brief  Allow a Data Cache producer to register to a new entry/service.
  * @param  p_dc_db         - reference to the Data Cache used. Must be set to &dc_com_db
  * @param  p_data          - address of the structure associated with Data Cache Entry.
  * @note                     this structure must be a persistent variable.
  * @param size             - size of p_data structure.
  * @retval dc_com_res_id_t - returns the identifier of the registered Data Cache entry
  */
dc_com_res_id_t dc_com_register_serv(dc_com_db_t *p_dc_db, void *p_data, uint16_t size);

/**
  * @brief  Allow a consumer to register to the Data Cache notifications.
  * @param  p_dc_db         - data base reference (Must be set to &dc_com_db)
  * @param  notif_cb        - address of callback.
  * @note                     This callback is called when a Data Cache event
  *                           is sent by a call to dc_com_write_event.
  *                           The callback is executed in the writing thread context.
  * @param  p_private_data  - address of consumer private context (optional).
  * @note                     This address is passed as a parameter of the callback
  * @retval dc_com_reg_id_t - return the identifier of the registered consumer or
  *                           DC_COM_INVALID_ENTRY in case of error
  */
dc_com_reg_id_t dc_com_register_gen_event_cb(dc_com_db_t *p_dc_db, dc_com_gen_event_callback_t notif_cb,
                                             const void *p_private_data);

/**
  * @brief  Allow a Data Cache producer to update data associated to a Data Cache entry.
  * @param  p_dc            - data base reference (Must be set to &dc_com_db)
  * @param  res_id          - entry/resource id
  * @param  p_data          - data to write
  * @param  len             - length of p_data to write
  * @retval dc_com_status_t - return status with DC_COM_OK or DC_COM_ERROR
  */
dc_com_status_t dc_com_write(dc_com_db_t *p_dc, dc_com_res_id_t res_id, const void *p_data, uint32_t len);

/**
  * @brief  Allow a consumer to read the currents data associated to a Data Cache entry.
  * @param  p_dc            - data base reference (Must be set to &dc_com_db)
  * @param  res_id          - entry/resource id
  * @param  p_data          - data to read
  * @param  len             - length of p_data to read
  * @retval dc_com_status_t - return status with DC_COM_OK or DC_COM_ERROR
  */
dc_com_status_t dc_com_read(dc_com_db_t *p_dc, dc_com_res_id_t res_id, void *p_data, uint32_t len);


/**
  * @}
  */

/**
  * @}
  */


/*** Component Initialization/Start *******************************************/
/*** Used by Cellular-Service - Not an consumer Interface *************************/

/**
  * @brief  Initialize the Data Cache module.
  * @param  p_dc - data base reference (Must be set to &dc_com_db)
  * @retval -
  */
void dc_com_init(dc_com_db_t *p_dc);

/**
  * @brief  Start Data Cache module.
  * @param  p_dc - data base reference (Unused)
  * @retval -
  */
void dc_com_start(dc_com_db_t *p_dc);

/**
  * @brief  Send an event to DC.
  * @param  p_dc            - data base reference (Must be set to &dc_com_db)
  * @param  event_id        - event id
  * @retval dc_com_status_t - return status with DC_COM_OK or DC_COM_ERROR
  */
dc_com_status_t dc_com_write_event(dc_com_db_t *p_dc, dc_com_event_id_t event_id);


#ifdef __cplusplus
}
#endif


#endif /* DC_COMMON_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
