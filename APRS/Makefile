SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

CPPFLAGS+=-I../lib

APRS.a: $(OBJS)
	$(AR) rcs APRS.a $(OBJS)

%.o : %.cpp
	$(CC) -I../BaseCommon $(CPPFLAGS) -MMD -MD -c $< -o $@
-include $(DEPS)

clean:
	$(RM) *.o *.d APRS.a