SHELL = /bin/sh

PACKAGE   := $(shell basename `pwd`)


### Dependencies
DEPPKG     := 
BASEDIR    := $(shell cd .. && pwd && cd ..)
DEPSDIR    := $(foreach DEPPKG,$(DEPPKG),$(BASEDIR)/$(DEPPKG))
DEPSLIBDIR := $(foreach DEPSDIR,$(DEPSDIR),-L$(DEPSDIR)/lib)
DEPSSEARCHLIBDIRS := $(foreach DEPSDIR,$(DEPSDIR),$(DEPSDIR)/lib)

### Macro to extract the path to the dependency libraries
getpkgbasedir  = $(foreach var,$(1), $(filter %/$(var),$(DEPSDIR))) 

.PHONY: all cleanall
 
all: 
	@# Checking if it is necessary to compile the dependency libraries
	@echo "======= $(PACKAGE): Compiling packages ======"
	@for libdir in $(call getpkgbasedir,$(subst -l,,$@)); do \
		$(MAKE) -C $$libdir; \
	done

cleanall: 
	@for libdir in $(DEPSDIR); do \
		$(MAKE) -C $$libdir clean; \
	done


	
