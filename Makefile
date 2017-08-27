CXXFLAGS=-W -Wall -pedantic -std=c++14 -MD -MP -g

# To build with mingw for Windows
#CXX=i686-w64-mingw32-c++-win32
#LIBS=-lws2_32

LINK=$(CXX) -static-libgcc  -static-libstdc++

all: run_unit_tests connection_speed_test messaging_test

run_unit_tests: messagebuilder_test.pass objectset_test.pass

%.pass: %
	./$*
	touch $@

messagebuilder_test: messagebuilder_test.o messagebuilder.o
	$(LINK) -o $@ $^ $(LIBS)

objectset_test: objectset_test.o
	$(LINK) -o $@ $^ $(LIBS)

connection_speed_test: \
  connectionspeedtestmain.o server.o socket.o internetaddress.o
	$(LINK) -o $@ $^ $(LIBS)

messaging_test: messagingtestmain.o \
  socket.o internetaddress.o messagingserver.o messagebuilder.o \
  feedmessagebuilder.o messagingclient.o
	$(LINK) -o $@ $^ $(LIBS)

clean:
	rm -f *.o *.d

-include *.d
