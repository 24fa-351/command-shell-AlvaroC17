CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = myshell
OBJS = myshell.o command_utils.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

myshell.o: myshell.c command_utils.h
	$(CC) $(CFLAGS) -c myshell.c

command_utils.o: command_utils.c command_utils.h
	$(CC) $(CFLAGS) -c command_utils.c

clean:
	rm -f $(TARGET) $(OBJS)
