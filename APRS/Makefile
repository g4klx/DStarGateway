SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

APRS.a: $(OBJS)
	$(AR) rcs APRS.a $(OBJS)

%.o : %.cpp
	$(CXX) -I../BaseCommon $(CPPFLAGS) -MMD -MD -c $< -o $@
-include $(DEPS)

clean:
	$(RM) *.o *.d APRS.a