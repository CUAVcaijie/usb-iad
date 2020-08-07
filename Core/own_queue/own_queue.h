/*
 * own_queue.h
 *
 *  Created on: Jul 17, 2020
 *      Author: caijie
 */

#ifndef DRIVE_OWN_QUEUE_OWN_QUEUE_H_
#define DRIVE_OWN_QUEUE_OWN_QUEUE_H_

#include <stdint.h>
#include <stddef.h>
#include <string.h>

/**
 *  队列属性结构体
 */
typedef struct {
    void *buff;         /*!< 数组指针  */
    uint16_t data_len;  /*!< 数据长度  */
    uint32_t front;     /*!< 队列队头索引  */
    uint32_t rear;      /*!< 队列队尾索引  */
    uint16_t max_size;  /*!< 最大容量  */
    uint8_t flag;       /*!< 初始化标致  */
}own_queue_t;

/**
 * @name: 创建队列
 * @param {queue 队列，data 存储指针，data_len 数据长度，number 数据数量}
 * @return: 0成功，1失败
 */
 int own_queue_init(own_queue_t *queue, void *data, uint16_t data_len, uint16_t number);

 /**
 * @name: 销毁队列
 * @param {queue 队列}
 * @return:
 */
void own_queue_free(own_queue_t *queue);

/**
 * @name: 清空队列
 * @param {queue 队列}
 * @return:
 */
void own_queue_empty(own_queue_t *queue);

/**
 * @name: 输出队列里剩余长度
 * @param {queue 队列}
 * @return:
 */
uint16_t own_queue_is_full(own_queue_t *queue);

/**
 * @name: 输出队列里的数据长度
 * @param {queue 队列}
 * @return:
 */
uint16_t own_queue_size(own_queue_t *queue);

/**
 * @name: 输出队列里队头位置
 * @param {queue 队列}
 * @return:
 */
uint16_t own_queue_front(own_queue_t *queue);

/**
 * @name: 输出队列里队尾位置
 * @param {queue 队列}
 * @return:
 */
uint16_t own_queue_rear(own_queue_t *queue);

/**
 * @name: 入队列
 * @param {queue 队列， buf 数组指针，len 数据长度}
 * @return: 0成功
 */
int own_queue_enter(own_queue_t *queue, void *buf, uint16_t len);
/**
 * @name: 出队列
 * @param {queue 队列， buf 数组指针，len 数据长度}
 * @return: 长度
 */
uint16_t own_queue_out(own_queue_t *queue,  void *buf, uint16_t len);


#endif /* DRIVE_OWN_QUEUE_OWN_QUEUE_H_ */
