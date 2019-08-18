include ../../../config.mak

ROOTPATH=../../../

TARGET = $(LIB)libnetusb.a

CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -G 8 -ggdb -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -ggdb -G 8 -I$(INC) -I$(G_INC)  -DBOOT_ROM_TYPE -Wa,-ahl=
CC_OPTION +=  -I../../../libSrc/os_linux/include 
CC_OPTION +=  -I../../../libSrc/lwip/include
CC_OPTION +=  -I../../../libSrc/usbotg/include
CC_OPTION +=  -I../../../libSrc/Peripheral/INCLUDE
CC_OPTION +=  -D__KERNEL__
CC_OPTION +=  -Wa,-ahl=
AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS =	$(OBJ)atcmd_usb.o \
		$(OBJ)netusb.o $(OBJ)qisda_h21.o $(OBJ)atcmd.o $(OBJ)threeg.o 

ALL_OBJS =	$(S_OBJS) $(C_OBJS)

all: $(TARGET)

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile $(G_INC)UtilRegFile.h
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst $(TARGET)
	
