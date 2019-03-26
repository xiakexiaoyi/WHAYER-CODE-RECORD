CC =gcc

INCLUDE =  -I../../inc	-I../
SRCPATH	= ../

OBJPATH		= ../../int/

LFLAGS = -lm
TARGET = testBed
libpath= ../../libs/linux/HY_IMAGEQUALITY.a
#LIBS = 

SRC1		:= $(wildcard $(SRCPATH)*.c)
COBJ1		:= $(patsubst $(SRCPATH)%.c,%.o,$(SRC1))

COBJS		:= $(patsubst %,$(OBJPATH)%,$(COBJ1)) 

$(OBJPATH)%.o: $(SRCPATH)%.c
	$(CC) $(CFLAGS) $(MACROS) $(INCLUDE) $(ARCH) -c -o $@ $< 

$(TARGET): $(COBJS)
	$(CC) -g $(INCLUDE) $(LFLAGS) -o $(TARGET) $(COBJS) $(libpath)
	
clean:
	rm -f *.o $(TARGET) 
