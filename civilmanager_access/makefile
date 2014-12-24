CONFIGSRC = cJSON.c cnconfig.c

TARGET = ca
SRC = main.c cnconsole.c sockets_buffer.c kfifo.c toolkit.c processappdata.c parseprotocol.c fmtreportsockdata.c cJSON.c cnconfig.c dblogin.c rbtree.c PGDatabase.cpp PGRecordset.cpp downstreammessage.c encodeprotocol.c fdfifo.c loginenterprisemanager.c loginmanager.c parseprotocol.c mqueue.c 

DBENTERPRISETARGET = dbe
DBENTERPRISESRC = $(CONFIGSRC) dblogin.c loginmanager.c fdfifo.c rbtree.c PGDatabase.cpp PGRecordset.cpp toolkit.c


DBMONITORCARDTARGET = dbmonitorcard
DBMONITORCARDSRC = $(CONFIGSRC) dbcardinfo.c cardmanager.c rbtree.c PGDatabase.cpp PGRecordset.cpp toolkit.c

TARGET = $(DBMONITORCARDTARGET)
SRC = $(DBMONITORCARDSRC)

$(TARGET):$(SRC)
	- rm $(TARGET)
	g++ -Wall -g -rdynamic  -I /usr/pgsql-9.3/include -L /usr/pgsql-9.3/lib -o $@ $^ -pthread -lpq -Wno-strict-aliasing -lzmq

qhsrvaccount_monitor : qhsrvaccount_monitor.c
	gcc -fpic -I /usr/pgsql-9.3/include/server -c qhsrvaccount_monitor.c
	gcc -shared -o qhsrvaccount_monitor.so qhsrvaccount_monitor.o
	- rm qhsrvaccount_monitor.o

qhcardinfo_monitor : qhcardinfo_monitor.c
	gcc -fpic -I /usr/pgsql-9.3/include/server -c qhcardinfo_monitor.c
	gcc -shared -o qhcardinfo_monitor.so qhcardinfo_monitor.o
	- rm qhcardinfo_monitor.o

clean:
	rm $(TARGET)
