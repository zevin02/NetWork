package main

//服务端代码
import (
	"bufio" //提供io操作的包
	"fmt"
	"io"  //专门进行io读取写入的包
	"net" //专门进行网络通行的包
)

//出现了粘包问题
//1.可能是由于Nagle算法导致的粘包
//2.接收端接收不及时导致的粘包
func main() {
	network := "tcp"
	address := "localhost:8080"
	//创建一个监听套接字
	listen, err := net.Listen(network, address)
	if err != nil {
		panic(err)
	}
	//养成好习惯，及时关闭
	defer listen.Close()
	for {
		//accept等待接收客户端的连接,conn里面就是新连接上来的东西，相当于客户端的文件描述符
		conn, err := listen.Accept()
		if err != nil {
			panic(err)
		}
		//创建一个新的goruntine来执行process命令，go开头后面跟一个函数，代表开一个线程来执行这个函数
		go process(conn)

	}

}

func process(conn net.Conn) {
	defer conn.Close()
	//bufio里面提供了IO操作的库，
	reader := bufio.NewReader(conn)
	//这里指定了buf为1024字节的字节切片
	var buf [1024]byte
	for {
		//把数据全部读取到buf中,buf[:]表示buf数组的切片，该切片可以访问数组中的所有元素,把数据存储到buf中
		//n表示读取到了多少个字节
		n, err := reader.Read(buf[:])
		if err == io.EOF {
			break
		}
		if err != nil {
			panic(err)
		}
		//从buf中读取一个切片，为0到n-1,大小为n字节
		//再把他放到一个string里面
		recvStr := string(buf[:n])
		fmt.Println(recvStr)
	}
}
