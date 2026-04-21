SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

CPPFLAGS+=-I../lib

BaseCommon.a: $(OBJS)
	$(AR) rcs BaseCommon.a $(OBJS)

%.o : %.cpp
	$(CXX) $(CPPFLAGS) -MMD -MD -c $< -o $@
-include $(DEPS)

clean:
	$(RM) *.o *.d BaseCommon.a