#include "fifo.h"

void fifo_init(fifo_t *f)
{
    f->head = 0;
    f->tail = 0;
}

void fifo_put(fifo_t *f, u8 ch)
{
    u16 next = (f->head + 1) & (FIFO_SIZE - 1);
    if (next != f->tail) {          // 满则丢弃
        f->buf[f->head] = ch;
        f->head = next;
    }
}

u8 fifo_get(fifo_t *f, u8 *ch)
{
    if (f->tail == f->head)
        return 0;                   // 空
    *ch = f->buf[f->tail];
    f->tail = (f->tail + 1) & (FIFO_SIZE - 1);
    return 1;
}

u16 fifo_avail(fifo_t *f)
{
    return (f->head - f->tail) & (FIFO_SIZE - 1);
}

void fifo_flush(fifo_t *f)
{
    f->tail = f->head;
}
