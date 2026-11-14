CC = clang
CFLAGS = -Wall -Wextra -std=c11 -O2

TARGET = program.exe
SRC = src/main.c

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)