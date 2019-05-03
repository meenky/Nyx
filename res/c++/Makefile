CXXFLAGS := -std=c++14 -I. -g


OBJS := driver.o \
        nyx/crc.o \
	      nyx/runtime.o \
        nyx/example/image.o \
				nyx/example/protobuf.o

all: driver

%.o: %.cpp
	$(COMPILE.cpp) -o $@ $^

driver: $(OBJS)
	$(LINK.cpp) -o $@ $(OBJS)

