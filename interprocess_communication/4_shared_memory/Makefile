CC = gcc
TARGET = reader writer
all:$(TARGET)
reader:
	$(CC) -DREADER_OR_WRITER -o reader main.c
writer:
	$(CC) -UREADER_OR_WRITER -o writer main.c

clean:
	rm reader writer
