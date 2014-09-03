TARGET = test
SRC = main.c cnconsole.c sockets_buffer.c kfifo.c
$(TARGET):$(SRC)
	gcc -g -Wall -o $@ $^

clean:
	rm $(TARGET)
