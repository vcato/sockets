CXXFLAGS=-W -Wall -pedantic -std=c++14 -MD -MP

main: main.o server.o client.o socket.o internetaddress.o
	$(CXX) -o $@ $^

-include *.d
