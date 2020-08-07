/*
 * debug.c
 *
 *  Created on: Jul 22, 2020
 *      Author: caijie
 */
#include "debug.h"
#include "own_queue.h"
#include "usbd_cdc_if.h"

uint8_t debug_buf[4096];
char debug_temp[1024];
own_queue_t debug_queue;          /*!< �������ݻ���*/


int _write8(int file , char *ptr,int len)
{
    own_queue_enter(&debug_queue, ptr, len);
    return len;
}

/**
 * @name: ���Դ�ӡ���ݱ���
 * @param {chan ͨ����fmt ��ʽ������}
 * @return:
 */
void debug_printf(uint8_t type, const char *fun, uint32_t lie, char * fmt, ...)
{
    char buf[128] = {0};

    uint16_t len = 0;

    va_list args;
    va_start(args, fmt);
    len = vsprintf(buf, fmt, args);
    va_end(args);

    if (type == 0) {
        len = sprintf(debug_temp, "[%s(%ld)] %s", fun, lie, buf);
        own_queue_enter(&debug_queue, (void *)debug_temp, len);
    }
    else if (type == 1) {
        own_queue_enter(&debug_queue, (void *)buf, len);
    }
}

/**
 * @name: ��ӡ�����������
 * @param {buf ���ݣ�len ���� }
 * @return:
 */
void debug_array(uint8_t *buf, uint16_t len)
{
    own_queue_enter(&debug_queue, (void *)buf, len);
}

void debug_loop(void *argument)
{
    static uint8_t buf[1024];

    while(own_queue_size(&debug_queue)) {
        uint16_t len = own_queue_out(&debug_queue,  buf, 1024);
        HAL_UART_Transmit((UART_HandleTypeDef *)argument, buf, len, 1000);
    }
}


void debug_init(void)
{
    own_queue_init(&debug_queue, debug_buf, 1, 4096);
}


