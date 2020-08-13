/*
 * usbd_mc.h
 *
 *  Created on: 2020Äê8ÔÂ12ÈÕ
 *      Author: caijie
 */

#ifndef APP_USBD_MC_H_
#define APP_USBD_MC_H_

#include  "usbd_ioreq.h"
#include "usbd_cdc.h"
#include "usbd_msc.h"

#define MC_MAX_FS_PACKET            CDC_DATA_HS_MAX_PACKET_SIZE

#define USB_MC_CONFIG_DESC_SIZ      106

#define MC_MSC_EPIN_ADDR                MSC_EPIN_ADDR
#define MC_MSC_EPOUT_ADDR               MSC_EPOUT_ADDR

#define MC_CDC_IN_EP                   CDC_IN_EP
#define MC_CDC_OUT_EP                  CDC_OUT_EP
#define MC_CDC_CMD_EP                  CDC_CMD_EP

extern USBD_ClassTypeDef  USBD_MC;

void MC_Switch_MSC(USBD_HandleTypeDef *pdev);
void MC_Switch_CDC(USBD_HandleTypeDef *pdev);

#endif /* APP_USBD_MC_H_ */
