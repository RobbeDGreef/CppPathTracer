CXX=g++
INCLUDES=./include
SOURCES = $(shell find ./src -name "*.cpp")

CPPFLAGS = -O3 -fno-omit-frame-pointer -lstdc++ -g -pthread -fsanitize=address -lasan 

all: linux

windows: $(SOURCES)
	$(CXX) $(CPPFLAGS) -I $(INCLUDES) $^ -o raytracer.exe

linux: $(SOURCES)
	$(CXX) $(CPPFLAGS) -I $(INCLUDES) $^ -o raytracer

clean:
	rm raytracer.exe