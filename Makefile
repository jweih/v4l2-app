#####  #
#####  # Makefile for the CAMERA Application.
#####  #
#####  
#####  CC			=	/opt/armv6/codesourcery/bin/arm-none-linux-gnueabi-gcc
#####  
#####  CFLAGS		=	-g -Wall -O0 -I../../kernel/linux-2.6.29-tcc/include/ -I../include -I./ -I../include/ipc
#####  
#####  SRCS 		= 	v4l2.c main.c rsc.c
#####  
#####  LIBS		=	-lpthread
#####  
#####  OBJS = $(SRCS:.c=.o)
#####  
#####  EXECUTABLE	= camapp
#####  
#####  all:
#####  	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(SRCS) $(LIBS)
#####  
#####  clean:
#####  	rm -f $(OBJS) camapp

	
####################################################################################
#  makefile
####################################################################################

ROOT_PATH =.


ARCH=arm
ifeq ($(ARCH), arm)
CROSS_COMPILE=arm-none-linux-gnueabi-

AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
CC = $(CROSS_COMPILE)gcc
CPP = $(CC) -E
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar
NM = $(CROSS_COMPILE)nm
STRIP = $(CROSS_COMPILE)strip
OBJDUMP = $(CROSS_COMPILE)objdump
else
COMPILE_OPTION += -g
endif

ifndef $(KDIR)
KDIR := ../../kernel
endif

REL_PATH = $(ROOT_PATH)
SRC_PATH =.

TARGET=$(REL_PATH)/camapp

COMPILE_OPTION	+= -c -O0  -Wall
#COMPILE_OPTION	+= -c -O2  -Wall -Werror -fomit-frame-pointer -pipe
DEFINE		+= -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_REENTRANT 

INC_PATH	+= -I$(REL_PATH)/inc
INC_PATH	+= -I$(ROOT_PATH)/include
INC_PATH	+= -I$(KDIR)/include
INC_PATH	+= -I$(KDIR)/arch/arm/plat-tcc/include
INC_PATH	+= -I$(KDIR)/arch/arm/mach-tcc893x/include
INC_PATH	+= -I$(KDIR)/arch/arm/mach-tcc893x/include/mach

C_OBJ		=  $(SRC_PATH)/v4l2.o\
			   $(SRC_PATH)/main.o\
			   $(SRC_PATH)/pmap.o\
			   $(SRC_PATH)/tcc_scaler_interface.o\
			   $(SRC_PATH)/rsc.o

C_SRCS		=  $(C_OBJ:.o=.c)

CFLAGS		+= $(COMPILE_OPTION) $(INC_PATH) $(DEFINE)
CPPFLAGS	+= $(COMPILE_OPTION) $(INC_PATH) $(DEFINE)
LDFLAGS		= -s

POST_EXE	=

J_LD		=  $(CC)
LINK_LIB        += -lpthread

all: $(TARGET)
	$(POST_EXE)
	@echo ""
	@echo "   ########## [$^] ##########"
	@echo ""

clean:
	$(RM) $(C_OBJ) $(CPP_OBJ) $(TARGET)

$(TARGET) : $(C_OBJ) $(CPP_OBJ)
	$(J_LD) $(LDFLAGS) -o $@ $^ $(LINK_LIB)

.c.o: $(C_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

