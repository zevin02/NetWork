.PHONY:all
all:server client
server:server.cc
		g++ server.cc -o server -g -std=c++11 -lpthread -ljsoncpp

client:client.cc
		g++ client.cc -o client -g -std=c++11 -lpthread -ljsoncpp

.PHONY:clean
clean:
		rm -f server client  