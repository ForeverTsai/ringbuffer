#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

typedef struct ringbuffer ringbuffer_t;

ringbuffer_t *ringbuffer_init(int size);
void ringbuffer_release(ringbuffer_t *rb);
int ringbuffer_enqueue(ringbuffer_t *rb, void *buf, int size);
int ringbuffer_dequeue(ringbuffer_t *rb, void *buf, int size, int timeout);
#endif /* _RINGBUFFER_H_ */
