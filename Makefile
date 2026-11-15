CC = clang

CFLAGS = -Wall -Wextra -std=c11 -O2

LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm

TARGET = program.exe
SRC = src/main.c

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS) && ./$(TARGET)

clean:
	rm -f $(TARGET)