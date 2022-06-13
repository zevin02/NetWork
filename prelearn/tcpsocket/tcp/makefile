.PHONY:all
all:server client
server:server.cc
		g++ server.cc -o server -std=c++11 -pthread

client:client.cc
		g++ client.cc -o client -std=c++11

.PHONY:clean
clean:
		rm -f server client  
