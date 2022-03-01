
GPP = g++

CFLAGS+= -Wfatal-errors -g -Wall -Wformat-nonliteral -Wformat-security -I ./include -I ../util/include -I ../protobuf/dst

CPPFLAGS+= -g -std=c++11 -std=gnu++14 -Wfatal-errors -O1

LDFLAGS +=   -lstdc++ -levent -std=c++11 -std=gnu++14 -lpthread -lprotobuf

remotedict_sources = \
					 ../util/src/EventImpl.cpp \
					 ../protobuf/dst/messagebody.pb.cc \
					 src/ServiceThread.cpp \
					 src/Stats.cpp \
					 src/Dictionary.cpp \
					 src/TcpFlow.cpp \
					 src/ServiceWorker.cpp \
					 src/WorkerManager.cpp \
					 src/Listener.cpp \
					 src/TcpServer.cpp \
					 src/main.cpp
					 #src/bloom_filter.hpp \

APPS = remotedict

all: $(APPS)

remotedict: ${remotedict_sources}
	$(GPP) $(CFLAGS) -fPIC $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(GPP) $(CFLAGS)  -fPIC -c $< -o $@	

clean:
	rm -rf $(APPS)*
	rm -f *.o
