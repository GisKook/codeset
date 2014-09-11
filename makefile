TARGET = test
SRC = main.c cnconsole.c sockets_buffer.c kfifo.c toolkit.c processappdata.c parseprotocol.c fmtreportsockdata.c
$(TARGET):$(SRC)
	g++ -Wall -rpath -g -o $@ $^ -pthread

clean:
	rm $(TARGET)
