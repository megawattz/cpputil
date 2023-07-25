#SET BOOST_ROOT TO THE DIRECTORY THAT CONTAINS

# built in rule
# .o = $(CC) -c $(CPPFLAGS) $(CFLAGS) 
# .o = $(CXX) -c $(CPPFLAGS) $(CXXFLAGS) 
# link = $(CC) $(LDFLAGS) N.o $(LOADLIBES) $(LDLIBS) 

# but we need them with -L in front for the library link paths ...
LIB_DIRS_OPTION = $(foreach var, $(LIB_DIRS), -L$(var))

# ... and the include paths
INCLUDE_DIRS_OPTION = $(foreach var, $(INCLUDE_DIRS), -I$(var))

# and we need them to imbed an rpath in the executable
#RPATH = $(foreach var, $(LIB_DIRS), -Wl,-rpath,$(var))
RPATH = -Wl,-rpath,.

LDFLAGS += $(LIB_DIRS_OPTION) $(RPATH)

#LINK_TYPE += -static -static-libgcc -L $(DEV_ROOT)/mm_lib_cpp -L thirdparty/boost/lib -L thirdparty/libevent/lib -L thirdparty/lua/lib

CRUMBS = *.o *.d deps.inc *~

CXXFLAGS += -std=c++0x

OPT = -O0

EXPORT = -Wl,--export-dynamic

ARCH = $(shell uname -m)

REVISION = $(shell svn info | grep Revision | cut -d' ' -f2)

%.o : %.cc %.d
	$(CXX) -c $(CXXFLAGS) $(OPTIONS) $<

%.o : %.cpp %.d
	$(CXX) -c $(CXXFLAGS) $(OPTIONS) $<

#%.d : %.cc %.h
#	$(CXX) -MM -MF$@ $(INCLUDE_DIRS_OPTION) $<

#deps.inc : $(patsubst %.cc, %.d, $(shell ls *.cc))
#	cat $^ > deps.inc

#include deps.inc

