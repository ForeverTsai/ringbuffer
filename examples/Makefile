CC 	= gcc
CFLAGS 	= -Wall -g -O2

TARGET  = example
SRCS 	= ../ringbuffer.c example.c
OBJS 	= $(SRCS:.c=.o)

LOCAL_CFLAGS 	= -I../

%.o:%.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -o $@ -c $<

all: $(TARGET)

example: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
clean:
	rm -f $(OBJS) $(TARGET)
