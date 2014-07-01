OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))

fs_client: fs_client.o fs_client_stub.o rpc_client_stub.o libstubs.a

$(OBJECTS): %.o: %.c ece454_fs.h
	gcc -c $< -o $@

clean:
	rm -rf a.out *.o core *.a fs_client

fs_server: fs_server.o fs_server_stub.o rpc_server_stub.o libstubs.a

client-api.a:
	gcc -c fs_client_stub.c

libstubs.a: rpc_server_stub.o rpc_client_stub.o rpc_helper.o rpc_my_bind.o
	ar r libstubs.a rpc_server_stub.o rpc_client_stub.o rpc_helper.o rpc_my_bind.o
