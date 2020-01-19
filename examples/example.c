#include <stdio.h>
#include <string.h>
#include <ringbuffer.h>

int main(int argc, char *argv[])
{
	char buf[512];
	ringbuffer_t *rb;
	char *test = "For test...";
	int len;

	printf("ringbuffer init...\n");
	rb = ringbuffer_init(1024);
	if (!rb)
		return -1;

	printf("ringbuffer enqueue...\n");
	len = ringbuffer_enqueue(rb, test, strlen(test));
	printf("enqueue return %d\n", len);

	
	printf("ringbuffer dequeue...\n");
	memset(buf, 0, sizeof(buf));
	len = ringbuffer_dequeue(rb, buf, sizeof(buf), 50);
	printf("dequeue return %d\n", len);
	if (len > 0) {
		printf("dequeue msg:%s\n", buf);
	}

	ringbuffer_release(rb);
	return 0;
}
