.PHONY:all
all:server client serverp
server:server.cc
		g++ server.cc -o server -g
serverp:server.cc
		g++ serverpthread.cc -o serverp -g -lpthread

client:client.cc
		g++ client.cc -o client -g

.PHONY:clean
clean:
		rm -f server client  serverp