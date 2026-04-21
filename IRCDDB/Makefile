SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

CPPFLAGS+=-I../lib

IRCDDB.a: $(OBJS) ../VersionInfo/GitVersion.h ../BaseCommon/BaseCommon.a
	$(AR) rcs IRCDDB.a $(OBJS)

%.o : %.cpp
	$(CXX) -I../BaseCommon -I../VersionInfo $(CPPFLAGS) -MMD -MD -c $< -o $@
-include $(DEPS)

.PHONY clean:
clean:
	$(RM) *.o *.d IRCDDB.a

../BaseCommon/BaseCommon.a:
../VersionInfo/GitVersion.h:
