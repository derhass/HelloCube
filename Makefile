# Makefile for unix systems
# this requires GNU make

APPNAME=HelloCube

# Compiler flags
# enable all warnings in general
WARNFLAGS= -Wall -Wno-comment

SHAREDFLAGS = $(WARNFLAGS) -pthread

ifeq ($(RELEASE), 1)
CFLAGS =   $(SHAREDFLAGS) -ffast-math -s -O4 -DNDEBUG
CXXFLAGS = $(SHAREDFLAGS) -ffast-math -s -O4 -DNDEBUG
else
CFLAGS =   $(SHAREDFLAGS) -g
CXXFLAGS = $(SHAREDFLAGS) -g
endif

# include paths for the builtin libraries glad and glm. We do not link them,
# as we directly incorporated the source code into our project
CPPFLAGS += -I glad/include -I glm/

# Try to find the system's GLFW3 library via pkg-config
CPPFLAGS += $(shell pkg-config --cflags glfw3)
LDFLAGS += $(shell pkg-config --static --libs glfw3) 

# additional libraries
LDFLAGS += -lrt -lm

CFILES=$(wildcard *.c) glad/src/gl.c
CPPFILES=$(wildcard *.cpp)
INCFILES=$(wildcard *.h)	
SRCFILES = $(CFILES) $(CPPFILES)
PRJFILES = Makefile $(wildcard *.vcxproj) $(wildcard *.sln)
ALLFILES = $(SRCFILES) $(INCFILES) $(PRJFILES)
OBJECTS = $(patsubst %.cpp,%.o,$(CPPFILES)) $(patsubst %.c,%.o,$(CFILES))
	   
# build rules
.PHONY: all
all:	$(APPNAME)

# build and start with "make run"
.PHONY: run
run:	all
	./$(APPNAME)

# automatic dependency generation
# create $(DEPDIR) (and an empty file dir)
# create a .d file for every .c source file which contains
# 		   all dependencies for that file
.PHONY: depend
depend:	$(DEPDIR)/dependencies
DEPDIR   = ./dep
DEPFILES = $(patsubst %.c,$(DEPDIR)/%.d,$(CFILES)) $(patsubst %.cpp,$(DEPDIR)/%.d,$(CPPFILES))
$(DEPDIR)/dependencies: $(DEPDIR)/dir $(DEPFILES)
	@cat $(DEPFILES) > $(DEPDIR)/dependencies
$(DEPDIR)/dir:
	@mkdir -p $(DEPDIR)
	@mkdir -p $(DEPDIR)/glad/src
	@touch $(DEPDIR)/dir
$(DEPDIR)/%.d: %.c $(DEPDIR)/dir
	@echo rebuilding dependencies for $*
	@set -e; $(CC) -M $(CPPFLAGS) $<	\
	| sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' \
	> $@; [ -s $@ ] || rm -f $@
$(DEPDIR)/%.d: %.cpp $(DEPDIR)/dir
	@echo rebuilding dependencies for $*
	@set -e; $(CXX) -M $(CPPFLAGS) $<	\
	| sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' \
	> $@; [ -s $@ ] || rm -f $@
-include $(DEPDIR)/dependencies

# rule to build application
$(APPNAME): $(OBJECTS) $(DEPDIR)/dependencies
	$(CXX) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o$(APPNAME)

# remove all unneeded files
.PHONY: clean
clean:
	@echo removing binary: $(APPNAME)
	@rm -f $(APPNAME)
	@echo removing object files: $(OBJECTS)
	@rm -f $(OBJECTS)
	@echo removing dependency files
	@rm -rf $(DEPDIR)
	@echo removing tags
	@rm -f tags

# update the tags file
.PHONY: TAGS
TAGS:	tags

tags:	$(SRCFILES) $(INCFILES) 
	ctags $(SRCFILES) $(INCFILES)

# look for 'TODO' in all relevant files
.PHONY: todo
todo:
	-egrep -in "TODO" $(SRCFILES) $(INCFILES)

