ROOTPATH=../../../
include ../../../config.mak

TARGET = $(LIB)libnl.a


MK_USB = $(word $(words $(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h))
MK_WPS = $(word $(words $(shell grep 'Make_WPS' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_WPS' $(ROOTPATH)include\corelib.h))

CC = $(BIN)gcc
//CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -G 8 -I$(INC) -I$(INC)utils/ -I$(G_INC) -DBOOT_ROM_TYPE 
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -G 8 -I$(INC) -I$(INC)utils/ -I$(G_INC) -DBOOT_ROM_TYPE 
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -ggdb -G 8 -I$(INC) -I$(G_INC) -I../../../libSrc/lwip/include -I../../../libSrc/sdio/include -DBOOT_ROM_TYPE 
CC_OPTION += -ggdb
CC_OPTION += -I../../../libSrc/compiler/include 
CC_OPTION += -I../../../libSrc/lwip/include 
CC_OPTION += -I../../../libSrc/OPENSSL/include 
#CC_OPTION += -DWPA_TRACE
CC_OPTION += -DPLATFORM_MPIXEL
#CC_OPTION += -D_GNU_SOURCE
CC_OPTION += -D_MIPS_SZPTR=32

#CC_OPTION += -I../../../libSrc/os_linux27/include 

ifeq ($(MK_USB),5)
#CC_OPTION += -include $(ROOTPATH)libSrc/os_linux27/include/linux/version.h
endif
CC_OPTION += -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS =  $(OBJ)iplay_libnl.o \
		  $(OBJ)msg.o $(OBJ)socket.o 

LIB_OBJS =  $(OBJ)handlers.o $(OBJ)nl.o $(OBJ)data.o \
		  $(OBJ)attr.o $(OBJ)utils.o $(OBJ)addr.o $(OBJ)error.o $(OBJ)object.o 

LIB_GENL_OBJS =  $(OBJ)genl.o $(OBJ)ctrl.o $(OBJ)family.o $(OBJ)mngt.o 

all: $(TARGET) makefile

NEED_RC4=y
NEED_AES=y
NEED_MD5=y
NEED_SHA1=y
CONFIG_NO_ACCOUNTING=y

ALL_OBJS =  $(S_OBJS) $(C_OBJS) $(LIB_OBJS) $(LIB_GENL_OBJS) 



all: $(TARGET) makefile


$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(LIB_OBJS):	$(OBJ)%.o: $(INC)lib/%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(LIB_GENL_OBJS):	$(OBJ)%.o: $(INC)lib/genl/%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)*.lst $(TARGET)
