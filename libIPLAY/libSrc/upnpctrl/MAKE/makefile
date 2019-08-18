include ../../../config.mak
 
TARGET = $(LIB)libupnpctrl.a

CC = $(BIN)gcc
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -G 8 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -ggdb -G 8 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE 
CC_OPTION +=  -I../../../libSrc/os_linux/include 
CC_OPTION += -I../../../libSrc/ixml/include 
CC_OPTION +=  -I../../../libSrc/lwip/include
CC_OPTION +=  -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS = $(OBJ)soap_ctrlpt.o $(OBJ)soap_common.o  $(OBJ)httpparser.o $(OBJ)httpreadwrite.o $(OBJ)membuffer.o $(OBJ)parsetools.o $(OBJ)soap_device.o \
		 $(OBJ)LinkedList.o $(OBJ)FreeList.o $(OBJ)strintmap.o $(OBJ)uri.o $(OBJ)sock.o $(OBJ)upnpapi.o $(OBJ)util.o $(OBJ)statcodes.o \
		 $(OBJ)gena_callback2.o $(OBJ)miniserver.o $(OBJ)gena_device.o $(OBJ)gena_ctrlpt.o $(OBJ)upnp_timeout.o $(OBJ)client_table.o \
		 $(OBJ)uuid.o $(OBJ)sysdep.o $(OBJ)service_table.o $(OBJ)ssdp_server.o $(OBJ)ssdp_ctrlpt.o $(OBJ)upnp_tv_ctrlpt.o \
		 $(OBJ)sample_util.o $(OBJ)upnptools.o $(OBJ)upnp_tv_ctrlpt_main.o $(OBJ)upnp_tv_device.o $(OBJ)webserver.o $(OBJ)ssdp_device.o $(OBJ)md5.o $(OBJ)mrdesc_xml.o\
		 $(OBJ)ixml_mem.o
          
ALL_OBJS =  $(C_OBJS) $(S_OBJS)

all: $(TARGET) makefile

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst  $(TARGET)

