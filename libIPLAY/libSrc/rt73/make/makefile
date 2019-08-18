include ../../../config.mak

ROOTPATH=../../../

TARGET = $(LIB)librt73.a

CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -G 8 -I$(INC) -I$(G_INC) -I../../../libSrc/lwip/include -I../../usbotg/include/ -DDBG -DBOOT_ROM_TYPE 
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -ggdb -G 8 -I$(INC) -I$(G_INC) -I../../../libSrc/lwip/include -I../../usbotg/include/ -DBOOT_ROM_TYPE -Wa,-ahl=
CC_OPTION +=  -DDEBUG -I../../../libSrc/os_linux/include 
CC_OPTION +=  -DDEBUG -I../../../libSrc/os_linux/include/linux 
CC_OPTION +=  -DDEBUG -I../../../libSrc/os_linux/include/linux/usb 
CC_OPTION +=  -DDEBUG -I../../../libSrc/os_linux/include/asm 
CC_OPTION +=  -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =
C_OBJS =	$(OBJ)rtmp_main.o \
					$(OBJ)mlme.o \
					$(OBJ)connect.o \
					$(OBJ)rtusb_bulk.o \
					$(OBJ)rtusb_io.o \
					$(OBJ)sync.o \
					$(OBJ)assoc.o \
					$(OBJ)auth.o \
					$(OBJ)auth_rsp.o \
					$(OBJ)rtusb_data.o \
					$(OBJ)rtmp_init.o \
					$(OBJ)sanity.o \
					$(OBJ)rtmp_wep.o \
					$(OBJ)rtmp_info.o \
					$(OBJ)wpa.o		\
					$(OBJ)rtmp_tkip.o \
					$(OBJ)md5.o \
					$(OBJ)rt73_mp.o \
					$(OBJ)rtmp_wext.o 

					
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


		
