/*
 * debug.c
 *
 *  Created on: Jul 22, 2020
 *      Author: caijie
 */
#include "debug.h"
#include "main.h"
#include "own_queue.h"
#include <stm32h743xx.h>

__attribute__((section(".RamD3Buffer"))) uint8_t uart_rx_buf[4096];
uint8_t debug_flag = 0;
extern UART_HandleTypeDef huart7;

/**
 * @name: 串口传输完成回调
 * @param {hdma 对应的DMA}
 * @return:
 */
void uart_tx_cplt_callback(DMA_HandleTypeDef *hdma)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)(hdma->Parent);
    huart->gState = HAL_UART_STATE_READY;
    debug_flag = 0;
}

/**
 * @name: DMA发送数据
 * @param {huart 串口，data 发送数据，size 发送的大小}
 * @return:1错误，2忙，0成功
 */
HAL_StatusTypeDef uart_dma_transmit(UART_HandleTypeDef *huart, uint8_t *data, uint16_t size)
{
  /* Check that a Tx process is not already ongoing */
  if (huart->gState == HAL_UART_STATE_READY)
  {
    if ((data == NULL) || (size == 0U))
    {
      return HAL_ERROR;
    }

    __HAL_LOCK(huart);

    huart->pTxBuffPtr  = data;
    huart->TxXferSize  = size;
    huart->TxXferCount = size;

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->gState = HAL_UART_STATE_BUSY_TX;

    if (huart->hdmatx != NULL)
    {
      /* Set the UART DMA transfer complete callback */
      huart->hdmatx->XferCpltCallback = uart_tx_cplt_callback;

      /* Set the UART DMA Half transfer complete callback */
      //static void UART_DMATxHalfCplt(DMA_HandleTypeDef *hdma)
      huart->hdmatx->XferHalfCpltCallback = NULL;

      /* Set the DMA error callback */
      huart->hdmatx->XferErrorCallback = NULL;

      /* Set the DMA abort callback */
      huart->hdmatx->XferAbortCallback = NULL;

      /* Enable the UART transmit DMA channel */
      if (HAL_DMA_Start_IT(huart->hdmatx, (uint32_t)huart->pTxBuffPtr, (uint32_t)&huart->Instance->TDR, size) != HAL_OK)
      {
        /* Set error code to DMA */
        huart->ErrorCode = HAL_UART_ERROR_DMA;

        __HAL_UNLOCK(huart);

        /* Restore huart->gState to ready */
        huart->gState = HAL_UART_STATE_READY;

        return HAL_ERROR;
      }
    }
    /* Clear the TC flag in the ICR register */
    __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_TCF);

    __HAL_UNLOCK(huart);

    /* Enable the DMA transfer for transmit request by setting the DMAT bit
    in the UART CR3 register */
    SET_BIT(huart->Instance->CR3, USART_CR3_DMAT);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}
/**
 * @name: 留给外部调用发送数据
 * @param {chan 通道，buf 发送的数据，len 发送数据的长度}
 * @return:
 */
uint16_t uart_write_data(uint8_t *buf, uint16_t len)
{
    debug_flag = 1;
    HAL_StatusTypeDef ret = uart_dma_transmit(&huart7, buf, len);
    if (ret == HAL_OK) {
        while(debug_flag){

        }
    }
    return len;
}

uint8_t debug_buf[4096*2];
char debug_temp[1024];
own_queue_t debug_queue;          /*!< 调试数据缓存*/
extern UART_HandleTypeDef huart7;
int _write(int file , char *ptr,int len)
{
    own_queue_enter(&debug_queue, ptr, len);
    return len;
}
/**
 * @name: 调试打印数据保存
 * @param {chan 通道，fmt 格式化参数}
 * @return:
 */
void debug_printf(uint8_t type, const char *fun, uint32_t lie, char * fmt, ...)
{

    static char buf[128] = {0};

    uint16_t len = 0;

    va_list args;
    va_start(args, fmt);
    len = vsprintf(buf, fmt, args);
    va_end(args);

    if (type == 0) {
        len = sprintf(debug_temp, "[%d %s(%ld)] %s", HAL_GetTick(), fun, lie, buf);
        own_queue_enter(&debug_queue, (void *)debug_temp, len);
        //HAL_UART_Transmit(&huart7, debug_temp, len, 1000);
    }
    else if (type == 1) {
        own_queue_enter(&debug_queue, (void *)buf, len);
        //HAL_UART_Transmit(&huart7, buf, len, 1000);
    }
}

/**
 * @name: 打印数组里的数据
 * @param {buf 数据，len 长度 }
 * @return:
 */
void debug_array(uint8_t *buf, uint16_t len)
{
    own_queue_enter(&debug_queue, (void *)buf, len);
}

void debug_loop(void *argument)
{

    while(own_queue_size(&debug_queue)) {
        uint16_t len = own_queue_out(&debug_queue,  uart_rx_buf, 4096);
        if (len >0) {
            uart_write_data(uart_rx_buf, len);
        }
       // HAL_UART_Transmit(&huart7, buf, len, 1000);
    }
}

void debug_init(void)
{
    own_queue_init(&debug_queue, debug_buf, 1, 4096);

}


