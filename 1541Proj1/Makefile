# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
CFLAGS  = -g 

# the build target executable:
TARGET = pipeline 

# the source file:
SOURCE = CPU.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	$(RM) $(TARGET)
