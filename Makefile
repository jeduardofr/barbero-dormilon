main:
	@gcc -lpthread -lrt main.c -o sleeping_barber
	@./sleeping_barber
