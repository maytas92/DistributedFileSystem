OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))

client: fs_client.o client_api.o
	gcc fs_client.o client_api.o simplified_rpc/libstubs.a -o client

server: fs_server.o libstubs.a
	gcc fs_server.o simplified_rpc/libstubs.a -o server

$(OBJECTS): %.o: %.c ece454_fs.h
	gcc -c $< -o $@

libstubs.a:
	${MAKE} -C simplified_rpc clean
	${MAKE} -C simplified_rpc libstubs.a

clean:
	rm -rf a.out *.o core *.a fs_client server
