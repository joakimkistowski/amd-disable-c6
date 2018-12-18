.PHONY: all install uninstall
name = amd-disable-c6
CXX ?= g++
CXXFLAGS = -std=c++11 -O3 -Wall
cppfiles = $(name).cpp
all: $(name)

$(name):$(cppfiles)
	$(CXX) $(CXXFLAGS) $(cppfiles) -o $(name)

install: $(name)
	install -m 540 $(name) /usr/local/sbin
	install -m 644 $(name).service /usr/lib/systemd/system

uninstall:
	rm -f /usr/local/sbin/$(name)
	rm -f /usr/lib/systemd/system/$(name).service

clean:
	rm -f $(name)
