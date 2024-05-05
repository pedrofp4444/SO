CC = gcc
CFLAGS = -Wall -g -Iinclude
LDFLAGS =

all: folders orchestrator client

orchestrator: bin/orchestrator

client: bin/client

folders:
	@mkdir -p src include obj bin tmp

bin/orchestrator: obj/orchestrator.o obj/utils.o obj/taskmanager.o
	$(CC) $(LDFLAGS) $^ -o $@

bin/client: obj/client.o obj/utils.o obj/taskmanager.o
	$(CC) $(LDFLAGS) $^ -o $@

bin/utils: obj/utils.o
	$(CC) $(LDFLAGS) $^ -o $@

bin/taskmanager: obj/taskmanager.o
	$(CC) $(LDFLAGS) $^ -o $@

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f obj/* tmp/* bin/* task_fifo log

format:
	clang-format -i src/*.c include/*.h
