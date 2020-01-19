环形缓冲区接口(RingBuffer Interface)
====================

提供了基本的环形缓冲区接口,包括创建，销毁，写入和取出。
 * 支持阻塞取出数据。
 * 取出数据时支持超时等待
 * 写入数据时支持事件发送

代码默认依赖posix层接口，如果在RTOS上运行，可以将posix层接口替换为RTOS上的对应函数,例如mutex类接口，event类接口等



接口介绍
----------------
1. 创建环形缓冲区

```
ringbuffer_t *ringbuffer_init(int size)

param
size:   缓冲区大小，byte

return value
返回非NULL表示正常，返回NULL表示失败

```

2. 销毁环形缓冲区

```
void ringbuffer_release(ringbuffer_t *rb)

param
rb:     缓冲区句柄
```
3. 往环形缓冲区写入数据

```
int ringbuffer_enqueue(ringbuffer_t *rb, void *buf, int size)

param
rb:     缓冲区句柄
buf:    写入数据
size:   写入数据大小

return value
正常返回写入的数据大小，异常时返回小于0
```
4. 从环形缓冲区取出数据

```
int ringbuffer_dequeue(ringbuffer_t *rb, void *buf, int size, int timeout)

param
rb:       缓冲区句柄
buf:      取出数据存放的buffer
size:     要取出数据的大小
timeout:  超时时间，毫秒

return value
正常返回取出数据大小，异常时返回值小于0
```

使用例子
----------------
参考example.c
