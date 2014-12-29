CONFIGSRC = cJSON.c cnconfig.c
PBSRC = ./pb/beidoumessage.pb.cc ./pb/smsTx.pb.cc ./pb/bcTx.pb.cc

TARGET = ca
SRC = cardmanager.c cJSON.c cnconfig.c cnconsole.c dbcardinfo.c dblogin.c downstreammessage.c encodeprotocol.c encodeprotocolupstream.c enterprisemanager.c fdfifo.c fmtreportsockdata.c fmtresponddata.c kfifo.c loginenterprisemanager.c loginmanager.c main.c mqueue.c parseprotocol.c PGDatabase.cpp PGRecordset.cpp processappdata.c rbtree.c sockets_buffer.c toolkit.c zmq_buffer.c $(PBSRC)

DBENTERPRISETARGET = dbe
DBENTERPRISESRC = $(CONFIGSRC) dblogin.c loginmanager.c fdfifo.c rbtree.c PGDatabase.cpp PGRecordset.cpp toolkit.c


DBMONITORCARDTARGET = dbmonitorcard
DBMONITORCARDSRC = $(CONFIGSRC) dbcardinfo.c cardmanager.c rbtree.c PGDatabase.cpp PGRecordset.cpp toolkit.c


$(TARGET):$(SRC)
	- rm $(TARGET)
	g++ -Wall -g -rdynamic -I /usr/pgsql-9.3/include -L /usr/pgsql-9.3/lib -o $@ $^ -pthread -lpq -Wno-strict-aliasing -lzmq -lprotobuf

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
