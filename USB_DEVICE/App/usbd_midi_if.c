/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_midi_if.c
  * @version        : v1.0_Cube
  * @brief          : Usb device for Virtual Com Port.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_midi_if.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_MIDI_IF
  * @{
  */

/** @defgroup USBD_MIDI_IF_Private_TypesDefinitions USBD_MIDI_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_MIDI_IF_Private_Defines USBD_MIDI_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */
/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_MIDI_IF_Private_Macros USBD_MIDI_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */
#define APP_RX_MASK (APP_RX_DATA_SIZE-1)
#define APP_TX_MASK (APP_TX_DATA_SIZE-1)
/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_MIDI_IF_Private_Variables USBD_MIDI_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE]={0};

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE]={0};

/* USER CODE BEGIN PRIVATE_VARIABLES */
__IO uint16_t UserRxBufferFS_wp = 0,  UserRxBufferFS_rp = 0;
__IO uint16_t UserTxBufferFS_wp = 0,  UserTxBufferFS_rp = 0;
__IO uint8_t UserTx_busy = 0;
/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_MIDI_IF_Exported_Variables USBD_MIDI_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_MIDI_IF_Private_FunctionPrototypes USBD_MIDI_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t USBMIDI_Init_FS(void);
static int8_t USBMIDI_DeInit_FS(void);
static int8_t USBMIDI_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t USBMIDI_Receive_FS(uint8_t* pbuf, uint32_t *Len);
static int8_t USBMIDI_TransmitCplt_FS(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_MIDI_ItfTypeDef USBD_Interface_fops_FS =
{
  USBMIDI_Init_FS,
  USBMIDI_DeInit_FS,
  USBMIDI_Control_FS,
  USBMIDI_Receive_FS,
  USBMIDI_TransmitCplt_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USBMIDI_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  /* Set Application Buffers */
  USBD_MIDI_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
  USBD_MIDI_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USBMIDI_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USBMIDI_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USBMIDI_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
  UserRxBufferFS_wp += *Len;
  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  USBMIDI_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t USBMIDI_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
  USBD_MIDI_HandleTypeDef *hcdc = (USBD_MIDI_HandleTypeDef*)hUsbDeviceFS.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  UserTx_busy = 1;
  USBD_MIDI_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_MIDI_TransmitPacket(&hUsbDeviceFS);
  /* USER CODE END 7 */
  return result;
}

/**
  * @brief  USBMIDI_TransmitCplt_FS
  *         Data transmitted callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USBMIDI_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 13 */
  UNUSED(Buf);
  //UNUSED(Len);
  UNUSED(epnum);
  UserTxBufferFS_rp += *Len;
  UserTx_busy = 0;
  /* USER CODE END 13 */
  return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
uint16_t tx_data_len(){
  if((UserTxBufferFS_wp&APP_TX_MASK) >= (UserTxBufferFS_rp&APP_TX_MASK))
    return UserTxBufferFS_wp - UserTxBufferFS_rp;
  return APP_TX_DATA_SIZE  - (UserTxBufferFS_rp&APP_TX_MASK);
}

void USBMIDI_send(uint32_t event){
  uint16_t len;
  if(hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED){
    UserTxBufferFS_wp = UserTxBufferFS_rp = 0;
    return;
  }
  UserTxBufferFS[UserTxBufferFS_wp++&APP_TX_MASK] = event>>24;
  UserTxBufferFS[UserTxBufferFS_wp++&APP_TX_MASK] = event>>16;
  UserTxBufferFS[UserTxBufferFS_wp++&APP_TX_MASK] = event>>8;
  UserTxBufferFS[UserTxBufferFS_wp++&APP_TX_MASK] = event;
  len = tx_data_len();
  if(len > MIDI_DATA_FS_IN_PACKET_SIZE)
    len = MIDI_DATA_FS_IN_PACKET_SIZE;
  if(!UserTx_busy)
    USBMIDI_Transmit_FS(&UserTxBufferFS[UserTxBufferFS_rp&APP_TX_MASK], len);
}

uint16_t rx_data_len(){
  if((UserRxBufferFS_wp&APP_RX_MASK) >= (UserRxBufferFS_rp&APP_RX_MASK))
    return UserRxBufferFS_wp - UserRxBufferFS_rp;
  return APP_RX_DATA_SIZE  - (UserRxBufferFS_rp&APP_RX_MASK);
}

__weak int USB_MIDI_decoder(uint8_t *Buf, uint32_t Len){
  UNUSED(Buf);
  UNUSED(Len);
  return 1;
}

void USBMIDI_polling(){
  uint16_t len;
  if(!UserTx_busy && UserTxBufferFS_wp != UserTxBufferFS_rp){
    len = tx_data_len();
    if(len > MIDI_DATA_FS_IN_PACKET_SIZE)
      len = MIDI_DATA_FS_IN_PACKET_SIZE;
    USBMIDI_Transmit_FS(&UserTxBufferFS[UserTxBufferFS_rp&APP_TX_MASK], len);
  }
  if(UserRxBufferFS_wp != UserRxBufferFS_rp){
    len = rx_data_len();
    USBD_MIDI_SetRxBuffer(&hUsbDeviceFS, &UserRxBufferFS[UserRxBufferFS_wp&APP_RX_MASK]);
    USBD_MIDI_ReceivePacket(&hUsbDeviceFS);

    if(USB_MIDI_decoder(&UserRxBufferFS[UserRxBufferFS_rp&APP_RX_MASK], len)){
      UserRxBufferFS_rp += len;
    }
  }
}
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */
