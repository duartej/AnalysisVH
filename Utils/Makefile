SHELL     = /bin/sh

PACKAGE     := $(shell basename `pwd`)
GENERALPATH := $(shell cd .. && pwd)
BINPATH     := $(GENERALPATH)/bin

CXX       = g++
CXXFLAGS  = -Wall

LDFLAGS   = -shared -fPIC

SOURCES    := $(wildcard *.cc)
EXE        := $(patsubst %.cc,$(BINPATH)/%,$(SOURCES))

ROOTINCDIR ?= -I$(shell root-config --incdir)
ROOTLIBDIR ?= -L$(shell root-config --libdir)
ROOTLIBS   ?= $(shell root-config --libs)


all: $(EXE)


$(EXE): $(SOURCES)
	@echo "++++ Creating Executable ($@)"
	@$(shell mkdir -p $(BINPATH))
	@$(CXX) $(CXXFLAGS) $(ROOTINCDIR) $(ROOTLIBDIR) $(ROOTLIBS) -o $@ $(patsubst $(BINPATH)/%,%.cc,$@)


