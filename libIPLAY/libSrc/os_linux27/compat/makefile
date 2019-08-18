ROOTPATH=../../../
include ../../../config.mak

TARGET = $(LIB)libcompat.a

SRC = ../compat/

INC = ./

CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -G 8 -I$(G_INC) -I../../../libSrc/lwip/include -I../../usbotg/include/ -DDBG -DBOOT_ROM_TYPE 
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -ggdb -G 8 -I$(INC) -I$(G_INC) -I../../../libSrc/lwip/include -I../../usbotg/include/ -DBOOT_ROM_TYPE 

CC_OPTION += -include ../../../libSrc/mac80211_36/include/linux/compat-2.6.h
CC_OPTION +=  -ggdb
CC_OPTION +=  -I$(ROOTPATH)libSrc/mac80211_36/include 
CC_OPTION +=  -I$(INC) 
CC_OPTION +=  -I$(ROOTPATH)libSrc/os_linux27/wireless_36 
CC_OPTION +=  -I$(ROOTPATH)libSrc/os_linux27/include 
CC_OPTION +=  -D__KERNEL__ 
CC_OPTION +=  -DPLATFORM_MPIXEL
CC_OPTION +=  -I../../../libSrc/Os/include 
CC_OPTION +=  -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =
C_OBJS =	$(OBJ)compat-2.6.37.o $(OBJ)compat-2.6.29.o
C_OBJS +=	$(OBJ)compat-2.6.36.o 
					
ALL_OBJS =  $(S_OBJS) $(C_OBJS)

all: $(TARGET) makefile


$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst $(TARGET)
		
