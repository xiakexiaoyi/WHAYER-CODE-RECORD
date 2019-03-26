CC =gcc
CPP =g++ -std=c++11

INCLUDE =  -I../../inc	-I../
SRCPATH	= ../

OBJPATH		= ../../int/

LFLAGS = -lm
TARGET = testBed

libpath=-L ../../libs/linux/ -ldarknet
libpath+= ../../libs/linux/MatchRigidBody.a   \
	`pkg-config --libs opencv`


#LIBS = 

SRC1		:= $(wildcard $(SRCPATH)*.cpp)
SRC2		:= $(wildcard $(SRCPATH)*.c)
COBJ1		:= $(patsubst $(SRCPATH)%.cpp,%.o,$(SRC1))
COBJ2		:= $(patsubst $(SRCPATH)%.c,%.o,$(SRC2))

COBJS		:= $(patsubst %,$(OBJPATH)%,$(COBJ1) $(COBJ2)) 

$(OBJPATH)%.o: $(SRCPATH)%.cpp
	$(CPP) $(CFLAGS) $(MACROS) $(INCLUDE) $(ARCH) -c -o $@ $< 
$(OBJPATH)%.o: $(SRCPATH)%.c
	$(CC) $(CFLAGS) $(MACROS) $(INCLUDE) $(ARCH) -c -o $@ $<  

$(TARGET): $(COBJS)
	$(CPP) -g $(INCLUDE) $(LFLAGS) -o $(TARGET) $(COBJS) $(libpath)
	
clean:
	rm -f $(OBJPATH)*.o $(TARGET) 
