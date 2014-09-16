all:server client

client:client.o
	gcc -DTHREADED -WL,-rpath,/usr/local/lib -lzookeeper_mt -lpthread -o $@ $^

client.o:client.c
	gcc -g -Wall -I/usr/local/include/zookeeper/ -o $@ -c $^

server:server.o
	gcc -DTHREADED -WL,-rpath,/usr/local/lib -lzookeeper_mt -lpthread -o $@ $^

server.o:server.c
	gcc -g -Wall -I/usr/local/include/zookeeper/ -o $@ -c $^

.PHONY:clean

clean:
	rm server.o server client.o client
