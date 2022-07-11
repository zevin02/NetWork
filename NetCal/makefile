.PHONY:all
all:server client
server:server.cc
		g++ server.cc -o server -g -std=c++11 -lpthread

client:client.cc
		g++ client.cc -o client -g -std=c++11 -lpthread

.PHONY:clean
clean:
		rm -f server client  