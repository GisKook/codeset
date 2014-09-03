TARGET = test
SRC = main.c cnconsole.c sockets_buffer.c kfifo.c toolkit.c
$(TARGET):$(SRC)
	gcc -g -o $@ $^

clean:
	rm $(TARGET)
