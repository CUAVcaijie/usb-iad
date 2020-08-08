/*
 * usbd_mc.c
 *
 *  Created on: May 7, 2020
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
  0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,   /* bmAttributes: self powered */
  0x32,   /* MaxPower 0 mA */

  /*---------------------------------------------------------------------------*/
  // IAD
  0x08,        //��������С
  0x0B,        //IAD����������
  0x00,        // bFirstInterface
  0x02,        // bInterfaceCount
  0x02,        // bFunctionClass: CDC Class
  0x02,        // bFunctionSubClass
  0x01,        // bFunctionProtocol
  0x00,        // iFunction

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
  0x00,   /* iInterface: */

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
  0x10,                           /* bInterval: */

  /*Data class interface descriptor*/
  0x09,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: */
  0x01,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x02,   /* bNumEndpoints: Two endpoints used */
  0x0A,   /* bInterfaceClass: CDC */
  0x00,   /* bInterfaceSubClass: */
  0x00,   /* bInterfaceProtocol: */
  0x00,   /* iInterface: */

  /*Endpoint OUT Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
  MC_CDC_OUT_EP,                        /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(MC_MAX_FS_PACKET),  /* wMaxPacketSize: */
  HIBYTE(MC_MAX_FS_PACKET),
  0x00,                              /* bInterval: ignore for Bulk transfer */

  /*Endpoint IN Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
  MC_CDC_IN_EP,                         /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(MC_MAX_FS_PACKET),  /* wMaxPacketSize: */
  HIBYTE(MC_MAX_FS_PACKET),
  0x00,                               /* bInterval: ignore for Bulk transfer */

  /*---------------------------------------------------------------------------*/
  // IAD
  0x08,        //��������С
  0x0B,        //IAD����������
  0x02,        // bFirstInterface
  0x01,        // bInterfaceCount
  0x08,        // bFunctionClass: MASS STORAGE Class
  0x06,        // bFunctionSubClass
  0x50,        // bFunctionProtocol
  0x01,        // iFunction

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
  0x00,   /*Polling interval in milliseconds */

  0x07,   /*Endpoint descriptor length = 7 */
  0x05,   /*Endpoint descriptor type */
  MC_MSC_EPOUT_ADDR,   /*Endpoint address (OUT, address 1) */
  0x02,   /*Bulk endpoint type */
  LOBYTE(MC_MAX_FS_PACKET),
  HIBYTE(MC_MAX_FS_PACKET),
  0x00     /*Polling interval in milliseconds*/
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
    cdc_hot_swap_time = HAL_GetTick();
  {
    USBD_CDC_HandleTypeDef * hcdc;

    MC_Switch_CDC(pdev);

    USBD_LL_OpenEP(pdev,
                   CDC_IN_EP,
                   USBD_EP_TYPE_BULK,
                   CDC_DATA_HS_IN_PACKET_SIZE);

    pdev->ep_in[CDC_IN_EP & 0xFU].is_used = 1U;

    USBD_LL_OpenEP(pdev,
                   CDC_OUT_EP,
                   USBD_EP_TYPE_BULK,
                   CDC_DATA_HS_OUT_PACKET_SIZE);

    pdev->ep_out[CDC_OUT_EP & 0xFU].is_used = 1U;

    USBD_LL_OpenEP(pdev,
                   CDC_CMD_EP,
                   USBD_EP_TYPE_INTR,
                   CDC_CMD_PACKET_SIZE);
    pdev->ep_in[CDC_CMD_EP & 0xFU].is_used = 1U;

    hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;

    ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->Init();

    hcdc->TxState =0;
    hcdc->RxState =0;

    USBD_LL_PrepareReceive(pdev,
                           MC_CDC_OUT_EP,
                           hcdc->RxBuffer,
                           CDC_DATA_HS_OUT_PACKET_SIZE);
  }

  {
    MC_Switch_MSC(pdev);

    USBD_LL_OpenEP(pdev,
                   MSC_EPOUT_ADDR,
                   USBD_EP_TYPE_BULK,
                   MSC_MAX_HS_PACKET);
    pdev->ep_out[MSC_EPOUT_ADDR & 0xFU].is_used = 1U;

    USBD_LL_OpenEP(pdev,
                   MSC_EPIN_ADDR,
                   USBD_EP_TYPE_BULK,
                   MSC_MAX_HS_PACKET);
    pdev->ep_in[MSC_EPIN_ADDR & 0xFU].is_used = 1U;

    MSC_BOT_Init(pdev);
  }

  return USBD_OK;
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
    if (HAL_GetTick() - cdc_hot_swap_time < 1000) {
        cdc_hot_swap_time = HAL_GetTick();
        return USBD_OK;
    }
    cdc_hot_swap_time = HAL_GetTick();

  {

    MC_Switch_CDC(pdev);
    /* Close EP IN */
    USBD_LL_CloseEP(pdev, CDC_IN_EP);
    pdev->ep_in[CDC_IN_EP & 0xFU].is_used = 0U;

    /* Close EP OUT */
    USBD_LL_CloseEP(pdev, CDC_OUT_EP);
    pdev->ep_out[CDC_OUT_EP & 0xFU].is_used = 0U;

    /* Close Command IN EP */
    USBD_LL_CloseEP(pdev, CDC_CMD_EP);
    pdev->ep_in[CDC_CMD_EP & 0xFU].is_used = 0U;

    /* DeInit  physical Interface components */
    if (pdev->pClassData)
    {
      //((USBD_CDC_ItfTypeDef *)pdev->pUserData)->DeInit();
      //USBD_free(pdev->pClassData);
       memset(pdev->pClassData, 0, sizeof(USBD_CDC_HandleTypeDef));
       pdev->pClassData = NULL;
    }
  }
  {
    MC_Switch_MSC(pdev);
    /* Close MSC EPs */
    USBD_LL_CloseEP(pdev, MSC_EPOUT_ADDR);
    pdev->ep_out[MSC_EPOUT_ADDR & 0xFU].is_used = 0U;

    /* Close EP IN */
    USBD_LL_CloseEP(pdev, MSC_EPIN_ADDR);
    pdev->ep_in[MSC_EPIN_ADDR & 0xFU].is_used = 0U;

    /* De-Init the BOT layer */
    MSC_BOT_DeInit(pdev);

    /* Free MSC Class Resources */
    if (pdev->pClassData)
    {
      //USBD_free(pdev->pClassData);
      memset(pdev->pClassData, 0, sizeof(USBD_MSC_BOT_HandleTypeDef));
      pdev->pClassData  = NULL;
    }
  }


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
  uint16_t status_info = 0U;
  if(req->wIndex == 0x0002)
  {
    MC_Switch_MSC(pdev);
    USBD_MSC_BOT_HandleTypeDef     *hmsc = (USBD_MSC_BOT_HandleTypeDef*) pdev->pClassData;
    uint16_t status_info = 0U;

    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {

    /* Class request */
    case USB_REQ_TYPE_CLASS :
      switch (req->bRequest)
      {
      case BOT_GET_MAX_LUN :
        if ((req->wValue  == 0U) && (req->wLength == 1U) &&
            ((req->bmRequest & 0x80U) == 0x80U))
        {
          hmsc->max_lun = (uint32_t)((USBD_StorageTypeDef *)pdev->pUserData)->GetMaxLun();
          USBD_CtlSendData(pdev, (uint8_t *)(void *)&hmsc->max_lun, 1U);
        }
        else
        {
           USBD_CtlError(pdev , req);
           return USBD_FAIL;
        }
        break;

      case BOT_RESET :
        if ((req->wValue  == 0U) && (req->wLength == 0U) &&
            ((req->bmRequest & 0x80U) != 0x80U))
        {
          MSC_BOT_Reset(pdev);
        }
        else
        {
           USBD_CtlError(pdev , req);
           return USBD_FAIL;
        }
        break;

      default:
         USBD_CtlError(pdev , req);
         return USBD_FAIL;
      }
      break;
    /* Interface & Endpoint request */
    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {

        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            USBD_CtlSendData(pdev, (uint8_t *)(void *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            return USBD_FAIL;
          }
          break;

      case USB_REQ_GET_INTERFACE :
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            USBD_CtlSendData(pdev, (uint8_t *)(void *)&hmsc->interface, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
             return USBD_FAIL;
          }
        break;

      case USB_REQ_SET_INTERFACE :
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            hmsc->interface = (uint8_t)(req->wValue);
          }
          else
          {
            USBD_CtlError(pdev, req);
            return USBD_FAIL;
          }
        break;

      case USB_REQ_CLEAR_FEATURE:

        /* Flush the FIFO and Clear the stall status */
        USBD_LL_FlushEP(pdev, (uint8_t)req->wIndex);

        /* Reactivate the EP */
        USBD_LL_CloseEP (pdev , (uint8_t)req->wIndex);
        if((((uint8_t)req->wIndex) & 0x80) == 0x80)
        {
          pdev->ep_in[(uint8_t)req->wIndex & 0xFU].is_used = 0U;
          if(pdev->dev_speed == USBD_SPEED_HIGH  )
          {
            /* Open EP IN */
            USBD_LL_OpenEP(pdev,
                           MSC_EPIN_ADDR,
                           USBD_EP_TYPE_BULK,
                           MSC_MAX_HS_PACKET);
          }
          else
          {
            /* Open EP IN */
            USBD_LL_OpenEP(pdev,
                           MC_MSC_EPIN_ADDR,
                           USBD_EP_TYPE_BULK,
                           MSC_MAX_FS_PACKET);
          }
           pdev->ep_in[MSC_EPIN_ADDR & 0xFU].is_used = 1U;
        }
        else
        {
          pdev->ep_out[(uint8_t)req->wIndex & 0xFU].is_used = 0U;
          if(pdev->dev_speed == USBD_SPEED_HIGH  )
          {
            /* Open EP IN */
            USBD_LL_OpenEP(pdev,
                           MC_MSC_EPOUT_ADDR,
                           USBD_EP_TYPE_BULK,
                           MSC_MAX_HS_PACKET);
          }
          else
          {
            /* Open EP IN */
            USBD_LL_OpenEP(pdev,
                           MC_MSC_EPOUT_ADDR,
                           USBD_EP_TYPE_BULK,
                           MSC_MAX_FS_PACKET);
          }
          pdev->ep_out[MSC_EPOUT_ADDR & 0xFU].is_used = 1U;
        }

        /* Handle BOT error */
        MSC_BOT_CplClrFeature(pdev, (uint8_t)req->wIndex);
        break;

      }
      break;

    default:
      break;
    }
  }
  else
  {
    MC_Switch_CDC(pdev);
    static uint8_t ifalt = 0;
    USBD_CDC_HandleTypeDef * hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;

    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
    case USB_REQ_TYPE_CLASS :
      if (req->wLength)
      {
        if (req->bmRequest & 0x80)
        {
          ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->Control(req->bRequest,
                                                            (uint8_t *)(void *)hcdc->data,
                                                            req->wLength);

          USBD_CtlSendData(pdev, (uint8_t *)(void *)hcdc->data, req->wLength);
        }
        else
        {
          hcdc->CmdOpCode = req->bRequest;
          hcdc->CmdLength = (uint8_t)req->wLength;

          USBD_CtlPrepareRx(pdev, (uint8_t *)(void *)hcdc->data, req->wLength);

        }

      }
      else
      {
        ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->Control(req->bRequest,
                                                          (uint8_t *)(void *)req, 0U);

      }
      break;

    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            USBD_CtlSendData(pdev, (uint8_t *)(void *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            return USBD_FAIL;
          }
          break;
      case USB_REQ_GET_INTERFACE :
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            USBD_CtlSendData(pdev, &ifalt, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            return USBD_FAIL;
          }
          break;

      case USB_REQ_SET_INTERFACE :
          if (pdev->dev_state != USBD_STATE_CONFIGURED)
          {
            USBD_CtlError(pdev, req);
            return USBD_FAIL;
          }
          break;
      }

    default:
      break;
    }
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

  eprintf("DataIn epnum: %x \r\n", epnum);
  if(epnum == (MC_MSC_EPIN_ADDR & 0x7f))
  {
    MC_Switch_MSC(pdev);
    MSC_BOT_DataIn(pdev , epnum);
  }
  else if(epnum == (MC_CDC_IN_EP & 0x7f))
  {
    USBD_CDC_HandleTypeDef   *hcdc;
    
    MC_Switch_CDC(pdev);
    hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;
    PCD_HandleTypeDef *hpcd = pdev->pData;
    if (pdev->pClassData != NULL)
    {
      if ((pdev->ep_in[epnum].total_length > 0U) && ((pdev->ep_in[epnum].total_length % hpcd->IN_ep[epnum].maxpacket) == 0U))
      {
        /* Update the packet total length */
        pdev->ep_in[epnum].total_length = 0U;

        /* Send ZLP */
        USBD_LL_Transmit(pdev, epnum, NULL, 0U);
      }
      else
      {
        hcdc->TxState = 0U;
      }
      return USBD_OK;
    }
    else
    {

      return USBD_FAIL;
    }
  }

  return USBD_OK;
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
    eprintf("DataIn epnum: %x \r\n", epnum);
  if(epnum == MC_MSC_EPOUT_ADDR)
  {
    MC_Switch_MSC(pdev);
    MSC_BOT_DataOut(pdev , epnum);
  }
  else if(epnum == MC_CDC_OUT_EP)
  {
    USBD_CDC_HandleTypeDef   *hcdc;

    MC_Switch_CDC(pdev);
    hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;

    hcdc->RxLength = USBD_LL_GetRxDataSize (pdev, epnum);
    ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->Receive(hcdc->RxBuffer, &hcdc->RxLength);
  }

  return USBD_OK;
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


/**
  * @}
  */

static uint8_t  USBD_MC_RxReady (USBD_HandleTypeDef *pdev)
{
  USBD_CDC_HandleTypeDef   *hcdc;

  MC_Switch_CDC(pdev);
  hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;

  if((pdev->pUserData != NULL) && (hcdc->CmdOpCode != 0xFF))
  {
    ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->Control(hcdc->CmdOpCode,
                                                      (uint8_t *)(void *)hcdc->data,
                                                      (uint16_t)hcdc->CmdLength);
      hcdc->CmdOpCode = 0xFF;

  }

  return USBD_OK;
}
//__attribute__((section(".RamD3Buffer")))
    USBD_MSC_BOT_HandleTypeDef msc_handle;
//__attribute__((section(".RamD3Buffer")))
    USBD_CDC_HandleTypeDef cdc_handle;

void MC_Switch_MSC(USBD_HandleTypeDef *pdev)
{

  USBD_MSC_RegisterStorage(pdev, &USBD_Storage_Interface_fops_HS);
  pdev->pClassData = &msc_handle;
}

void MC_Switch_CDC(USBD_HandleTypeDef *pdev)
{

  USBD_CDC_RegisterInterface(pdev, &USBD_Interface_fops_HS);
  pdev->pClassData = &cdc_handle;
}


