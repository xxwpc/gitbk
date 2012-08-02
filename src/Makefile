SOURCES = $(wildcard *.cpp)

all: gitbk


CXX ?= g++
CXXFLAGS = -Wall -g -std=c++11
#CXXFLAGS = -Wall -O4 -std=c++0x -fno-exceptions -Dnullptr=0 -DNDEBUG

OBJECTS = $(foreach src, $(SOURCES), $(basename $(src)).o )

DEPENDS = $(foreach src, $(SOURCES), $(basename $(src)).dps )

pch.h.gch : pch.h
	$(CXX) $(CXXFLAGS) $<

version.h : FORCE
	@./gen-version.sh

version.o : version.h

$(OBJECTS): pch.h.gch |.dps

gitbk: $(OBJECTS)
	@echo "[link] $@"
	$(CXX) -g $(OBJECTS) $(STDSRC) -o $@ -lbz2 -lpthread -lboost_thread-mt -lboost_system-mt -lboost_filesystem-mt -lboost_iostreams-mt -lboost_date_time-mt -lboost_regex-mt -lssl

clean:
	rm -rfv *.o *.S .dps gitbk pch.h.gch version.h

-include .dps/*

.cpp.o :
	@echo "[compile $<] $(CXX) $(CXXFLAGS) -MMD -MF .dps/$<.dps -c $< -o $@"
	@$(CXX) $(CXXFLAGS) -MMD -MF .dps/$<.dps -c $< -o $@

.dps :
	mkdir -p .dps


.PHONY: FORCE
