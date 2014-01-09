all:
	gcc main.c -lpthread -o Debug/main
	gcc tickets.c -lpthread -o Debug/tickets
	gcc threads_buffers.c -o Debug/threads_buffers
	gcc lslr.c -o Debug/lslr
