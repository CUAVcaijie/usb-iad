/*
 * debug.h
 *
 *  Created on: Jul 22, 2020
 *      Author: caijie
 */

#ifndef DEBUG_DEBUG_H_
#define DEBUG_DEBUG_H_

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define dprintf(fmt, ...) debug_printf(0, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define eprintf(fmt, ...) debug_printf(0, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define aprintf(fmt, ...) debug_printf(1, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)


/**
 * @name: ���Դ�ӡ���ݱ���
 * @param {chan ͨ����fmt ��ʽ������}
 * @return:
 */
void debug_printf(uint8_t type, const char *fun, uint32_t lie, char * fmt, ...);

/**
 * @name: ��ӡ�����������
 * @param {buf ���ݣ�len ���� }
 * @return:
 */
void debug_array(uint8_t *buf, uint16_t len);

/**
 * @name: ���Գ�ʼ��
 * @param {chan ͨ��}
 * @return:
 */
void debug_init(void);

/**
 * @name: ѭ��
 * @param {chan ͨ��}
 * @return:
 */
void debug_loop(void *argument);
#endif /* DEBUG_DEBUG_H_ */
