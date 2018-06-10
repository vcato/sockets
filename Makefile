CXXFLAGS=-W -Wall -pedantic -std=c++14 -MD -MP -g

# To build with mingw for Windows
#CXX=i686-w64-mingw32-c++-win32
#LIBS=-lws2_32

LINK=$(CXX) -static-libgcc  -static-libstdc++

all: run_unit_tests build_manual_tests

run_unit_tests: \
  messagebuilder_test.pass \
  objectset_test.pass

build_manual_tests: \
  connectionspeed_manualtest \
  messaging_manualtest \
  nodelay_manualtest \
  nonblocking_manualtest

%.pass: %
	./$*
	touch $@

messagebuilder_test: messagebuilder_test.o messagebuilder.o
	$(LINK) -o $@ $^ $(LIBS)

objectset_test: objectset_test.o
	$(LINK) -o $@ $^ $(LIBS)

connectionspeed_manualtest: \
  connectionspeedtestmain.o socket.o internetaddress.o
	$(LINK) -o $@ $^ $(LIBS)

messaging_manualtest: messagingtestmain.o \
  socket.o internetaddress.o messagingserver.o messagebuilder.o \
  feedmessagebuilder.o messagingclient.o
	$(LINK) -o $@ $^ $(LIBS)

nodelay_manualtest: nodelaytestmain.o socket.o internetaddress.o
	$(LINK) -o $@ $^ $(LIBS)

nonblocking_manualtest: nonblockingtestmain.o socket.o internetaddress.o
	$(LINK) -o $@ $^ $(LIBS)

clean:
	rm -f *.o *.d

-include *.d
