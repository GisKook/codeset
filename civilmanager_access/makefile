CONFIGSRC = cJSON.c cnconfig.c

TARGET = test
SRC = main.c cnconsole.c sockets_buffer.c kfifo.c toolkit.c processappdata.c parseprotocol.c fmtreportsockdata.c cJSON.c cnconfig.c

DBENTERPRISETARGET = dbe
DBENTERPRISESRC = $(CONFIGSRC) dbenterprise.c enterprisemanager.c fdfifo.c rbtree.c PGDatabase.cpp PGRecordset.cpp toolkit.c

TARGET = $(DBENTERPRISETARGET)
SRC = $(DBENTERPRISESRC)

$(TARGET):$(SRC)
	- rm $(TARGET)
	g++ -Wall -g -I /usr/pgsql-9.3/include -L /usr/pgsql-9.3/lib -o $@ $^ -pthread -lpq

clean:
	rm $(TARGET)
