CC = gcc
TARGET = writer reader
all:$(TARGET)
writer:
	$(CC) -o writer -DWRITER_OR_NOT main.c
reader:
	$(CC) -o reader -UWRITER_OR_NOT main.c

clean:
	rm writer reader
