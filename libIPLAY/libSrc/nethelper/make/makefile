include ../../../config.mak
 
ROOTPATH=../../../
 
TARGET = $(LIB)libnethelper.a

MK_USB = $(word $(words $(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h))

CC = $(BIN)gcc
CC_OPTION += -O2 -G 8
#CC_OPTION += -O0 -ggdb
CC_OPTION += -I$(INC) -I$(G_INC) 
CC_OPTION += -I../../../libSrc/usbotg/include
ifeq ($(MK_USB),5)
CC_OPTION += -include $(ROOTPATH)libSrc/os_linux27/include/linux/compat-2.6.h
CC_OPTION += -I../../../libSrc/ath9k/include 
CC_OPTION += -I../../../libSrc/os_linux27/include 
else
CC_OPTION += -I../../../libSrc/os_linux/include 
endif
CC_OPTION +=  -D__KERNEL__
CC_OPTION += -DCONFIG_NO_MALLOC
CC_OPTION += -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

MK_SMTP = $(word $(words $(shell grep 'Make_SMTP' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_SMTP' $(ROOTPATH)include\corelib.h))
MK_UMODEM = $(word $(words $(shell grep 'Make_MODEM' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_MODEM' $(ROOTPATH)include\corelib.h))
MK_ADHOC = $(word $(words $(shell grep 'Make_ADHOC' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_ADHOC' $(ROOTPATH)include\corelib.h))

S_OBJS =

C_OBJS =  $(OBJ)usb_helper.o 

ALL_OBJS =  $(C_OBJS) $(S_OBJS)

all: $(TARGET) makefile

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)*.lst
