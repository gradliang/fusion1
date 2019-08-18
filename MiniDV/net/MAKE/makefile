ROOTPATH=../../

include $(ROOTPATH)config.mak


INC = ../include/
OBJ = ../object/
SRC = ../source/

VPATH   = $(SRC)
 
TARGET = $(LIBMAIN)libnet.a

MK_USB = $(word $(words $(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h))

CC = $(BIN)gcc
#CC_OPTION += -O0 -ggdb -G 8
CC_OPTION += -O2 -G 8
CC_OPTION += -I$(INC) -I$(G_INC)
CC_OPTION += -I$(G_INC)/../libSrc/lwip/include 
ifeq ($(MK_USB),7)
CC_OPTION += -include $(LIBSRCPATH)os_linux27/include/linux/compat-2.6.h
CC_OPTION += -I$(G_INC)/../libSrc/os_linux27/include 
else
ifeq ($(MK_USB),8)
CC_OPTION += -include $(LIBSRCPATH)os_linux27/include/linux/compat-2.6.h
CC_OPTION += -I$(G_INC)/../libSrc/mac80211_36/include 
CC_OPTION += -I$(G_INC)/../libSrc/os_linux27/include 
else
CC_OPTION += -I$(G_INC)/../libSrc/os_linux/include 
endif
endif
CC_OPTION += -I$(G_INC)/../libSrc/netusb/include 
CC_OPTION += -I$(G_INC)/../libSrc/usbotg/include 
CC_OPTION += -I$(G_INC)/../libSrc/mac80211/include 
CC_OPTION += -I$(G_INC)/../libSrc/Os/include 
CC_OPTION += -D__KERNEL__
CC_OPTION += -Wa,-ahl=

ALL_OBJS =  

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

MK_LWIP = $(word $(words $(shell grep 'Make_LWIP' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_LWIP' $(ROOTPATH)include\corelib.h))
MK_NET = $(word $(words $(shell grep 'NETWARE_ENABLE' $(ROOTPATH)include\corelib.h)),$(shell grep 'NETWARE_ENABLE' $(ROOTPATH)include\corelib.h))
MK_UMODEM = $(word $(words $(shell grep 'Make_MODEM' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_MODEM' $(ROOTPATH)include\corelib.h))

S_OBJS =

C_OBJS =

ifeq ($(MK_LWIP),1)
C_OBJS +=  $(OBJ)net_init.o 
endif

ifeq ($(MK_USB),3)
C_OBJS +=  $(OBJ)dev_ar2524.o 
endif

ifeq ($(MK_USB),5)
C_OBJS +=  $(OBJ)dev_ar9271.o 
endif

ifeq ($(MK_USB),7)
C_OBJS += $(OBJ)dev_r8188c.o
endif

ifeq ($(MK_USB),8)
C_OBJS += $(OBJ)dev_r8188e.o
endif

ifeq ($(MK_UMODEM),1)
C_OBJS +=  $(OBJ)atcmd.o $(OBJ)api_lib.o $(OBJ)net_event.o 
endif

ALL_OBJS +=  $(C_OBJS) $(S_OBJS)

all: $(TARGET) makefile

$(TARGET)	: $(ALL_OBJS) makefile
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: %.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: %.c  makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(TARGET) $(OBJ)/*.lst

