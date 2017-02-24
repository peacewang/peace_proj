include $(PEACE_BASE_PATH)/learn/makefile.learn.comm

INCLUDE = -I$(INC_BASE_PATH) -I$(INC_LEARN_PATH)
		   
LIB = -L$(LIB_LEARN_PATH) \
		-lpth \
	  -L$(LIB_PEACE_PATH) \

OUTPUT = test_pth 

OBJS = test_pth.o 

SOURCES=`ls *.cpp`

.cpp.o:
	$(CXX) $(CPPFLAGS) $(INCLUDE) -c $(filter %.cpp, $^) 

.o:
	$(CXX) $(CPPFLAGS) -o $@ $^

all:$(OUTPUT)

$(OUTPUT): $(OBJS)
	$(CXX) $(CPPFLAGS) -o $@ $^ ${LIB}

clean:
	rm -f *.o *.~ *.bak
	rm -f $(OUTPUT)

dep:
	mkdep $(INCLUDE) $(SOURCES) > .depend

ifneq ($(wildcard .depend),)
include .depend
endif

