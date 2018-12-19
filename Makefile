.PHONY: all install uninstall
name = amd-disable-c6
CXX ?= g++
CXXFLAGS = -std=c++11 -O3 -Wall
cppfiles = $(name).cpp
all: $(name)

$(name):$(cppfiles)
	$(CXX) $(CXXFLAGS) $(cppfiles) -o $(name)

install: $(name)
	mkdir -p $(DESTDIR)/usr/local/sbin
	mkdir -p $(DESTDIR)/usr/lib/systemd/system
	install -m 540 $(name) $(DESTDIR)/usr/local/sbin
	install -m 644 $(name).service $(DESTDIR)/usr/lib/systemd/system

uninstall:
	rm -f $(DESTDIR)/usr/local/sbin/$(name)
	rm -f $(DESTDIR)/usr/lib/systemd/system/$(name).service

clean:
	rm -f $(name)
