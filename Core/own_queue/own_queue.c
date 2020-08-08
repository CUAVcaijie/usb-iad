/*
 * own_queue.c
 *
 *  Created on: Jul 17, 2020
 *      Author: caijie
 */
#include "own_queue.h"

/**
 * @name: ��������
 * @param {queue ���У�data �洢ָ�룬data_len ���ݳ��ȣ�number ��������}
 * @return: 0�ɹ���1ʧ��
 */
 int own_queue_init(own_queue_t *queue, void *data, uint16_t data_len, uint16_t number)
{
     queue->buff = data;
     if (queue->buff == NULL) {
        return 1;
    }
    queue->data_len = data_len;
    queue->front = 0;
    queue->rear = 0;
    queue->max_size = number;
    queue->flag = 1;
    return 0;
}
/**
 * @name: ���ٶ���
 * @param {queue ����}
 * @return:
 */
void own_queue_free(own_queue_t *queue)
{
    if (queue->flag == 1) {
        queue->buff = NULL;
        queue->flag = 0;
    }
}
/**
 * @name: ��ն���
 * @param {queue ����}
 * @return:
 */
void own_queue_empty(own_queue_t *queue)
{
    queue->front = 0;
    queue->rear = 0;
}
/**
 * @name: ���������ʣ�೤��
 * @param {queue ����}
 * @return:
 */
uint16_t own_queue_is_full(own_queue_t *queue)
{
    if (queue->buff == NULL) {
        return 0;
    }
    return queue->max_size - (queue->front - queue->rear);
}
/**
 * @name: �������������ݳ���
 * @param {queue ����}
 * @return:
 */
uint16_t own_queue_size(own_queue_t *queue)
{
    if (queue->buff == NULL) {
        return 0;
    }
    return queue->front - queue->rear;
}
/**
 * @name: ����������ͷλ��
 * @param {queue ����}
 * @return:
 */
uint16_t own_queue_front(own_queue_t *queue)
{
    return (queue->front%queue->max_size);
}
/**
 * @name: ����������βλ��
 * @param {queue ����}
 * @return:
 */
uint16_t own_queue_rear(own_queue_t *queue)
{
    return (queue->rear%queue->max_size);
}

/**
 * @name: ���鸴��
 * @param {queue ����}
 * @return:
 */
void own_queue_cpy(uint32_t *destin, uint32_t *source, uint32_t n)
{
    for(uint32_t i = 0; i < n; i++) {
        *destin++ = *source++;
    }
}

/**
 * @name: �����
 * @param {queue ���У� buf ����ָ�룬len ���ݳ���}
 * @return: 0�ɹ���1û�г�ʼ����2��������,��3�����������Ѿ�����һ��������
 */
int own_queue_enter(own_queue_t *queue, void *buf, uint16_t len)
{
    uint16_t trace_len = 0, offset = 0;
    int ret = 0;

    if (queue->flag != 1) {
        return 1;
    }

    trace_len = own_queue_is_full(queue);
    if (trace_len == 0) {
        return 2;
    }
    if (len <= trace_len) {
        trace_len = len;
    } else {
        ret = 3;
    }

    if (queue->front > 0x1FFFFFFF) {
        queue->front = own_queue_front(queue);
        queue->rear = own_queue_rear(queue);
    }

    if ((queue->max_size - own_queue_front(queue)) < trace_len) {
        own_queue_cpy((queue->buff + own_queue_front(queue) * queue->data_len),
                buf,
                (queue->data_len * (queue->max_size - own_queue_front(queue))));

        offset = queue->max_size - own_queue_front(queue);
        trace_len = trace_len - (queue->max_size - own_queue_front(queue));
        queue->front += (queue->max_size - own_queue_front(queue));
    }

    own_queue_cpy((queue->buff + own_queue_front(queue)*queue->data_len),
            (buf + offset * queue->data_len),
            (trace_len * queue->data_len));

    queue->front +=trace_len;
    return ret;
}
/**
 * @name: ������
 * @param {queue ���У� buf ����ָ�룬len ���ݳ���}
 * @return: 0û�г�ʼ������û������
 */
uint16_t own_queue_out(own_queue_t *queue,  void *buf, uint16_t len)
{
    uint16_t trace_len = 0;
    uint16_t offset = 0;

    if (queue->flag != 1) {
        return 0;
    }

    trace_len = own_queue_size(queue);
    if (trace_len > len) {
        trace_len = len;
    }

    if (trace_len == 0) {
        return 0;
    }
    if ((queue->max_size - own_queue_rear(queue)) < trace_len) {
        own_queue_cpy(buf,
                (queue->buff + own_queue_rear(queue) * queue->data_len),
                ((queue->max_size - own_queue_rear(queue)) * queue->data_len));

        offset = (queue->max_size - own_queue_rear(queue));
        trace_len = trace_len - offset;
        queue->rear += (queue->max_size - own_queue_rear(queue));
    }

    own_queue_cpy((buf+offset * queue->data_len),
            (queue->buff + own_queue_rear(queue) * queue->data_len),
            (trace_len * queue->data_len));
    queue->rear += trace_len;
    return trace_len + offset;
}
