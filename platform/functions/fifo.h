#ifndef _FIFO_H
#define _FIFO_H

#include "include.h"

#define FIFO_SIZE   512     // 必须为 2 的幂

typedef struct {
    volatile u8  buf[FIFO_SIZE];
    volatile u16 head;      // 生产者 (ISR) 写入
    volatile u16 tail;      // 消费者 (主循环) 读取
} fifo_t;

void fifo_init(fifo_t *f);
void fifo_put(fifo_t *f, u8 ch);    // ISR 安全
u8   fifo_get(fifo_t *f, u8 *ch);   // 返回 0=空, 1=成功
u16  fifo_avail(fifo_t *f);
void fifo_flush(fifo_t *f);

#endif
