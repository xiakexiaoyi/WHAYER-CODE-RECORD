

SHELL = /bin/sh  

CC		=		gcc

SHARED	=		-shared -g -o

FPIC	=		-fPIC -c

AR		=		ar

RM 		= 		rm -fr
 
CP		=		cp

MV		= 		mv

#定义我们需要的各种局部变量 
INCLUDES   = -I../../inc/ -I/usr/include 

CFLAGS   += -O2 $(INCLUDES)

#LFLAGS   += -L/usr/lib  -L/usr/local/lib
LFFMPEGFLAGS = -L/usr/lib
#LOPENCVFLAGS = -L/usr/local/lib

#FFMPEGLIBS   =   -lavcodec \
#-lswscale \
#-lavutil

LIBS += ../../libs/linux/libHY_IMAGEQUALITY.so -lavcodec -lswscale -lavutil -lx264 -lglib-2.0 -lgthread-2.0 /usr/lib64/libpng.so -lrt -lpthread -lm -ldl -lstdc++ /usr/lib64/libz.so \



#文件夹
LIB_DIR = ../../lib

BIN_DIR = ../../bin

OBJECT_DIR = ./obj

SOURCE_DIR = ../decoder
#


 #创建文件夹
 


SRC_OBJECT = $(SOURCE_DIR)/Decoder.cpp  \
			$(SOURCE_DIR)/Demux.cpp  $(SOURCE_DIR)/filePlayer.cpp $(SOURCE_DIR)/initDll.cpp \
			$(SOURCE_DIR)/libImageQuality.cpp $(SOURCE_DIR)/packet264.cpp

OBJECT = Decoder.o \
		Demux.o filePlayer.o initDll.o \
		 libImageQuality.o  packet264.o

 

DY_SRC_OBJECT = $(SOURCE_DIR)/testVideo.cpp

DY_OBJECT=testVideo.o

 

LIB_OBJECT = libImageQuality.so

BIN_OBJECT = testVideo

 

#****************************************************************************

 

.PHONY:all

all:init $(LIB_OBJECT) $(BIN_OBJECT) 

 

$(LIB_OBJECT):$(OBJECT)
	$(CC) $(OBJECT) $(LFFMPEGLAGS) $(FFMPEGLIBS) $(LOPENCVFLAGS) $(LIBS) $(SHARED) -fPIC -g -o $(LIB_OBJECT)
	$(MV) $(LIB_OBJECT) $(LIB_DIR)
	rm -rf /home/hadoop/$(LIB_OBJECT)
	cp   $(LIB_DIR)/$(LIB_OBJECT) /home/hadoop
 

$(OBJECT):$(SRC_OBJECT)
	$(CC) $(CFLAGS) $(FPIC) $(SRC_OBJECT)

 

$(BIN_OBJECT):$(DY_OBJECT)
	$(CC)  $(OBJECT_DIR)/$(DY_OBJECT) -L$(LIB_DIR) -lImageQuality $(LIBS) -g -o $(BIN_OBJECT)
	$(MV) $(BIN_OBJECT) $(BIN_DIR)

 

$(DY_OBJECT):$(DY_SRC_OBJECT)
	$(CC) $(CFLAGS) -c $(DY_SRC_OBJECT) 
	$(MV) $(OBJECT) $(DY_OBJECT) $(OBJECT_DIR)

 

clean:
	$(RM) $(LIB_DIR) $(BIN_DIR) $(OBJECT_DIR) $(OBJECT) $(DY_OBJECT)

init:
	$(RM) $(LIB_DIR) $(BIN_DIR) $(OBJECT_DIR)
	mkdir $(LIB_DIR) $(BIN_DIR) $(OBJECT_DIR)
