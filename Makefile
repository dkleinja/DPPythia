
PYTHIACFLAGS  = $(shell pythia8-config --cxxflags)
PYTHIALIBS    = $(shell pythia8-config --libs)
ROOTCFLAGS    = $(shell root-config --cflags)
ROOTLIBS      = $(shell root-config --libs)
ROOTGLIBS     = $(shell root-config --glibs)
CXXFLAGS     += $(ROOTCFLAGS) $(PYTHIACFLAGS)
LIBS          = $(ROOTLIBS)
LIBS         += $(PYTHIALIBS)
GLIBS         = $(ROOTGLIBS)

CC= g++ 
CCFLAGS = -g -fPIC -I include 
VPATH = src include 
SRC = DPPythia.C

.SUFFIXES  : .o .C

.C.o:
	$(CC) $(CCFLAGS) $(CXXFLAGS) -c  $<


DPPythia : DPPythia.o
	
	$(CC) $(CCFLAGS) $(ROOTCFLAGS) $(PYTHIAFLAGS) $(ROOTGLIBS) $(PYTHIALIBS) \
        -o $@ $^ $(ROOTGLIBS) $(PYTHIALIBS)


clean:
	rm *.o 


all: DPPythia
