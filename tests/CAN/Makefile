CC = gcc
CFLAGS = -Wall -Wextra -I./rt_service/CAN -I./shared_memory
LDFLAGS =

all: can_test

can_test: can_test.c rt_service/CAN/can.c
	$(CC) $(CFLAGS) -o $@ can_test.c rt_service/CAN/can.c

clean:
	rm -f can_test
