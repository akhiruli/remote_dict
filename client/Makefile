
GPP = g++

CFLAGS+= -Wfatal-errors -g -Wall -Wformat-nonliteral -Wformat-security -I ./include -I ../protobuf/dst -I ../util/include

CPPFLAGS+= -g -std=c++11 -std=gnu++14 -Wfatal-errors -O1

LDFLAGS +=  -lstdc++ -levent -std=c++11 -std=gnu++14 -lpthread -lprotobuf

msgclient_sources = \
					../protobuf/dst/messagebody.pb.cc \
					src/Client.cpp  \
					src/TestClient.cpp

APPS = msgclient

all: $(APPS)

msgclient: ${msgclient_sources}
	$(GPP) $(CFLAGS) -fPIC $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(GPP) $(CFLAGS)  -fPIC -c $< -o $@	

clean:
	rm -f $(APPS)
	rm -f *.o
