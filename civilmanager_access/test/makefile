TARGET = test_zmq
SRC = test_zmq.c bcTx.pb.cc
$(TARGET):
	g++ -g -o test_zmq ./test_zmq.c ./pb/bcTx.pb.cc -lzmq -lprotobuf -pthread

pull:
	g++ -g -o test_zmq_pull ./test_zmq_pull.c ./pb/bcTx.pb.cc -lzmq -lprotobuf -pthread

