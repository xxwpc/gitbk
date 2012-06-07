SOURCES = main.cpp init.cpp backup.cpp restore.cpp clean.cpp GbFile.cpp \
          mthread.cpp HashId.cpp HashSet.cpp path.cpp GitbkFs.cpp \
          NodeAttr.cpp SubCmd.cpp

all: gitbk


CXX ?= g++
CXXFLAGS = -Wall -g -std=c++11
#CXXFLAGS = -Wall -O4 -std=c++0x -fno-exceptions -Dnullptr=0 -DNDEBUG

OBJECTS = $(foreach src, $(SOURCES), $(basename $(src)).o )

DEPENDS = $(foreach src, $(SOURCES), $(basename $(src)).dps )

pch.h.gch : pch.h
	$(CXX) $(CXXFLAGS) $<

$(OBJECTS): .dps pch.h.gch

gitbk: $(OBJECTS)
	@echo "[link] $@"
	$(CXX) -g $(OBJECTS) $(STDSRC) -o $@ -lbz2 -lpthread -lboost_thread-mt -lboost_system-mt -lboost_filesystem-mt -lboost_iostreams-mt -lboost_date_time-mt -lssl

clean:
	rm -rf *.o *.S .dps gitbk pch.h.gch

-include .dps/*

.cpp.o :
	@echo "[compile] $<"
	$(CXX) $(CXXFLAGS) -MMD -MF .dps/$<.dps -c $< -o $@
	@echo

.dps :
	mkdir -p .dps
	@echo
