CXXFLAGS=-g3 -O0 -std=c++17 -pedantic -Wall
APP=aifscan
all: aifscan

$(APP): main.cc chunk.cc
	$(CXX) $(CXXFLAGS) $^ -o $@

.PHONY: clean
clean:
	$(RM) $(APP)
