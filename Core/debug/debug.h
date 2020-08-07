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
 * @name: 调试打印数据保存
 * @param {chan 通道，fmt 格式化参数}
 * @return:
 */
void debug_printf(uint8_t type, const char *fun, uint32_t lie, char * fmt, ...);

/**
 * @name: 打印数组里的数据
 * @param {buf 数据，len 长度 }
 * @return:
 */
void debug_array(uint8_t *buf, uint16_t len);

/**
 * @name: 调试初始化
 * @param {chan 通道}
 * @return:
 */
void debug_init(void);

/**
 * @name: 循环
 * @param {chan 通道}
 * @return:
 */
void debug_loop(void *argument);
#endif /* DEBUG_DEBUG_H_ */
