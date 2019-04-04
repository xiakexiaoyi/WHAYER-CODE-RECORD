include config_c10

CFLAGS = -lm
OPTS=-Ofast
CFLAGS+=-Wall -Wfatal-errors -fPIC -fvisibility=hidden

CFLAGS+=$(OPTS)
MACROS	 = $(PLAT_MACROS) $(OTHER_MACROS)
MACROS	:= $(patsubst %,-D%,$(MACROS))


LIBNAME		= HYAlgorithm

INCLUDE	 = $(SYSINC)		\
	    -I../../inc					\
            -I../../src/image				\
            -I../../src/imagepro			\
            -I../../src/imgfmt_trans			\
            -I../../src/licommon			\
            -I../../src/Match			\
	    -I../../src/gradient				\
	    -I../../src/mask				\
	    -I../../src/facedespot			\
	    -I../../src/	                        \
            -I../../src/hisi                            \
            -I../../src/colorseg                           \
            -I../../src/filter                      \
            -I../../src/RidMatch_ydq                      \
            
			
SRCPATH1	= ../../src/facedespot/
SRCPATH2	= ../../src/
SRCPATH3	= ../../src/mask/
SRCPATH4	= ../../src/image/
SRCPATH5	= ../../src/imgfmt_trans/
SRCPATH6	= ../../src/licommon/
SRCPATH7	= ../../src/imagepro/
SRCPATH8	= ../../src/hisi/
SRCPATH9	= ../../src/Match/
SRCPATH10	= ../../src/colorseg/
SRCPATH11	= ../../src/gradient/
SRCPATH12	= ../../src/filter/
SRCPATH13	= ../../src/RidMatch_ydq/




SRC1		:= $(wildcard $(SRCPATH1)*.c)
SRC2		:= $(wildcard $(SRCPATH2)*.c)
SRC3		:= $(wildcard $(SRCPATH3)*.c)
SRC4		:= $(wildcard $(SRCPATH4)*.c)
SRC5		:= $(wildcard $(SRCPATH5)*.c)
SRC6		:= $(wildcard $(SRCPATH6)*.c)
SRC7		:= $(wildcard $(SRCPATH7)*.c)
SRC8		:= $(wildcard $(SRCPATH8)*.c)
SRC9		:= $(wildcard $(SRCPATH9)*.c)
SRC10		:= $(wildcard $(SRCPATH10)*.c)
SRC11		:= $(wildcard $(SRCPATH11)*.c)
SRC12		:= $(wildcard $(SRCPATH12)*.c)
SRC13		:= $(wildcard $(SRCPATH13)*.c)

COBJ1		:= $(patsubst $(SRCPATH1)%.c,%.o,$(SRC1))
COBJ2		:= $(patsubst $(SRCPATH2)%.c,%.o,$(SRC2))
COBJ3		:= $(patsubst $(SRCPATH3)%.c,%.o,$(SRC3))
COBJ4		:= $(patsubst $(SRCPATH4)%.c,%.o,$(SRC4))
COBJ5		:= $(patsubst $(SRCPATH5)%.c,%.o,$(SRC5))
COBJ6		:= $(patsubst $(SRCPATH6)%.c,%.o,$(SRC6))
COBJ7		:= $(patsubst $(SRCPATH7)%.c,%.o,$(SRC7))
COBJ8		:= $(patsubst $(SRCPATH8)%.c,%.o,$(SRC8))
COBJ9		:= $(patsubst $(SRCPATH9)%.c,%.o,$(SRC9))
COBJ10		:= $(patsubst $(SRCPATH10)%.c,%.o,$(SRC10))
COBJ11		:= $(patsubst $(SRCPATH11)%.c,%.o,$(SRC11))
COBJ12		:= $(patsubst $(SRCPATH12)%.c,%.o,$(SRC12))
COBJ13		:= $(patsubst $(SRCPATH13)%.c,%.o,$(SRC13))

COBJS		:= $(patsubst %,$(OBJPATH)%,$(COBJ1) $(COBJ2) $(COBJ3) $(COBJ4) $(COBJ5) $(COBJ6) $(COBJ7) $(COBJ8) $(COBJ9) $(COBJ10) $(COBJ11) $(COBJ12) $(COBJ13)) 


dll: $(COBJS)
	mkdir -p $(LIBDIR)
	$(CC) $(CFLAGS) -shared $^ -o $(LIBDIR)lib$(LIBNAME).so $(LIBS)


$(OBJPATH)%.o: $(SRCPATH1)%.c
	$(CC) $(CFLAGS) $(MACROS) $(INCLUDE) $(ARCH) -c -o $@ $<   
$(OBJPATH)%.o: $(SRCPATH2)%.c
	$(CC) $(CFLAGS) $(MACROS) $(INCLUDE) $(ARCH) -c -o $@ $<   
$(OBJPATH)%.o: $(SRCPATH3)%.c
	$(CC) $(CFLAGS) $(MACROS) $(INCLUDE) $(ARCH) -c -o $@ $< 
$(OBJPATH)%.o: $(SRCPATH4)%.c
	$(CC) $(CFLAGS) $(MACROS) $(INCLUDE) $(ARCH) -c -o $@ $< 
$(OBJPATH)%.o: $(SRCPATH5)%.c
	$(CC) $(CFLAGS) $(MACROS) $(INCLUDE) $(ARCH) -c -o $@ $< 
$(OBJPATH)%.o: $(SRCPATH6)%.c
	$(CC) $(CFLAGS) $(MACROS) $(INCLUDE) $(ARCH) -c -o $@ $< 
$(OBJPATH)%.o: $(SRCPATH7)%.c
	$(CC) $(CFLAGS) $(MACROS) $(INCLUDE) $(ARCH) -c -o $@ $< 
$(OBJPATH)%.o: $(SRCPATH8)%.c
	$(CC) $(CFLAGS) $(MACROS) $(INCLUDE) $(ARCH) -c -o $@ $< 
$(OBJPATH)%.o: $(SRCPATH9)%.c
	$(CC) $(CFLAGS) $(MACROS) $(INCLUDE) $(ARCH) -c -o $@ $< 
$(OBJPATH)%.o: $(SRCPATH10)%.c
	$(CC) $(CFLAGS) $(MACROS) $(INCLUDE) $(ARCH) -c -o $@ $< 
$(OBJPATH)%.o: $(SRCPATH11)%.c
	$(CC) $(CFLAGS) $(MACROS) $(INCLUDE) $(ARCH) -c -o $@ $<
$(OBJPATH)%.o: $(SRCPATH12)%.c
	$(CC) $(CFLAGS) $(MACROS) $(INCLUDE) $(ARCH) -c -o $@ $< 
$(OBJPATH)%.o: $(SRCPATH13)%.c
	$(CC) $(CFLAGS) $(MACROS) $(INCLUDE) $(ARCH) -c -o $@ $< 	
clean:
	-rm $(COBJS) $(STATICOUT) -f
