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
 *  �������Խṹ��
 */
typedef struct {
    void *buff;         /*!< ����ָ��  */
    uint16_t data_len;  /*!< ���ݳ���  */
    uint32_t front;     /*!< ���ж�ͷ����  */
    uint32_t rear;      /*!< ���ж�β����  */
    uint16_t max_size;  /*!< �������  */
    uint8_t flag;       /*!< ��ʼ������  */
}own_queue_t;

/**
 * @name: ��������
 * @param {queue ���У�data �洢ָ�룬data_len ���ݳ��ȣ�number ��������}
 * @return: 0�ɹ���1ʧ��
 */
 int own_queue_init(own_queue_t *queue, void *data, uint16_t data_len, uint16_t number);

 /**
 * @name: ���ٶ���
 * @param {queue ����}
 * @return:
 */
void own_queue_free(own_queue_t *queue);

/**
 * @name: ��ն���
 * @param {queue ����}
 * @return:
 */
void own_queue_empty(own_queue_t *queue);

/**
 * @name: ���������ʣ�೤��
 * @param {queue ����}
 * @return:
 */
uint16_t own_queue_is_full(own_queue_t *queue);

/**
 * @name: �������������ݳ���
 * @param {queue ����}
 * @return:
 */
uint16_t own_queue_size(own_queue_t *queue);

/**
 * @name: ����������ͷλ��
 * @param {queue ����}
 * @return:
 */
uint16_t own_queue_front(own_queue_t *queue);

/**
 * @name: ����������βλ��
 * @param {queue ����}
 * @return:
 */
uint16_t own_queue_rear(own_queue_t *queue);

/**
 * @name: �����
 * @param {queue ���У� buf ����ָ�룬len ���ݳ���}
 * @return: 0�ɹ�
 */
int own_queue_enter(own_queue_t *queue, void *buf, uint16_t len);
/**
 * @name: ������
 * @param {queue ���У� buf ����ָ�룬len ���ݳ���}
 * @return: ����
 */
uint16_t own_queue_out(own_queue_t *queue,  void *buf, uint16_t len);


#endif /* DRIVE_OWN_QUEUE_OWN_QUEUE_H_ */
