//客户端代码
package main

import (
	"net"
)

func main() {
	conn, err := net.Dial("tcp", "localhost:8080")
	if err != nil {
		panic(err)
	}
	defer conn.Close()
	//循环发送20次，helloworld
	for i := 0; i < 20; i++ {
		msg := "hello world"
		conn.Write([]byte(msg)) //发送数据
	}

}
