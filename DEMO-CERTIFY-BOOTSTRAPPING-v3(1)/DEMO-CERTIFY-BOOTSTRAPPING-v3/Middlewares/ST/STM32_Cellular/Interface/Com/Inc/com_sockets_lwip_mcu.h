/**
  ******************************************************************************
  * @file    com_sockets_lwip_mcu.h
  * @author  MCD Application Team
  * @brief   Header for com_sockets_lwip_mcu.c module
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
#ifndef COM_SOCKETS_LWIP_MCU_H
#define COM_SOCKETS_LWIP_MCU_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

#include "com_common.h"
#include "com_sockets_addr_compat.h"

/* Exported constants --------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* External variables --------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

/** @addtogroup COM_SOCKETS_Functions
  * @{
  */

/**
  ******************************************************************************
  @verbatim
  ==============================================================================
                    ##### How to use Com Sockets interface #####
  ==============================================================================
  IP stack can be at two different places:
  1) on MCU side : LwIP (when USE_SOCKETS_TYPE = USE_SOCKETS_LWIP)
  2) on Modem side : Modem socket (when USE_SOCKETS_TYPE = USE_SOCKETS_MODEM))
  com_socket modules offers a generic interface whatever the configuration selected

  in Application call:
  generic com_socket services (e.g com_socket()) rather than
  com_socket_ip_modem() or com_socket_lwip_mcu() to be independent of the USE_SOCKETS_TYPE value

  When LwIP is on MCU side, check the options in lwipopts.h to activated /deactivate / configure features
  e.g: client/server support, protocol, IPv4 / IPv6 ...

  @endverbatim

  */

/** @defgroup COM_SOCKETS_Functions_LWIP_MCU LwIP MCU Socket services
  * @{
  */


/*** Socket management ********************************************************/

/**
  * @brief  Socket handle creation
  * @note   Create a communication endpoint called socket
  *         Restrictions, if any, are linked to LwIP module used
  * @param  family   - address family
  * @param  type     - connection type
  * @param  protocol - protocol type
  * @retval int32_t  - socket handle or error value
  */
int32_t com_socket_lwip_mcu(int32_t family, int32_t type, int32_t protocol);

/**
  * @brief  Socket option set
  * @note   Set option for the socket
  *         Restrictions, if any, are linked to LwIP module used
  * @param  sock      - socket handle obtained with com_socket
  * @param  level     - level at which the option is defined
  * @param  optname   - option name for which the value is to be set
  * @param  optval    - pointer to the buffer containing the option value
  * @param  optlen    - size of the buffer containing the option value
  * @retval int32_t   - ok or error value
  */
int32_t com_setsockopt_lwip_mcu(int32_t sock, int32_t level, int32_t optname,
                                const void *optval, int32_t optlen);

/**
  * @brief  Socket option get
  * @note   Get option for a socket
  *         Restrictions, if any, are linked to LwIP module used
  * @param  sock      - socket handle obtained with com_socket
  * @param  level     - level at which option is defined
  * @param  optname   - option name for which the value is requested
  * @param  optval    - pointer to the buffer that will contain the option value
  * @param  optlen    - size of the buffer that will contain the option value
  * @retval int32_t   - ok or error value
  */
int32_t com_getsockopt_lwip_mcu(int32_t sock, int32_t level, int32_t optname,
                                void *optval, int32_t *optlen);

/**
  * @brief  Socket bind
  * @note   Assign a local address and port to a socket
  *         Restrictions, if any, are linked to LwIP module used
  * @param  sock      - socket handle obtained with com_socket
  * @param  addr      - local IP address and port
  * @param  addrlen   - addr length
  * @retval int32_t   - ok or error value
  */
int32_t com_bind_lwip_mcu(int32_t sock,
                          const com_sockaddr_t *addr, int32_t addrlen);

/**
  * @brief  Socket listen
  * @note   Set socket in listening mode
  *         Restrictions, if any, are linked to LwIP module used
  * @param  sock      - socket handle obtained with com_socket
  * @param  backlog   - number of connection requests that can be queued
  * @retval int32_t   - ok or error value
  */
int32_t com_listen_lwip_mcu(int32_t sock,
                            int32_t backlog);

/**
  * @brief  Socket accept
  * @note   Accept a connect request for a listening socket
  *         Restrictions, if any, are linked to LwIP module used
  * @param  sock      - socket handle obtained with com_socket
  * @param  addr      - IP address and port number of the accepted connection
  * @param  addrlen   - addr length
  * @retval int32_t   - ok or error value
  */
int32_t com_accept_lwip_mcu(int32_t sock,
                            com_sockaddr_t *addr, int32_t *addrlen);

/**
  * @brief  Socket connect
  * @note   Connect socket to a remote host
  *         Restrictions, if any, are linked to LwIP module used
  * @param  sock      - socket handle obtained with com_socket
  * @param  addr      - remote IP address and port
  * @param  addrlen   - addr length
  * @retval int32_t   - ok or error value
  */
int32_t com_connect_lwip_mcu(int32_t sock,
                             const com_sockaddr_t *addr, int32_t addrlen);

/**
  * @brief  Socket send data
  * @note   Send data on already connected socket
  *         Restrictions, if any, are linked to LwIP module used
  * @param  sock      - socket handle obtained with com_socket
  * @param  buf       - pointer to application data buffer to send
  * @param  len       - length of the data to send (in bytes)
  * @param  flags     - options
  * @retval int32_t   - number of bytes sent or error value
  */
int32_t com_send_lwip_mcu(int32_t sock,
                          const com_char_t *buf, int32_t len,
                          int32_t flags);

/**
  * @brief  Socket send to data
  * @note   Send data to a remote host
  *         Restrictions, if any, are linked to LwIP module used
  * @param  sock      - socket handle obtained with com_socket
  * @param  buf       - pointer to application data buffer to send
  * @param  len       - length of the data to send (in bytes)
  * @param  flags     - options
  * @param  to        - remote IP address and port number
  * @param  tolen     - remote IP length
  * @retval int32_t   - number of bytes sent or error value
  */
int32_t com_sendto_lwip_mcu(int32_t sock,
                            const com_char_t *buf, int32_t len,
                            int32_t flags,
                            const com_sockaddr_t *to, int32_t tolen);

/**
  * @brief  Socket receive data
  * @note   Receive data on already connected socket
  *         Restrictions, if any, are linked to LwIP module used
  * @param  sock      - socket handle obtained with com_socket
  * @param  buf       - pointer to application data buffer to store the data to
  * @param  len       - size of application data buffer (in bytes)
  * @param  flags     - options
  * @retval int32_t   - number of bytes received or error value
  */
int32_t com_recv_lwip_mcu(int32_t sock,
                          com_char_t *buf, int32_t len,
                          int32_t flags);

/**
  * @brief  Socket receive from data
  * @note   Receive data from a remote host
  *         Restrictions, if any, are linked to LwIP module used
  * @param  sock      - socket handle obtained with com_socket
  * @param  buf       - pointer to application data buffer to store the data to
  * @param  len       - size of application data buffer (in bytes)
  * @param  flags     - options
  * @param  from      - remote IP address and port number
  * @param  fromlen   - remote IP length
  * @retval int32_t   - number of bytes received or error value
  */
int32_t com_recvfrom_lwip_mcu(int32_t sock,
                              com_char_t *buf, int32_t len,
                              int32_t flags,
                              com_sockaddr_t *from, int32_t *fromlen);

/**
  * @brief  Socket close
  * @note   Close a socket and release socket handle
  *         Restrictions, if any, are linked to LwIP module used
  * @param  sock      - socket handle obtained with com_socket
  * @retval int32_t   - ok or error value
  */
int32_t com_closesocket_lwip_mcu(int32_t sock);

/**
  * @}
  */

/** @defgroup COM_SOCKETS_Functions_LWIP_MCU_Other LwIP MCU Other Socket services
  * @{
  */

/*** Other functionalities ****************************************************/

/**
  * @brief  Get host IP from host name
  * @note   Retrieve host IP address from host name
  *         Restrictions, if any, are linked to LwIP module used
  * @param  name      - host name
  * @param  addr      - host IP corresponding to host name
  * @retval int32_t   - ok or error value
  */
int32_t com_gethostbyname_lwip_mcu(const com_char_t *name,
                                   com_sockaddr_t   *addr);

/**
  * @brief  Get peer name
  * @note   Retrieve IP address and port number
  *         Restrictions, if any, are linked to LwIP module used
  * @param  sock      - socket handle obtained with com_socket
  * @param  name      - IP address and port number of the peer
  * @param  namelen   - name length
  * @retval int32_t   - ok or error value
  */
int32_t com_getpeername_lwip_mcu(int32_t sock,
                                 com_sockaddr_t *name, int32_t *namelen);

/**
  * @brief  Get sock name
  * @note   Retrieve local IP address and port number
  *         Restrictions, if any, are linked to LwIP module used
  * @param  sock      - socket handle obtained with com_socket
  * @param  name      - IP address and port number
  * @param  namelen   - name length
  * @retval int32_t   - ok or error value
  */
int32_t com_getsockname_lwip_mcu(int32_t sock,
                                 com_sockaddr_t *name, int32_t *namelen);

/**
  * @}
  */

/**
  * @}
  */


#if (USE_COM_PING == 1)

/** @addtogroup COM_PING_Functions
  * @{
  */

/**
  ******************************************************************************
  @verbatim
  ==============================================================================
                    ##### How to use Com Ping interface #####
  ==============================================================================
  IP stack can be at two different places:
  1) on MCU side : LwIP (when USE_SOCKETS_TYPE = USE_SOCKETS_LWIP)
  2) on Modem side : Modem socket (when USE_SOCKETS_TYPE = USE_SOCKETS_MODEM))
  com_socket modules offers a generic interface whatever the configuration selected

  in Application call:
  generic com_ping services (e.g com_ping()) rather than
  com_ping_ip_modem() or com_ping_lwip_mcu() to be independent of the USE_SOCKETS_TYPE value

  When LwIP is on MCU side, to use com_ping services ICMP options are needed (see lwipopts.h)
  (define USE_COM_PING must also be activated)

  @endverbatim

  */

/** @defgroup COM_PING_Functions_LWIP_MCU LwIP MCU Ping services
  * @{
  */

/*** PING functionalities *****************************************************/

/**
  * @brief  Ping handle creation
  * @note   Create a ping session
  * @param  -
  * @retval int32_t  - ping handle or error value
  */
int32_t com_ping_lwip_mcu(void);

/**
  * @brief  Ping process request
  * @note   Process a ping
  * @param  ping     - ping handle obtained with com_ping
  * @param  addr     - remote IP address and port
  * @param  addrlen  - addr length
  * @param  timeout  - timeout for ping response (in sec)
  * @param  rsp      - ping response
  * @retval int32_t  - ok or error value
  */
int32_t com_ping_process_lwip_mcu(int32_t ping,
                                  const com_sockaddr_t *addr, int32_t addrlen,
                                  uint8_t timeout, com_ping_rsp_t *rsp);

/**
  * @brief  Ping close
  * @note   Close a ping session and release ping handle
  * @param  ping      - ping handle obtained with com_ping
  * @retval int32_t   - ok or error value
  */
int32_t com_closeping_lwip_mcu(int32_t ping);

/**
  * @}
  */

/**
  * @}
  */

#endif /* (USE_COM_PING == 1) */


/*** Component Initialization/Start *******************************************/
/*** Used by com_sockets module - Not an User Interface ***********************/

/**
  * @brief  Component initialization
  * @note   must be called only one time and
  *         before using any other functions of com_*
  *         Restrictions, if any, are linked to LwIP module used
  * @param  -
  * @retval bool      - true/false init ok/nok
  */
bool com_init_lwip_mcu(void);

/**
  * @brief  Component start
  * @note   must be called only one time but
  *         after com_init and dc_start
  *         and before using any other functions of com_*
  *         Restrictions, if any, are linked to LwIP module used
  * @param  -
  * @retval -
  */
void com_start_lwip_mcu(void);

#ifdef __cplusplus
}
#endif

#endif /* COM_SOCKETS_LWIP_MCU_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
