ROOTPATH=../../../
include $(ROOTPATH)config.mak

LIB = $(ROOTPATH)lib/

TARGET = $(LIB)libathv32.a

G_INC = $(ROOTPATH)include/

CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -G 8 -I$(INC) -I$(G_INC) -I$(ROOTPATH)libSrc/lwip/include -I$(ROOTPATH)libSrc/usbotg/include/ -DDBG -DBOOT_ROM_TYPE 

CC_OPTION += -include $(ROOTPATH)libSrc/mac80211_3/include/linux/compat-2.6.h
CC_OPTION +=  -I$(ROOTPATH)libSrc/mac80211_3/include 
CC_OPTION +=  -I$(ROOTPATH)libSrc/os_linux27/include 
CC_OPTION +=  -D__KERNEL__
CC_OPTION +=  -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =
C_OBJS =	$(OBJ)main.o \
					$(OBJ)regd.o \
					$(OBJ)debug.o \
					$(OBJ)uartcmd.o \
					$(OBJ)hw.o 

#C_OBJS +=	$(OBJ)regd.o 
					
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
		
