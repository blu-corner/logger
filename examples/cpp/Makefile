INSTALL_PREFIX=../../../
CXX=g++

CFLAGS=-I${INSTALL_PREFIX}/include/logger -I$(INSTALL_PREFIX)/include/properties/ -I${INSTALL_PREFIX}/include/utils -I${INSTALL_PREFIX}/include/sbf
LDFLAGS=-L$(INSTALL_PREFIX)/lib 
LIBS=-llogger -lproperties -lsbfcore -lsbfcommon -lutils

all: example customHandlerExample

example: example.o
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

example.o: example.cpp
	$(CXX) -c $(CFLAGS) $<

customHandlerExample: customHandlerExample.o
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

customHandlerExample.o: customHandlerExample.cpp
	$(CXX) -c $(CFLAGS) $<

clean:
	rm *.o example customHandlerExample
