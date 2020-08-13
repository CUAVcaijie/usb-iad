/*
 * usbd_mc.c
 *
 *  Created on: 2020年8月12日
 *      Author: caijie
 */

#include "usbd_mc.h"
#include "usbd_def.h"
#include "usbd_msc.h"
#include "usbd_cdc.h"
#include "usbd_storage_if.h"
#include "usbd_cdc_if.h"
#include "usbd_cdc.h"
#include "usbd_ctlreq.h"
#include "debug.h"

/** @defgroup MC_CORE_Private_FunctionPrototypes
  * @{
  */
uint8_t  USBD_MC_Init (USBD_HandleTypeDef *pdev,
                            uint8_t cfgidx);

uint8_t  USBD_MC_DeInit (USBD_HandleTypeDef *pdev,
                              uint8_t cfgidx);

uint8_t  USBD_MC_Setup (USBD_HandleTypeDef *pdev,
                             USBD_SetupReqTypedef *req);

uint8_t  USBD_MC_DataIn (USBD_HandleTypeDef *pdev,
                              uint8_t epnum);


uint8_t  USBD_MC_DataOut (USBD_HandleTypeDef *pdev,
                               uint8_t epnum);

uint8_t  *USBD_MC_GetHSCfgDesc (uint16_t *length);

uint8_t  *USBD_MC_GetFSCfgDesc (uint16_t *length);

uint8_t  *USBD_MC_GetOtherSpeedCfgDesc (uint16_t *length);

uint8_t  *USBD_MC_GetDeviceQualifierDescriptor (uint16_t *length);
static uint8_t  USBD_MC_RxReady (USBD_HandleTypeDef *pdev);

/** @defgroup MC_CORE_Private_FunctionPrototypes
  * @{
  */
uint8_t  USBD_MC_Init (USBD_HandleTypeDef *pdev,
                            uint8_t cfgidx);

uint8_t  USBD_MC_DeInit (USBD_HandleTypeDef *pdev,
                              uint8_t cfgidx);

uint8_t  USBD_MC_Setup (USBD_HandleTypeDef *pdev,
                             USBD_SetupReqTypedef *req);

uint8_t  USBD_MC_DataIn (USBD_HandleTypeDef *pdev,
                              uint8_t epnum);


uint8_t  USBD_MC_DataOut (USBD_HandleTypeDef *pdev,
                               uint8_t epnum);

uint8_t  *USBD_MC_GetHSCfgDesc (uint16_t *length);

uint8_t  *USBD_MC_GetFSCfgDesc (uint16_t *length);

uint8_t  *USBD_MC_GetOtherSpeedCfgDesc (uint16_t *length);

uint8_t  *USBD_MC_GetDeviceQualifierDescriptor (uint16_t *length);
static uint8_t  USBD_MC_RxReady (USBD_HandleTypeDef *pdev);
/**
  * @}
  */
extern USBD_HandleTypeDef hUsbDeviceFS;
extern uint8_t usb_connect_hardware;

/** @defgroup MC_CORE_Private_Variables
  * @{
  */


USBD_ClassTypeDef  USBD_MC =
{
  USBD_MC_Init,
  USBD_MC_DeInit,
  USBD_MC_Setup,
  NULL, /*EP0_TxSent*/
  USBD_MC_RxReady, /*EP0_RxReady*/
  USBD_MC_DataIn,
  USBD_MC_DataOut,
  NULL, /*SOF */
  NULL,
  NULL,
  USBD_MC_GetHSCfgDesc,
  USBD_MC_GetFSCfgDesc,
  USBD_MC_GetOtherSpeedCfgDesc,
  USBD_MC_GetDeviceQualifierDescriptor,
};

/* USB Mass storage device Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
uint8_t USBD_MC_CfgDesc[USB_MC_CONFIG_DESC_SIZ] =
{
  /*Configuration Descriptor*/
  0x09,   /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,      /* bDescriptorType: Configuration */
  USB_MC_CONFIG_DESC_SIZ,                /* wTotalLength:no of returned bytes */
  0x00,
  0x03,   /* bNumInterfaces: 3 interface */
  0x01,   /* bConfigurationValue: Configuration value */
  0x04,   /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,   /* bmAttributes: self powered */
  0x96,   /* MaxPower 0 mA */

  /*---------------------------------------------------------------------------*/
  // IAD
  0x08,        //��������С
  0x0B,        //IAD����������
  0x00,        // bFirstInterface
  0x02,        // bInterfaceCount
  0x02,        // bFunctionClass: CDC Class
  0x02,        // bFunctionSubClass
  0x01,        // bFunctionProtocol
  0x04,        // iFunction

  /*---------------------------------------------------------------------------*/
  /*Interface Descriptor */
  0x09,   /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x01,   /* bNumEndpoints: One endpoints used */
  0x02,   /* bInterfaceClass: Communication Interface Class */
  0x02,   /* bInterfaceSubClass: Abstract Control Model */
  0x01,   /* bInterfaceProtocol: Common AT commands */
  0x01,   /* iInterface: */

  /*Header Functional Descriptor*/
  0x05,   /* bLength: Endpoint Descriptor size */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x00,   /* bDescriptorSubtype: Header Func Desc */
  0x10,   /* bcdCDC: spec release number */
  0x01,

  /*Call Management Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x01,   /* bDescriptorSubtype: Call Management Func Desc */
  0x00,   /* bmCapabilities: D0+D1 */
  0x01,   /* bDataInterface: 1 */

  /*ACM Functional Descriptor*/
  0x04,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,   /* bmCapabilities */

  /*Union Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x06,   /* bDescriptorSubtype: Union func desc */
  0x00,   /* bMasterInterface: Communication class interface */
  0x01,   /* bSlaveInterface0: Data Class Interface */

  /*Endpoint 2 Descriptor*/
  0x07,                           /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,   /* bDescriptorType: Endpoint */
  MC_CDC_CMD_EP,                     /* bEndpointAddress */
  0x03,                           /* bmAttributes: Interrupt */
  LOBYTE(CDC_CMD_PACKET_SIZE),     /* wMaxPacketSize: */
  HIBYTE(CDC_CMD_PACKET_SIZE),
  0x01,                           /* bInterval: */

  /*Data class interface descriptor*/
  0x09,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: */
  0x01,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x02,   /* bNumEndpoints: Two endpoints used */
  0x0A,   /* bInterfaceClass: CDC */
  0x02,   /* bInterfaceSubClass: */
  0x00,   /* bInterfaceProtocol: */
  0x01,   /* iInterface: */

  /*Endpoint OUT Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
  MC_CDC_OUT_EP,                        /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(MC_MAX_FS_PACKET),  /* wMaxPacketSize: */
  HIBYTE(MC_MAX_FS_PACKET),
  0x01,    /* bInterval: ignore for Bulk transfer */

  /*Endpoint IN Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
  MC_CDC_IN_EP,                         /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(MC_MAX_FS_PACKET),  /* wMaxPacketSize: */
  HIBYTE(MC_MAX_FS_PACKET),
  0x01,                               /* bInterval: ignore for Bulk transfer */

  /*---------------------------------------------------------------------------*/
  // IAD
  0x08,        //��������С
  0x0B,        //IAD����������
  0x02,        // bFirstInterface
  0x01,        // bInterfaceCount
  0x08,        // bFunctionClass: MASS STORAGE Class
  0x06,        // bFunctionSubClass
  0x50,        // bFunctionProtocol
  0x05,        // iFunction

  /********************  Mass Storage interface ********************/
  0x09,   /* bLength: Interface Descriptor size */
  0x04,   /* bDescriptorType: */
  0x02,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x02,   /* bNumEndpoints*/
  0x08,   /* bInterfaceClass: MSC Class */
  0x06,   /* bInterfaceSubClass : SCSI transparent*/
  0x50,   /* nInterfaceProtocol */
  0x05,          /* iInterface: */
  /********************  Mass Storage Endpoints ********************/
  0x07,   /*Endpoint descriptor length = 7*/
  0x05,   /*Endpoint descriptor type */
  MC_MSC_EPIN_ADDR,   /*Endpoint address (IN, address 1) */
  0x02,   /*Bulk endpoint type */
  LOBYTE(MC_MAX_FS_PACKET),
  HIBYTE(MC_MAX_FS_PACKET),
  0x01,   /*Polling interval in milliseconds */

  0x07,   /*Endpoint descriptor length = 7 */
  0x05,   /*Endpoint descriptor type */
  MC_MSC_EPOUT_ADDR,   /*Endpoint address (OUT, address 1) */
  0x02,   /*Bulk endpoint type */
  LOBYTE(MC_MAX_FS_PACKET),
  HIBYTE(MC_MAX_FS_PACKET),
  0x01     /*Polling interval in milliseconds*/
};

uint8_t USBD_MC_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0xEF,
  0x02,
  0x01,
  MSC_MAX_FS_PACKET,
  0x01,
  0x00,
};

static USBD_CDC_HandleTypeDef *pCDCData;
static USBD_MSC_BOT_HandleTypeDef *pMSCData;

static uint32_t cdc_hot_swap_time = 0;
/**
  * @}
  */

/** @defgroup MC_CORE_Private_Functions
  * @{
  */

/**
  * @brief  USBD_MC_Init
  *         Initialize  the mass storage configuration
  * @param  pdev: device instance
  * @param  cfgidx: configuration index
  * @retval status
  */
uint8_t  USBD_MC_Init (USBD_HandleTypeDef *pdev,
                            uint8_t cfgidx)
{
    eprintf("USBD_MC_Init \r\n");
    uint8_t res = 0;

    pdev->pUserData =  &USBD_Interface_fops_HS;
    res += USBD_CDC.Init(pdev, cfgidx);
    pCDCData = pdev->pClassData;

    pdev->pUserData = &USBD_Storage_Interface_fops_HS;
    res += USBD_MSC.Init(pdev, cfgidx);
    pMSCData = pdev->pClassData;

  return res;
}

/**
  * @brief  USBD_MC_DeInit
  *         DeInitilaize  the mass storage configuration
  * @param  pdev: device instance
  * @param  cfgidx: configuration index
  * @retval status
  */
uint8_t  USBD_MC_DeInit (USBD_HandleTypeDef *pdev,
                              uint8_t cfgidx)
{
    eprintf("USBD_MC_DeInit \r\n");
    uint8_t res = USBD_OK;

    pdev->pClassData = pCDCData;
    pdev->pUserData =  &USBD_Interface_fops_HS;
    res += USBD_CDC.DeInit(pdev, cfgidx);

    pdev->pClassData = pMSCData;
    pdev->pUserData = &USBD_Storage_Interface_fops_HS;
    res += USBD_MSC.DeInit(pdev, cfgidx);

    return USBD_OK;
}

/**
  * @}
  */

static uint8_t  USBD_MC_RxReady (USBD_HandleTypeDef *pdev)
{
    USBD_CDC.EP0_RxReady(pdev);
    return USBD_OK;
}

/**
* @brief  USBD_MC_Setup
*         Handle the MC specific requests
* @param  pdev: device instance
* @param  req: USB request
* @retval status
*/
uint8_t  USBD_MC_Setup (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  eprintf("USBD_MC_Setup wIndex: %x wLength: %x bmRequest: %x \r\n", req->wIndex, req->wLength, req->bmRequest);

  switch (req->bmRequest & USB_REQ_RECIPIENT_MASK)
  {
      case USB_REQ_RECIPIENT_INTERFACE:
          switch(req->wIndex)
           {
              case 0x01:
              case 0x00:
                  pdev->pClassData = pCDCData;
                  pdev->pUserData =  &USBD_Interface_fops_HS;
                return(USBD_CDC.Setup(pdev, req));

              case 0x02:
                  pdev->pClassData = pMSCData;
                  pdev->pUserData = &USBD_Storage_Interface_fops_HS;
                return(USBD_MSC.Setup (pdev, req));

              default:
                 break;
          }
      break;

      case USB_REQ_RECIPIENT_ENDPOINT:
          switch(req->wIndex)
          {
              case CDC_IN_EP:
              case CDC_OUT_EP:
              case CDC_CMD_EP:
                  pdev->pClassData = pCDCData;
                  pdev->pUserData =  &USBD_Interface_fops_HS;
              return(USBD_CDC.Setup(pdev, req));

              case MSC_EPIN_ADDR:
              case MSC_EPOUT_ADDR:
                  pdev->pClassData = pMSCData;
                  pdev->pUserData = &USBD_Storage_Interface_fops_HS;
              return(USBD_MSC.Setup (pdev, req));

              default:
                 break;
          }
      break;

  }
  return USBD_OK;
}

/**
* @brief  USBD_MC_DataIn
*         handle data IN Stage
* @param  pdev: device instance
* @param  epnum: endpoint index
* @retval status
*/
uint8_t  USBD_MC_DataIn (USBD_HandleTypeDef *pdev,
                              uint8_t epnum)
{
    switch(epnum)
    {
        case (CDC_IN_EP & 0x0F):
            pdev->pClassData = pCDCData;
            pdev->pUserData =  &USBD_Interface_fops_HS;
            return(USBD_CDC.DataIn(pdev,epnum));

        case (MSC_EPIN_ADDR & 0x0F):
            pdev->pClassData = pMSCData;
            pdev->pUserData = &USBD_Storage_Interface_fops_HS;
            return(USBD_MSC.DataIn(pdev, epnum));

        default:
            break;
    }
    return USBD_FAIL;
}

/**
* @brief  USBD_MC_DataOut
*         handle data OUT Stage
* @param  pdev: device instance
* @param  epnum: endpoint index
* @retval status
*/
uint8_t  USBD_MC_DataOut (USBD_HandleTypeDef *pdev,
                               uint8_t epnum)
{
    switch(epnum)
    {
        case (CDC_OUT_EP & 0x0F):
        case (CDC_CMD_EP & 0x0F):
            pdev->pClassData = pCDCData;
            pdev->pUserData =  &USBD_Interface_fops_HS;
            return USBD_CDC.DataOut(pdev, epnum);

        case (MSC_EPOUT_ADDR & 0x0F):
            pdev->pClassData = pMSCData;
            pdev->pUserData = &USBD_Storage_Interface_fops_HS;
            return USBD_MSC.DataOut(pdev, epnum);

        default:
            break;
    }

    return USBD_FAIL;
}

/**
* @brief  USBD_MC_GetHSCfgDesc
*         return configuration descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
uint8_t  *USBD_MC_GetHSCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_MC_CfgDesc);
  return USBD_MC_CfgDesc;
}

/**
* @brief  USBD_MC_GetFSCfgDesc
*         return configuration descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
uint8_t  *USBD_MC_GetFSCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_MC_CfgDesc);
  return USBD_MC_CfgDesc;
}

/**
* @brief  USBD_MC_GetOtherSpeedCfgDesc
*         return other speed configuration descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
uint8_t  *USBD_MC_GetOtherSpeedCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_MC_CfgDesc);
  return USBD_MC_CfgDesc;
}
/**
* @brief  DeviceQualifierDescriptor
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
uint8_t  *USBD_MC_GetDeviceQualifierDescriptor (uint16_t *length)
{
  *length = sizeof (USBD_MC_DeviceQualifierDesc);
  return USBD_MC_DeviceQualifierDesc;
}

/**
* @brief  USBD_MC_RegisterStorage
* @param  fops: storage callback
* @retval status
*/
void MC_Switch_MSC(USBD_HandleTypeDef *pdev)
{
    pdev->pClassData = pMSCData;
    pdev->pUserData = &USBD_Storage_Interface_fops_HS;

//  static USBD_MSC_BOT_HandleTypeDef msc_handle;
//
//  USBD_MSC_RegisterStorage(pdev, &USBD_Storage_Interface_fops_HS);
//  pdev->pClassData = &msc_handle;
}

void MC_Switch_CDC(USBD_HandleTypeDef *pdev)
{
    pdev->pClassData = pCDCData;
    pdev->pUserData =  &USBD_Interface_fops_HS;

//  static USBD_CDC_HandleTypeDef cdc_handle;
//
//  USBD_CDC_RegisterInterface(pdev, &USBD_Interface_fops_HS);
//  pdev->pClassData = &cdc_handle;
}
