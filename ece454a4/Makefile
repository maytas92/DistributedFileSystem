OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))

fs_client: fs_client.o client_api.o
	gcc fs_client.o client_api.o simplified_rpc/libstubs.a -o fs_client

fs_client2: fs_client2.o client_api.o
	gcc fs_client2.o client_api.o simplified_rpc/libstubs.a -o fs_client2

fs_client3: fs_client3.o client_api.o
	gcc fs_client3.o client_api.o simplified_rpc/libstubs.a -o fs_client3

server: fs_server.o libstubs.a
	gcc fs_server.o simplified_rpc/libstubs.a -o fsServer

client-api.a: libstubs.a client_api.o
				ar r libclient-api.a client_api.o simplified_rpc/server_stub.o simplified_rpc/client_stub.o simplified_rpc/helper.o simplified_rpc/mybind.o

$(OBJECTS): %.o: %.c ece454_fs.h
	gcc -c $< -o $@

libstubs.a:
	${MAKE} -C simplified_rpc clean
	${MAKE} -C simplified_rpc libstubs.a

clean:
	rm -rf a.out *.o core *.a fs_client server
