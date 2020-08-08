/*
 * debug.c
 *
 *  Created on: Jul 22, 2020
 *      Author: caijie
 */
#include "debug.h"
#include "own_queue.h"
#include "usbd_cdc_if.h"
#include <stm32h743xx.h>


uint8_t debug_buf[4096];
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
        //own_queue_enter(&debug_queue, (void *)debug_temp, len);
        HAL_UART_Transmit(&huart7, debug_temp, len, 1000);
    }
    else if (type == 1) {
        //own_queue_enter(&debug_queue, (void *)buf, len);
        HAL_UART_Transmit(&huart7, buf, len, 1000);
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
    static uint8_t buf[512];

//    while(own_queue_size(&debug_queue)) {
//        uint16_t len = own_queue_out(&debug_queue,  buf, 512);
//       // HAL_UART_Transmit(&huart7, buf, len, 1000);
//    }
}

void debug_init(void)
{
    own_queue_init(&debug_queue, debug_buf, 1, 4096);

}


