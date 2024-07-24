/**
  ******************************************************************************
  * @file    usbd_cdc.c
  * @author  MCD Application Team
  * @brief   This file provides the high layer firmware functions to manage the
  *          following functionalities of the USB MIDI Class:
  *           - Initialization and Configuration of high and low layer
  *           - Enumeration as MIDI Device (and enumeration for each implemented memory interface)
  *           - OUT/IN data transfer
  *           - Error management
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  *  @verbatim
  *
  *          ===================================================================
  *                                USB MIDI Class Driver Description
  *          ===================================================================
  *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
  *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus
  *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as MIDI device with 2 data endpoints (IN and OUT)
  *             - Requests management (as described in section 6.2 in specification)
  *             - Abstract Control Model compliant
  *             - Data interface class
  *
  *           These aspects may be enriched or modified for a specific user application.
  *
  *            This driver doesn't implement the following aspects of the specification
  *            (but it is possible to manage these features with some modifications on this driver):
  *             - Any class-specific aspect relative to communication classes should be managed by user application.
  *             - All communication classes other than PSTN are not managed
  *
  *  @endverbatim
  *
  ******************************************************************************
  */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_midi.h"
#include "usbd_ctlreq.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_MIDI
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_MIDI_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_MIDI_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_MIDI_Private_Macros
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_MIDI_Private_FunctionPrototypes
  * @{
  */

static uint8_t USBD_MIDI_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_MIDI_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_MIDI_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_MIDI_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_MIDI_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_MIDI_EP0_RxReady(USBD_HandleTypeDef *pdev);
#ifndef USE_USBD_COMPOSITE
static uint8_t *USBD_MIDI_GetFSCfgDesc(uint16_t *length);
static uint8_t *USBD_MIDI_GetHSCfgDesc(uint16_t *length);
static uint8_t *USBD_MIDI_GetOtherSpeedCfgDesc(uint16_t *length);
uint8_t *USBD_MIDI_GetDeviceQualifierDescriptor(uint16_t *length);
#endif /* USE_USBD_COMPOSITE  */

#ifndef USE_USBD_COMPOSITE
/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_MIDI_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};
#endif /* USE_USBD_COMPOSITE  */
/**
  * @}
  */

/** @defgroup USBD_MIDI_Private_Variables
  * @{
  */


/* USBMIDI interface class callbacks structure */
USBD_ClassTypeDef  USBD_MIDI =
{
  USBD_MIDI_Init,
  USBD_MIDI_DeInit,
  USBD_MIDI_Setup,
  NULL,                 /* EP0_TxSent */
  USBD_MIDI_EP0_RxReady,
  USBD_MIDI_DataIn,
  USBD_MIDI_DataOut,
  NULL,
  NULL,
  NULL,
#ifdef USE_USBD_COMPOSITE
  NULL,
  NULL,
  NULL,
  NULL,
#else
  USBD_MIDI_GetHSCfgDesc,
  USBD_MIDI_GetFSCfgDesc,
  USBD_MIDI_GetOtherSpeedCfgDesc,
  USBD_MIDI_GetDeviceQualifierDescriptor,
#endif /* USE_USBD_COMPOSITE  */
};

#ifndef USE_USBD_COMPOSITE
/* USB USBMIDI device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_MIDI_CfgDesc[USB_MIDI_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /* Configuration Descriptor */
  0x09,                                       /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,                /* bDescriptorType: Configuration */
  USB_MIDI_CONFIG_DESC_SIZ,                    /* wTotalLength */
  0x00,
  0x02,                                       /* bNumInterfaces: 2 interfaces */
  0x01,                                       /* bConfigurationValue: Configuration value */
  0x00,                                       /* iConfiguration: Index of string descriptor
                                                 describing the configuration */
#if (USBD_SELF_POWERED == 1U)
  0xC0,                                       /* bmAttributes: Bus Powered according to user configuration */
#else
  0x80,                                       /* bmAttributes: Bus Powered according to user configuration */
#endif /* USBD_SELF_POWERED */
  USBD_MAX_POWER,                             /* MaxPower (mA) */

  /*---------------------------------------------------------------------------*/

  /* Interface Descriptor */
  0x09,                                       /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,                    /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00,                                       /* bInterfaceNumber: Number of Interface */
  0x00,                                       /* bAlternateSetting: Alternate setting */
  0x00,                                       /* bNumEndpoints: One endpoint used */
  0x01,                                       /* bInterfaceClass: AUDIO Interface Class */
  0x01,                                       /* bInterfaceSubClass: AUDIO Control */
  0x00,                                       /* bInterfaceProtocol */
  0x00,                                       /* iInterface */
  /* Class-specific AC Interface Descriptor */
  0x09,                                       /* bLength: Interface Descriptor size */
  0x24,                                       /* CS_INTERFACE */
  0x01,                                       /* HEADER subtype. */
  0x00,                                       /* Revision of class specification - 1.0 */
  0x01,
  0x00,                                       /* bLength: Interface Descriptor size */
  0x09,                                       /* Total size of class specific descriptors */
  0x01,                                       /* Number of streaming interfaces */
  0x01,                                       /* MIDIStreaming interface 1 belongs to this
AudioControl interface */
  /* MIDIStreaming Interface Descriptors */
  /* Standard MS Interface Descriptor */
  0x09,                                       /* bLength: Interface Descriptor size */
  0x04,                                       /* INTERFACE descriptor */
  0x01,                                       /* Index of this interface */
  0x00,                                       /* Index of this alternate setting */
  0x02,                                       /* 2 endpoints */
  0x01,                                       /* AUDIO */
  0x03,                                       /* MIDISTREAMING */
  0x00,                                       /* bInterfaceProtocol */
  0x00,                                       /* iInterface */

  /* Class-specific MS Interface Descriptor */
  0x07,                                       /* bLength: Endpoint Descriptor size */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x01,                                       /* bDescriptorSubtype: MS_HEADER */
  0x00,                                       /* Revision of this class specification */
  0x01,
  0x41,                                       /* Total size of class-specific descriptors */
  0x00,

  /* MIDI IN Jack Descriptor */
  /* : MIDI Adapter MIDI IN Jack Descriptor (Embedded) */
  0x06,                                       /* bLength: Endpoint Descriptor size */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x02,                                       /* bDescriptorSubtype: MIDI_IN_JACK */
  0x01,                                       /* EMBEDDED */
  0x01,                                       /* ID of this Jack */
  0x00,                                       /* iJack: unused */
  /* MIDI Adapter MIDI IN Jack Descriptor (External) */
  0x06,                                       /* bLength: Endpoint Descriptor size */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x02,                                       /* bDescriptorSubtype: MIDI_IN_JACK */
  0x02,                                       /* EXTERNAL */
  0x02,                                       /* ID of this Jack */
  0x00,                                       /* iJack: unused */

  /* MIDI Adapter MIDI OUT Jack Descriptor (Embedded) */
  0x09,                                       /* bLength: Interface Descriptor size */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x03,                                       /* MIDI_OUT_JACK */
  0x01,                                       /* EMBEDDED */
  0x03,                                       /* ID of this Jack */
  0x01,                                       /* Number of Input Pins of this Jack */
  0x02,                                       /* ID of the Entity to which this Pin is
connected */
  0x01,                                       /* Output Pin number of the Entity to which
this Input Pin is connected */
  0x00,                                       /* iJack: unused */
  /* MIDI Adapter MIDI OUT Jack Descriptor (External) */
  0x09,                                       /* bLength: Interface Descriptor size */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x03,                                       /* MIDI_OUT_JACK */
  0x02,                                       /* EXTERNAL */
  0x04,                                       /* ID of this Jack */
  0x01,                                       /* Number of Input Pins of this Jack */
  0x01,                                       /* ID of the Entity to which this Pin is
connected */
  0x01,                                       /* Output Pin number of the Entity to which
this Input Pin is connected */
  0x00,                                       /* iJack: unused */

  /* Bulk OUT Endpoint Descriptors */
  /* Standard Bulk OUT Endpoint Descriptor */
  /* MIDI Adapter Standard Bulk OUT Endpoint Descriptor */
  0x09,                                       /* bLength: Interface Descriptor size */
  0x05,                                       /* bDescriptorType: ENDPOINT */
  MIDI_OUT_EP,                                       /* OUT Endpoint 1 */
  0x02,                                       /* Bulk, not shared. */
  MIDI_DATA_FS_OUT_PACKET_SIZE,                                       /* 64 bytes per packet */
  MIDI_DATA_FS_OUT_PACKET_SIZE>>8,
  0x00,                                       /* Ignored for Bulk. Set to zero */
  0x00,                                       /* bRefresh: unused */
  0x00,                                       /* bSynchAddress: unused */
  /* Class-specific MS Bulk OUT Endpoint Descriptor */
  0x05,                                       /* bLength: Endpoint Descriptor size */
  0x25,                                       /* bDescriptorType: CS_ENDPOINT */
  0x01,                                       /* bDescriptorSubtype: MS_GENERAL */
  0x01,                                       /* Number of embedded MIDI IN Jacks */
  0x01,                                       /* ID of the Embedded MIDI IN Jack */

  /* Standard Bulk IN Endpoint Descriptor */
  /* Standard Bulk IN Endpoint Descriptor */
  /* MIDI Adapter Standard Bulk OUT Endpoint Descriptor */
  0x09,                                       /* bLength: Interface Descriptor size */
  0x05,                                       /* bDescriptorType: ENDPOINT */
  MIDI_IN_EP,                                       /* IN Endpoint 1 */
  0x02,                                       /* Bulk, not shared. */
  MIDI_DATA_FS_IN_PACKET_SIZE,                                       /* 64 bytes per packet */
  MIDI_DATA_FS_IN_PACKET_SIZE>>8,
  0x00,                                       /* Ignored for Bulk. Set to zero */
  0x00,                                       /* bRefresh: unused */
  0x00,                                       /* bSynchAddress: unused */
  /* Class-specific MS Bulk IN Endpoint Descriptor */
  0x05,                                       /* bLength: Endpoint Descriptor size */
  0x25,                                       /* bDescriptorType: CS_ENDPOINT */
  0x01,                                       /* bDescriptorSubtype: MS_GENERAL */
  0x01,                                       /* Number of embedded MIDI IN Jacks */
  0x03,                                       /* ID of the Embedded MIDI IN Jack */
};
#endif /* USE_USBD_COMPOSITE  */

static uint8_t USBMIDIInEpAdd = MIDI_IN_EP;
static uint8_t USBMIDIOutEpAdd = MIDI_OUT_EP;

/**
  * @}
  */

/** @defgroup USBD_MIDI_Private_Functions
  * @{
  */

/**
  * @brief  USBD_MIDI_Init
  *         Initialize the USBMIDI interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_MIDI_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);
  USBD_MIDI_HandleTypeDef *husbmidi;

  husbmidi = (USBD_MIDI_HandleTypeDef *)USBD_malloc(sizeof(USBD_MIDI_HandleTypeDef));

  if (husbmidi == NULL)
  {
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    return (uint8_t)USBD_EMEM;
  }

  (void)USBD_memset(husbmidi, 0, sizeof(USBD_MIDI_HandleTypeDef));

  pdev->pClassDataCmsit[pdev->classId] = (void *)husbmidi;
  pdev->pClassData = pdev->pClassDataCmsit[pdev->classId];

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  USBMIDIInEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
  USBMIDIOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    /* Open EP IN */
    (void)USBD_LL_OpenEP(pdev, USBMIDIInEpAdd, USBD_EP_TYPE_BULK,
                         MIDI_DATA_HS_IN_PACKET_SIZE);

    pdev->ep_in[USBMIDIInEpAdd & 0xFU].is_used = 1U;

    /* Open EP OUT */
    (void)USBD_LL_OpenEP(pdev, USBMIDIOutEpAdd, USBD_EP_TYPE_BULK,
                         MIDI_DATA_HS_OUT_PACKET_SIZE);

    pdev->ep_out[USBMIDIOutEpAdd & 0xFU].is_used = 1U;
  }
  else
  {
    /* Open EP IN */
    (void)USBD_LL_OpenEP(pdev, USBMIDIInEpAdd, USBD_EP_TYPE_BULK,
                         MIDI_DATA_FS_IN_PACKET_SIZE);

    pdev->ep_in[USBMIDIInEpAdd & 0xFU].is_used = 1U;

    /* Open EP OUT */
    (void)USBD_LL_OpenEP(pdev, USBMIDIOutEpAdd, USBD_EP_TYPE_BULK,
                         MIDI_DATA_FS_OUT_PACKET_SIZE);

    pdev->ep_out[USBMIDIOutEpAdd & 0xFU].is_used = 1U;
  }

  husbmidi->RxBuffer = NULL;

  /* Init  physical Interface components */
  ((USBD_MIDI_ItfTypeDef *)pdev->pUserData[pdev->classId])->Init();

  /* Init Xfer states */
  husbmidi->TxState = 0U;
  husbmidi->RxState = 0U;

  if (husbmidi->RxBuffer == NULL)
  {
    return (uint8_t)USBD_EMEM;
  }

  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    /* Prepare Out endpoint to receive next packet */
    (void)USBD_LL_PrepareReceive(pdev, USBMIDIOutEpAdd, husbmidi->RxBuffer,
                                 MIDI_DATA_HS_OUT_PACKET_SIZE);
  }
  else
  {
    /* Prepare Out endpoint to receive next packet */
    (void)USBD_LL_PrepareReceive(pdev, USBMIDIOutEpAdd, husbmidi->RxBuffer,
                                 MIDI_DATA_FS_OUT_PACKET_SIZE);
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_MIDI_Init
  *         DeInitialize the USBMIDI layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_MIDI_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);


#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this USBMIDI class instance */
  USBMIDIInEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
  USBMIDIOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  /* Close EP IN */
  (void)USBD_LL_CloseEP(pdev, USBMIDIInEpAdd);
  pdev->ep_in[USBMIDIInEpAdd & 0xFU].is_used = 0U;

  /* Close EP OUT */
  (void)USBD_LL_CloseEP(pdev, USBMIDIOutEpAdd);
  pdev->ep_out[USBMIDIOutEpAdd & 0xFU].is_used = 0U;

  /* DeInit  physical Interface components */
  if (pdev->pClassDataCmsit[pdev->classId] != NULL)
  {
    ((USBD_MIDI_ItfTypeDef *)pdev->pUserData[pdev->classId])->DeInit();
    (void)USBD_free(pdev->pClassDataCmsit[pdev->classId]);
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    pdev->pClassData = NULL;
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_MIDI_Setup
  *         Handle the USBMIDI specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_MIDI_Setup(USBD_HandleTypeDef *pdev,
                              USBD_SetupReqTypedef *req)
{
  USBD_MIDI_HandleTypeDef *husbmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint16_t len;
  uint8_t ifalt = 0U;
  uint16_t status_info = 0U;
  USBD_StatusTypeDef ret = USBD_OK;

  if (husbmidi == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS:
      if (req->wLength != 0U)
      {
        if ((req->bmRequest & 0x80U) != 0U)
        {
          ((USBD_MIDI_ItfTypeDef *)pdev->pUserData[pdev->classId])->Control(req->bRequest,
                                                                           (uint8_t *)husbmidi->data,
                                                                           req->wLength);

          len = MIN(MIDI_REQ_MAX_DATA_SIZE, req->wLength);
          (void)USBD_CtlSendData(pdev, (uint8_t *)husbmidi->data, len);
        }
        else
        {
          husbmidi->CmdOpCode = req->bRequest;
          husbmidi->CmdLength = (uint8_t)MIN(req->wLength, USB_MAX_EP0_SIZE);

          (void)USBD_CtlPrepareRx(pdev, (uint8_t *)husbmidi->data, husbmidi->CmdLength);
        }
      }
      else
      {
        ((USBD_MIDI_ItfTypeDef *)pdev->pUserData[pdev->classId])->Control(req->bRequest,
                                                                         (uint8_t *)req, 0U);
      }
      break;

    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_GET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, &ifalt, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_SET_INTERFACE:
          if (pdev->dev_state != USBD_STATE_CONFIGURED)
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_CLEAR_FEATURE:
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }

  return (uint8_t)ret;
}

/**
  * @brief  USBD_MIDI_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t USBD_MIDI_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_MIDI_HandleTypeDef *husbmidi;
  PCD_HandleTypeDef *hpcd = (PCD_HandleTypeDef *)pdev->pData;

  if (pdev->pClassDataCmsit[pdev->classId] == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  husbmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  if ((pdev->ep_in[epnum & 0xFU].total_length > 0U) &&
      ((pdev->ep_in[epnum & 0xFU].total_length % hpcd->IN_ep[epnum & 0xFU].maxpacket) == 0U))
  {
    /* Update the packet total length */
    pdev->ep_in[epnum & 0xFU].total_length = 0U;

    /* Send ZLP */
    (void)USBD_LL_Transmit(pdev, epnum, NULL, 0U);
  }
  else
  {
    husbmidi->TxState = 0U;

    if (((USBD_MIDI_ItfTypeDef *)pdev->pUserData[pdev->classId])->TransmitCplt != NULL)
    {
      ((USBD_MIDI_ItfTypeDef *)pdev->pUserData[pdev->classId])->TransmitCplt(husbmidi->TxBuffer, &husbmidi->TxLength, epnum);
    }
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_MIDI_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t USBD_MIDI_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_MIDI_HandleTypeDef *husbmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  if (pdev->pClassDataCmsit[pdev->classId] == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  /* Get the received data length */
  husbmidi->RxLength = USBD_LL_GetRxDataSize(pdev, epnum);

  /* USB data will be immediately processed, this allow next USB traffic being
  NAKed till the end of the application Xfer */

  ((USBD_MIDI_ItfTypeDef *)pdev->pUserData[pdev->classId])->Receive(husbmidi->RxBuffer, &husbmidi->RxLength);

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_MIDI_EP0_RxReady
  *         Handle EP0 Rx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_MIDI_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
  USBD_MIDI_HandleTypeDef *husbmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  if (husbmidi == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  if ((pdev->pUserData[pdev->classId] != NULL) && (husbmidi->CmdOpCode != 0xFFU))
  {
    ((USBD_MIDI_ItfTypeDef *)pdev->pUserData[pdev->classId])->Control(husbmidi->CmdOpCode,
                                                                     (uint8_t *)husbmidi->data,
                                                                     (uint16_t)husbmidi->CmdLength);
    husbmidi->CmdOpCode = 0xFFU;
  }

  return (uint8_t)USBD_OK;
}
#ifndef USE_USBD_COMPOSITE
/**
  * @brief  USBD_MIDI_GetFSCfgDesc
  *         Return configuration descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_MIDI_GetFSCfgDesc(uint16_t *length)
{
  USBD_EpDescTypeDef *pEpOutDesc = USBD_GetEpDesc(USBD_MIDI_CfgDesc, MIDI_OUT_EP);
  USBD_EpDescTypeDef *pEpInDesc = USBD_GetEpDesc(USBD_MIDI_CfgDesc, MIDI_IN_EP);

  if (pEpOutDesc != NULL)
  {
    pEpOutDesc->wMaxPacketSize = MIDI_DATA_FS_MAX_PACKET_SIZE;
  }

  if (pEpInDesc != NULL)
  {
    pEpInDesc->wMaxPacketSize = MIDI_DATA_FS_MAX_PACKET_SIZE;
  }

  *length = (uint16_t)sizeof(USBD_MIDI_CfgDesc);
  return USBD_MIDI_CfgDesc;
}

/**
  * @brief  USBD_MIDI_GetHSCfgDesc
  *         Return configuration descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_MIDI_GetHSCfgDesc(uint16_t *length)
{
  USBD_EpDescTypeDef *pEpOutDesc = USBD_GetEpDesc(USBD_MIDI_CfgDesc, MIDI_OUT_EP);
  USBD_EpDescTypeDef *pEpInDesc = USBD_GetEpDesc(USBD_MIDI_CfgDesc, MIDI_IN_EP);

  if (pEpOutDesc != NULL)
  {
    pEpOutDesc->wMaxPacketSize = MIDI_DATA_HS_MAX_PACKET_SIZE;
  }

  if (pEpInDesc != NULL)
  {
    pEpInDesc->wMaxPacketSize = MIDI_DATA_HS_MAX_PACKET_SIZE;
  }

  *length = (uint16_t)sizeof(USBD_MIDI_CfgDesc);
  return USBD_MIDI_CfgDesc;
}

/**
  * @brief  USBD_MIDI_GetOtherSpeedCfgDesc
  *         Return configuration descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_MIDI_GetOtherSpeedCfgDesc(uint16_t *length)
{
  USBD_EpDescTypeDef *pEpOutDesc = USBD_GetEpDesc(USBD_MIDI_CfgDesc, MIDI_OUT_EP);
  USBD_EpDescTypeDef *pEpInDesc = USBD_GetEpDesc(USBD_MIDI_CfgDesc, MIDI_IN_EP);

  if (pEpOutDesc != NULL)
  {
    pEpOutDesc->wMaxPacketSize = MIDI_DATA_FS_MAX_PACKET_SIZE;
  }

  if (pEpInDesc != NULL)
  {
    pEpInDesc->wMaxPacketSize = MIDI_DATA_FS_MAX_PACKET_SIZE;
  }

  *length = (uint16_t)sizeof(USBD_MIDI_CfgDesc);
  return USBD_MIDI_CfgDesc;
}

/**
  * @brief  USBD_MIDI_GetDeviceQualifierDescriptor
  *         return Device Qualifier descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
uint8_t *USBD_MIDI_GetDeviceQualifierDescriptor(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_MIDI_DeviceQualifierDesc);

  return USBD_MIDI_DeviceQualifierDesc;
}
#endif /* USE_USBD_COMPOSITE  */
/**
  * @brief  USBD_MIDI_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CD  Interface callback
  * @retval status
  */
uint8_t USBD_MIDI_RegisterInterface(USBD_HandleTypeDef *pdev,
                                   USBD_MIDI_ItfTypeDef *fops)
{
  if (fops == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  pdev->pUserData[pdev->classId] = fops;

  return (uint8_t)USBD_OK;
}


/**
  * @brief  USBD_MIDI_SetTxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Tx Buffer
  * @param  length: length of data to be sent
  * @param  ClassId: The Class ID
  * @retval status
  */
#ifdef USE_USBD_COMPOSITE
uint8_t USBD_MIDI_SetTxBuffer(USBD_HandleTypeDef *pdev,
                             uint8_t *pbuff, uint32_t length, uint8_t ClassId)
{
  USBD_MIDI_HandleTypeDef *husbmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassDataCmsit[ClassId];
#else
uint8_t USBD_MIDI_SetTxBuffer(USBD_HandleTypeDef *pdev,
                             uint8_t *pbuff, uint32_t length)
{
  USBD_MIDI_HandleTypeDef *husbmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
#endif /* USE_USBD_COMPOSITE */

  if (husbmidi == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  husbmidi->TxBuffer = pbuff;
  husbmidi->TxLength = length;

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_MIDI_SetRxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Rx Buffer
  * @retval status
  */
uint8_t USBD_MIDI_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff)
{
  USBD_MIDI_HandleTypeDef *husbmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  if (husbmidi == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  husbmidi->RxBuffer = pbuff;

  return (uint8_t)USBD_OK;
}


/**
  * @brief  USBD_MIDI_TransmitPacket
  *         Transmit packet on IN endpoint
  * @param  pdev: device instance
  * @param  ClassId: The Class ID
  * @retval status
  */
#ifdef USE_USBD_COMPOSITE
uint8_t USBD_MIDI_TransmitPacket(USBD_HandleTypeDef *pdev, uint8_t ClassId)
{
  USBD_MIDI_HandleTypeDef *husbmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassDataCmsit[ClassId];
#else
uint8_t USBD_MIDI_TransmitPacket(USBD_HandleTypeDef *pdev)
{
  USBD_MIDI_HandleTypeDef *husbmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
#endif  /* USE_USBD_COMPOSITE */

  USBD_StatusTypeDef ret = USBD_BUSY;

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  USBMIDIInEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_BULK, ClassId);
#endif  /* USE_USBD_COMPOSITE */

  if (husbmidi == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  if (husbmidi->TxState == 0U)
  {
    /* Tx Transfer in progress */
    husbmidi->TxState = 1U;

    /* Update the packet total length */
    pdev->ep_in[USBMIDIInEpAdd & 0xFU].total_length = husbmidi->TxLength;

    /* Transmit next packet */
    (void)USBD_LL_Transmit(pdev, USBMIDIInEpAdd, husbmidi->TxBuffer, husbmidi->TxLength);

    ret = USBD_OK;
  }

  return (uint8_t)ret;
}

/**
  * @brief  USBD_MIDI_ReceivePacket
  *         prepare OUT Endpoint for reception
  * @param  pdev: device instance
  * @retval status
  */
uint8_t USBD_MIDI_ReceivePacket(USBD_HandleTypeDef *pdev)
{
  USBD_MIDI_HandleTypeDef *husbmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  USBMIDIOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  if (pdev->pClassDataCmsit[pdev->classId] == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    /* Prepare Out endpoint to receive next packet */
    (void)USBD_LL_PrepareReceive(pdev, USBMIDIOutEpAdd, husbmidi->RxBuffer,
                                 MIDI_DATA_HS_OUT_PACKET_SIZE);
  }
  else
  {
    /* Prepare Out endpoint to receive next packet */
    (void)USBD_LL_PrepareReceive(pdev, USBMIDIOutEpAdd, husbmidi->RxBuffer,
                                 MIDI_DATA_FS_OUT_PACKET_SIZE);
  }

  return (uint8_t)USBD_OK;
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

