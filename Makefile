#SET BOOST_ROOT TO THE DIRECTORY THAT CONTAINS

THIRDPARTY=thirdparty

CXXFLAGS +=$(OPTIONS) -Wno-deprecated $(EXPORT) -fPIC -shared -I. -I$(THIRDPARTY)/boost/include -I$(THIRDPARTY)/libevent/include -I$(THIRDPARTY)/yajl/include 
CXX := g++

CRUMBS = *.o *.d deps.inc *~

CXXFLAGS += -std=c++11

EXPORT = -Wl,--export-dynamic

ARCH = $(shell uname -m)

.PHONY: version links

all: version links libmm.a

%.o : %.cc %.d
	$(CXX) -c $(CXXFLAGS) $(OPTIONS) $<

%.o : %.cpp %.d
	$(CXX) -c $(CXXFLAGS) $(OPTIONS) $<

version:
	./verstamp.sh -i LibVersion.h
	./verstamp.sh -i version.txt

fast:	version
	CXXFLAGS="$(CXXFLAGS) -O3" make

debug:  version
	CXXFLAGS="$(CXXFLAGS) -O0 -g" make

# get ALL the cc files
CFILES = $(shell ls *.cc)

# turn the list of .cc files into a list of .o files
OBJS = $(patsubst %.cc, %.o, $(CFILES))

libmm.a: $(OBJS) Makefile
	ar rcs libmm.a $(filter %.o, $^)

clean:
	@-rm $(CRUMBS)
	@-rm libmm.so
	@-rm *.o
	@-rm *.a

include rules_cpp.mk

##
## $Id: Makefile 2591 2012-11-06 22:18:02Z whoward $
##
