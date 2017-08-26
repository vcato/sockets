CXXFLAGS=-W -Wall -pedantic -std=c++14 -MD -MP

# To build with mingw for Windows
#CXX=i686-w64-mingw32-c++-win32
#LIBS=-lws2_32

main: main.o server.o client.o socket.o internetaddress.o
	$(CXX) -static-libgcc -static-libstdc++ -o $@ $^ $(LIBS)

clean:
	rm -f *.o *.d

-include *.d
