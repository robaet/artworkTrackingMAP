/**
  ******************************************************************************
  * @file    ipc_common.c
  * @author  MCD Application Team
  * @brief   This file provides common code for IPC
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
#include <string.h>
#include "ipc_common.h"
#if (IPC_USE_UART == 1U)
#include "ipc_uart.h"
#endif /* (IPC_USE_UART == 1U) */

/* Private typedef -----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/
IPC_ClientDescription_t IPC_DevicesList[IPC_MAX_DEVICES];

/* Private function prototypes -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  The function initialize the global variables of the IPC.
  * @param  device IPC device identifier.
  * @param  itf_type Type of the interface.
  * @param  hitf Handle to the HAL structure of the interface.
  * @retval status
  */
IPC_Status_t IPC_init(IPC_Device_t device, IPC_Interface_t itf_type, void *hitf)
{
  IPC_Status_t status;

  /* check that device value is valid */
  if (device >= IPC_MAX_DEVICES)
  {
    /* IPC device id exceed maximum size defined */
    status = IPC_ERROR;
  }
  else if (hitf == NULL)
  {
    status = IPC_ERROR;
  }
  else
  {
    if (itf_type == IPC_INTERFACE_UART)
    {
#if (IPC_USE_UART == 1U)
      status = IPC_UART_init(device, (UART_HandleTypeDef *) hitf);
#else
      status = IPC_ERROR;
#endif /* IPC_USE_UART == 1U */
    }
    else if (itf_type == IPC_INTERFACE_SPI)
    {
      status = IPC_ERROR;
    }
    else
    {
      /* interface not supported yet */
      status = IPC_ERROR;
    }
  }
  return (status);
}

/**
  * @brief  Reinitialize the IPC global variables.
  * @param  device IPC device identifier.
  * @retval status
  */
IPC_Status_t IPC_deinit(IPC_Device_t  device)
{
  IPC_Status_t status;

  if (device >= IPC_MAX_DEVICES)
  {
    /* IPC device id exceed maximum size defined */
    status = IPC_ERROR;
  }
  else
  {
    status = IPC_UART_deinit(device);
  }

  return (status);
}

/**
  * @brief  Open a specific channel.
  * @param  hipc IPC handle to open.
  * @param  device IPC device identifier.
  * @param  mode IPC mode (char or stream).
  * @param  pRxClientCallback Callback ptr called when a message has been received.
  * @param  pTxClientCallback Callback ptr called when a message has been send.
  * @param  pCheckEndOfMsg Callback ptr to the function used to analyze if char received is a termination char
  * @retval status
  */
IPC_Status_t IPC_open(IPC_Handle_t *hipc,
                      IPC_Device_t  device,
                      IPC_Mode_t    mode,
                      IPC_RxCallbackTypeDef pRxClientCallback,
                      IPC_TxCallbackTypeDef pTxClientCallback,
                      IPC_CheckEndOfMsgCallbackTypeDef pCheckEndOfMsg)
{
  IPC_Status_t status;

  /* check pointers */
  if ((hipc == NULL) ||
      (pRxClientCallback == NULL) ||
      (pTxClientCallback == NULL))
  {
    status = IPC_ERROR;
  }
  else if (device >= IPC_MAX_DEVICES)
  {
    /* IPC device id exceed maximum size defined */
    status = IPC_ERROR;
  }
  /* check if the device has been correctly initialized */
  else if (IPC_DevicesList[device].state != IPC_STATE_INITIALIZED)
  {
    status = IPC_ERROR;
  }
  else
  {
    if (IPC_DevicesList[device].phy_int.interface_type == IPC_INTERFACE_UART)
    {
#if (IPC_USE_UART == 1U)
      status = IPC_UART_open(hipc, device, mode, pRxClientCallback, pTxClientCallback, pCheckEndOfMsg);
#else
      status = IPC_ERROR;
#endif /* IPC_USE_UART == 1U */
    }
    else
    {
      status = IPC_ERROR;
    }
  }

  return (status);
}

/**
  * @brief  Close a specific channel.
  * @param  hipc IPC handle to close.
  * @retval status
  */
IPC_Status_t IPC_close(IPC_Handle_t *hipc)
{
  IPC_Status_t status;

  if (hipc != NULL)
  {
    status = IPC_UART_close(hipc);
  }
  else
  {
    status = IPC_ERROR;
  }

  return (status);
}

/**
  * @brief  Reset a specific channel.
  * @param  hipc IPC handle to reset.
  * @retval status
  */
IPC_Status_t IPC_reset(IPC_Handle_t *hipc)
{
  IPC_Status_t status;

  if (hipc != NULL)
  {
    status = IPC_UART_reset(hipc);
  }
  else
  {
    status = IPC_ERROR;
  }

  return (status);
}

/**
  * @brief  Abort transaction on a specific channel.
  * @param  hipc IPC handle to abort.
  * @retval status
  */
IPC_Status_t IPC_abort(IPC_Handle_t *hipc)
{
  IPC_Status_t status;

  if (hipc != NULL)
  {
    status = IPC_UART_abort(hipc);
  }
  else
  {
    status = IPC_ERROR;
  }

  return (status);
}

/**
  * @brief  Select current channel.
  * @param  hipc IPC handle to select.
  * @retval status
  */
IPC_Status_t IPC_select(IPC_Handle_t *hipc)
{
  IPC_Status_t status;

  if (hipc != NULL)
  {
    status = IPC_UART_select(hipc);
  }
  else
  {
    status = IPC_ERROR;
  }

  return (status);
}

/**
  * @brief  Get other channel handle if exists.
  * @param  hipc IPC handle.
  * @retval IPC_Handle_t*
  */
IPC_Handle_t *IPC_get_other_channel(IPC_Handle_t *hipc)
{
  return (IPC_UART_get_other_channel(hipc));
}

/**
  * @brief  Send data over a channel.
  * @param  hipc IPC handle.
  * @param  p_TxBuffer Pointer to the data buffer to transfer.
  * @param  bufsize Length of the data buffer.
  * @retval status
  */
IPC_Status_t IPC_send(IPC_Handle_t *hipc, uint8_t *p_TxBuffer, uint16_t bufsize)
{
  IPC_Status_t status;

  if ((hipc != NULL) && (p_TxBuffer != NULL))
  {
    status = IPC_UART_send(hipc, p_TxBuffer, bufsize);
  }
  else
  {
    status = IPC_ERROR;
  }

  return (status);
}

/**
  * @brief  Receive a message from a channel.
  * @param  hipc IPC handle.
  * @param  p_msg Pointer to the IPC message structure to fill with received message.
  * @retval status
  */
IPC_Status_t IPC_receive(IPC_Handle_t *hipc, IPC_RxMessage_t *p_msg)
{
  IPC_Status_t status;

  if (hipc != NULL)
  {
    status = IPC_UART_receive(hipc, p_msg);
  }
  else
  {
    status = IPC_ERROR;
  }

  return (status);
}

/**
  * @brief  Receive a data buffer from a channel.
  * @param  hipc IPC handle.
  * @param  p_buffer Pointer to the data buffer to transfer.
  * @param  p_len Length of the data buffer.
  * @note   p_len value received from user indicates the maximum user buffer length
  *         and p_len is updated with received buffer size.
  * @retval status
  */
IPC_Status_t IPC_streamReceive(IPC_Handle_t *hipc, uint8_t *p_buffer, int16_t *p_len)
{
#if (IPC_USE_STREAM_MODE == 1U)
  IPC_Status_t status;

  if ((hipc != NULL) && (p_buffer != NULL))
  {
    status = IPC_UART_streamReceive(hipc, (uint8_t *)p_buffer, p_len);
  }
  else
  {
    status = IPC_ERROR;
  }

  return (status);
#else
  UNUSED(hipc);
  UNUSED(p_buffer);
  UNUSED(p_len);
  return (IPC_ERROR);
#endif  /* IPC_USE_STREAM_MODE == 1U */
}

/**
  * @brief  Dump content of IPC Rx queue (for debug purpose).
  * @param  hipc IPC handle.
  * @param  readable If equal 1, print special characters explicitly (<CR>, <LF>, <NULL>).
  * @retval none
  */
void IPC_DumpRXQueue(IPC_Handle_t *hipc, uint8_t readable)
{
#if (DBG_IPC_RX_FIFO == 1U)
  if (hipc != NULL)
  {
    IPC_UART_DumpRXQueue(hipc, readable);
  }
#else
  UNUSED(hipc);
  UNUSED(readable);
  __NOP();
#endif /* DBG_IPC_RX_FIFO */
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

