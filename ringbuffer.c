#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "ringbuffer.h"

/* common event function
 * you should use another api instead of it, such as
 * xEventGroupCreate, xEventGroupSetBits
 * */
typedef struct common_event {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	uint32_t event, mask;
	int state;
} cevent_t;

cevent_t *cevent_init(void)
{
	cevent_t *ev;

	ev = calloc(1, sizeof(cevent_t));
	if (!ev)
		return NULL;
	pthread_mutex_init(&ev->mutex, NULL);	
	pthread_cond_init(&ev->cond, NULL);
	return ev;	
}

void cevent_set(cevent_t *ev, uint32_t bits)
{
	if (!ev || ev->state != 0)
		return;
	pthread_mutex_lock(&ev->mutex);
	ev->event |= bits;
	if ((ev->event & ev->mask) != 0)
		pthread_cond_signal(&ev->cond);
	pthread_mutex_unlock(&ev->mutex);
	return;
}

int cevent_get(cevent_t *ev, uint32_t bits, int32_t ms)
{
	int32_t event;

	if (!ev || ev->state != 0 || !(bits & ev->mask))
		return -1;
	pthread_mutex_lock(&ev->mutex);
	while (!(ev->event & ev->mask) && !ev->state)
		pthread_cond_wait(&ev->cond, &ev->mutex);
	event = ev->event;
	ev->event = 0;
	pthread_mutex_unlock(&ev->mutex);
	return event;
}

void cevent_release(cevent_t *ev)
{
	if (!ev)
		return;
	ev->state = -1;
	cevent_set(ev, 0);
	pthread_mutex_destroy(&ev->mutex);
	pthread_cond_destroy(&ev->cond);
	free(ev);
}


/*-----------------------*/

#define RB_READ 	(1 << 0)
#define RB_WRITE 	(1 << 1)

/* some can't define this macro, such as run ringbuffer_enqueue during irq handler */
#define RB_ENQUEUE_MUTEX

struct ringbuffer {
	uint8_t *buffer;
	uint32_t length;
	unsigned int start, end;
	uint32_t isfull;
	pthread_mutex_t mutex;
	cevent_t *ev;
};

ringbuffer_t *ringbuffer_init(int size)
{
	ringbuffer_t *rb;

	if (size < 0)	
		return NULL;
	rb = calloc(1, sizeof(ringbuffer_t));
	if (!rb)
		return NULL;
	rb->ev = cevent_init();
	if (!rb->ev) {
		free(rb);
		return NULL;
	}
	rb->buffer = calloc(1, size);
	if (!rb->buffer) {
		cevent_release(rb->ev);
		free(rb);
		return NULL;
	}
	rb->length = size;
	rb->start = rb->end = 0;
	pthread_mutex_init(&rb->mutex, NULL);
	return rb;
}

void ringbuffer_release(ringbuffer_t *rb)
{
	if (!rb)
		return;
	if (rb->buffer)
		free(rb->buffer);
	cevent_release(rb->ev);
	pthread_mutex_destroy(&rb->mutex);
	free(rb);
}

int ringbuffer_enqueue(ringbuffer_t *rb, void *buf, int size)
{
	int len, cross = 0;
	if (!rb)
		return -1;
	if (rb->isfull) {
		/* notify to dequeue msg */
		cevent_set(rb->ev, RB_WRITE);
		return 0;
	}
#ifdef RB_ENQUEUE_MUTEX
	pthread_mutex_lock(&rb->mutex);
#endif
	if (rb->start > rb->end)
		len = rb->start - rb->end;
	else
		len = rb->length - (rb->end - rb->start);
	len = len > size ? size : len;
	if (rb->end + len > rb->length)
		cross = 1;
	if (cross != 0) {
		int first = rb->length - rb->end;
		memcpy(rb->buffer + rb->end, buf, first);
		memcpy(rb->buffer, buf + first, len - first);
		rb->end = len - first;
	} else {
		memcpy(rb->buffer + rb->end, buf, len);
		rb->end += len;
		rb->end %= rb->length;
	}
	if (rb->end == rb->start && len != 0)
		rb->isfull = 1;
#ifdef RB_ENQUEUE_MUTEX
	pthread_mutex_unlock(&rb->mutex);
#endif
	if (rb->isfull || rb->start != rb->end)
		cevent_set(rb->ev, RB_WRITE);
	return len;
}

int ringbuffer_dequeue(ringbuffer_t *rb, void *buf, int size, int timeout)
{
	int len, cross = 0;

	if (!rb)
		return -1;
	pthread_mutex_lock(&rb->mutex);
	if (rb->isfull) {
		len = rb->length;
		goto cal_actual_size;
	}
	if (rb->end - rb->start == 0 && timeout >= 0) {
		pthread_mutex_unlock(&rb->mutex);
		cevent_get(rb->ev, RB_WRITE, timeout);
		pthread_mutex_lock(&rb->mutex);
	}
	len = rb->end - rb->start;
	if (len == 0) {
		pthread_mutex_unlock(&rb->mutex);
		return 0;	
	} else if (len < 0) {
		len += rb->length;
	} else if (len > rb->length)
		fprintf(stderr, "sth error, len=%d, but rb->length=%u\n",
			len, rb->length);
cal_actual_size:
	len = len > size ? size : len;
	if (rb->start + len >= rb->length)
		cross = 1;
	if (cross != 0) {
		int first = rb->length - rb->start;
		memcpy(buf, rb->buffer + rb->start, first);
		memcpy(buf + first, rb->buffer, len - first);
		rb->start = len - first;	
	} else {
		memcpy(buf, rb->buffer + rb->start, len);
		rb->start += len;
	}
	if (rb->isfull && len != 0)
		rb->isfull = 0;
	pthread_mutex_unlock(&rb->mutex);
	return len;
}









