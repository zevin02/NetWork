就绪事件通知机制

# select1

select 只负责等待，没有读取和接收的操作
可以用作多个文件描述符检测的功能

read 和write本身也有等待的功能，但是一次只能传入一个fd

select可以同时等待多个fd，这样read就直接取读取需要读取的功能

# 函数
       int select(int nfds, fd_set *readfds, fd_set *writefds,
                  fd_set *exceptfds, struct timeval *timeout
                  ）

nfd=maxfd+1
fd_set是一个位图结构，比特位的位置代表它是第几个文件描述符
，比特位的内容代表该文件描述符是否有存在
s
是一个输入输出型参数




我们想要的是两个结果
读就绪（底层已经从网络中拿到系统当中），写就绪（发送缓冲区的剩余空间，可以供用户把数据拷贝到内核层发送给对方了）
异常就绪

