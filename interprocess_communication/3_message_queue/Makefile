CC = gcc
TARGET = sender reader
all:$(TARGET)
sender:
	$(CC) -DSWITCH_PRO -o sender main.c
reader:
	$(CC) -USWITCH_PRO -o reader main.c

clean:
	rm sender reader