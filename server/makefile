main: server.o status.o command.o message_transport.o file_transport.o info_handle.o client
	gcc server.o status.o command.o message_transport.o file_transport.o info_handle.o -o server -pthread
	gcc client.c -o client
command: command.c command.h command_define.h
	gcc -c command.c -o command.o command_define.h
file_transport: file_transport.c file_transport.h command_define.h
	gcc -c file_transport.c -o file_transport.o
message_transport: message_transport.c message_transport.h command_define.h
	gcc -c message_transport.c -o message_transport.o
server: server.c command_define.h
	gcc -c server.c -o server.o
status: status.c status.h command_define.h
	gcc -c status.c -o status.o
info_handle:
	gcc -c info_handle.c -o info_handle.o
clean:
	rm *.o