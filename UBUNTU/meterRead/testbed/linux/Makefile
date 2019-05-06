CC =gcc
CPP =g++ -std=c++11

INCLUDE =  -I../../inc	-I../
SRCPATH	= ../

OBJPATH		= ../../int/

LFLAGS = -lm
TARGET = testBed
libpath=-L ../../libs/linux/ -ldarknet -lHYAlgorithm  \
	`pkg-config --libs opencv`


#LIBS = 

SRC1		:= $(wildcard $(SRCPATH)*.cpp)
COBJ1		:= $(patsubst $(SRCPATH)%.cpp,%.o,$(SRC1))

COBJS		:= $(patsubst %,$(OBJPATH)%,$(COBJ1)) 

$(OBJPATH)%.o: $(SRCPATH)%.cpp
	$(CPP) $(CFLAGS) $(MACROS) $(INCLUDE) $(ARCH) -c -o $@ $< 

$(TARGET): $(COBJS)
	$(CPP) -g $(INCLUDE) $(LFLAGS) -o $(TARGET) $(COBJS) $(libpath)
	
clean:
	rm -f *.o $(TARGET) 
