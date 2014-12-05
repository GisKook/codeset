CONFIGSRC = cJSON.c cnconfig.c

TARGET = ca
SRC = main.c cnconsole.c sockets_buffer.c kfifo.c toolkit.c processappdata.c parseprotocol.c fmtreportsockdata.c cJSON.c cnconfig.c dblogin.c rbtree.c PGDatabase.cpp PGRecordset.cpp downstreammessage.c encodeprotocol.c fdfifo.c loginenterprisemanager.c loginmanager.c parseprotocol.c mqueue.c 

DBENTERPRISETARGET = dbe
DBENTERPRISESRC = $(CONFIGSRC) dblogin.c loginmanager.c fdfifo.c rbtree.c PGDatabase.cpp PGRecordset.cpp toolkit.c


$(TARGET):$(SRC)
	- rm $(TARGET)
	g++ -Wall -g -I /usr/pgsql-9.3/include -L /usr/pgsql-9.3/lib -o $@ $^ -pthread -lpq -Wno-strict-aliasing

clean:
	rm $(TARGET)
