include ../../../config.mak

ROOTPATH=../../../
MK_RELEASE = $(word $(words $(shell grep 'e_RE' $(ROOTPATH)include\iplaysysconfig.h)),$(shell grep 'e_RE' $(ROOTPATH)include\iplaysysconfig.h))

TARGET = $(LIB)libwls.a

CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -G 8 -I$(INC) -I$(G_INC) -I../../../libSrc/lwip/include -I../../usbotg/include/ -DDBG -DBOOT_ROM_TYPE 
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -ggdb -G 8 -I$(INC) -I$(G_INC) -I../../../libSrc/lwip/include -I../../usbotg/include/ -DBOOT_ROM_TYPE -Wa,-ahl=
CC_OPTION +=  -DDEBUG -I../../../libSrc/os_linux/include 
CC_OPTION +=  -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =
C_OBJS =	$(OBJ)windows_live.o $(OBJ)windows_live_id.o 
					
ALL_OBJS =  $(S_OBJS) $(C_OBJS)



all: $(TARGET) makefile


$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst
ifeq ($(MK_RELEASE), 0)	
	rm -f $(TARGET)
endif