//Go 模拟实现Nagle算法
/*
	实现Nagle算法的核心就是在发送数据的时候进行缓存和延迟

*/
package main

import (
	"net"
	"sync"
	"time"
)

//这里使用互斥锁来保证线程安全
var mu sync.Mutex

func main() {

	//conn变量代表与服务段的TCP连接，这个是一个客户端
	conn, err := net.Dial("tcp", "localhost:8080")
	if err != nil {
		panic(err)
	}
	//用于在函数执行完之后，自动断开连接
	defer conn.Close()
	//模拟Nagle算法：缓存小的数据包，在一定事件或者缓存到一定程度的时候发送
	//sendbuffer只哦叛逆用来缓冲需要发送的小数据包，等待合并
	sendBuffer := make([]byte, 0) //创建一个空的字节片,长度为0,可以将空的字节片看作一个缓冲区，在需要的时候进行动态扩容
	maxSendBufferSize := 1024     //需要缓存的最大容量
	delayedSecond := time.Second  //发送的延迟时间,发送小数据包等待的最大时间
	lastSendTime := time.Now()    //当前时间

	//向服务器发送数据
	msg1 := "hello"
	msg2 := "world"
	//将数据缓存到,
	//将byte数据一个字节一个字节添加到sendBuffer中
	sendBuffer = append(sendBuffer, []byte(msg1)...)
	sendBuffer = append(sendBuffer, []byte(msg2)...)
	for {
		mu.Lock()
		if len(sendBuffer) > 0 {
			idleSecond := time.Since(lastSendTime) //计算距离上一次有多少时间
			//如果当前缓存的数据量达到了指定的发送大小或者时间已经达到了超时时间。就需要发送
			if len(sendBuffer) >= maxSendBufferSize || (idleSecond >= delayedSecond && len(sendBuffer) > 0) {
				n, err := conn.Write(sendBuffer)
				if err != nil || n == 0 {
					panic(err)
				}
				lastSendTime = time.Now()
				//因为已经成功发送了n个字节，所以就需要从n位置开始截取新的切片，进行下一次的发送
				sendBuffer = sendBuffer[n:] //这个截取一个切片

			}

		}
		mu.Unlock()
	}
}
