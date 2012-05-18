SOURCES = main.cpp init.cpp backup.cpp restore.cpp clean.cpp Bz2File.cpp \
          mthread.cpp HashId.cpp HashSet.cpp path.cpp GitbkFs.cpp IGbStream.cpp \
          NodeAttr.cpp SubCmd.cpp

include Make_cfg

all: gitbk

pch.h.gch : pch.h
	g++-4.7 $(CXXFLAGS) $<

gitbk: pch.h.gch $(OBJECTS)
	@echo "[link] $@"
	g++-4.7 -g $(OBJECTS) $(STDSRC) -o $@ -lbz2 -lpthread -lboost_thread-mt -lboost_system-mt -lboost_filesystem-mt -lboost_iostreams-mt -lboost_date_time-mt -lssl

clean:
	rm -rf *.o *.S .dps gitbk pch.h.gch

