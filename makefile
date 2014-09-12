TARGET = test
SRC = main.c cnconsole.c sockets_buffer.c kfifo.c toolkit.c processappdata.c parseprotocol.c fmtreportsockdata.c cJSON.c cnconfig.c
$(TARGET):$(SRC)
	g++ -Wall -g -o $@ $^ -pthread

clean:
	rm $(TARGET)
