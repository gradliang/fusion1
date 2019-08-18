ROOTPATH=../../../
include ../../../config.mak

TARGET = $(LIB)libos_linux27.a

MK_WPA = $(word $(words $(shell grep 'Make_WPA' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_WPA' $(ROOTPATH)include\corelib.h))
MK_USB = $(word $(words $(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h))

CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -G 8 -ggdb -I$(G_INC) -I../../../libSrc/lwip/include -I../../usbotg/include/ -DDBG -DBOOT_ROM_TYPE 
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -ggdb -G 8 -I$(INC) -I$(G_INC) -I../../../libSrc/lwip/include -I../../usbotg/include/ -DBOOT_ROM_TYPE 

CC_OPTION += -include ../../../libSrc/os_linux27/include/linux/compat-2.6.h
CC_OPTION +=  -I$(ROOTPATH)libSrc/ath9k/include 
CC_OPTION +=  -I$(INC) 
CC_OPTION +=  -I$(ROOTPATH)libSrc/os_linux27/drivers/leds 
CC_OPTION +=  -D__KERNEL__ 
ifeq ($(MK_WPA),0)
CC_OPTION +=  -DCONFIG_LIB80211=0
CC_OPTION +=  -DCONFIG_CFG80211=0
endif
ifeq ($(MK_WPA),1)
ifeq ($(MK_USB),8)
CC_OPTION +=  -DHAVE_AF_NETLINK
CC_OPTION +=  -DHAVE_AF_PACKET
else 
CC_OPTION +=  -DCONFIG_LIB80211=0
CC_OPTION +=  -DCONFIG_CFG80211=0
endif
endif
ifeq ($(MK_WPA),2)
CC_OPTION +=  -DHAVE_AF_NETLINK
CC_OPTION +=  -DHAVE_AF_PACKET
endif
CC_OPTION +=  -I../../../libSrc/Os/include 
#CC_OPTION +=  -ggdb
CC_OPTION +=  -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =
C_OBJS =	$(OBJ)workqueue.o \
			$(OBJ)eth.o \
			$(OBJ)wlan_usb.o \
			$(OBJ)dev.o \
			$(OBJ)mpx_linux.o \
			$(OBJ)driver.o \
			$(OBJ)softirq.o \
			$(OBJ)usb.o \
			$(OBJ)timer.o \
			$(OBJ)crc32.o \
			$(OBJ)pm_qos_params.o \
			$(OBJ)hweight.o \
			$(OBJ)led-triggers.o \
			$(OBJ)leds.o \
			$(OBJ)rbtree.o \
			$(OBJ)stubs.o 
					
NETLINK_OBJS =  $(OBJ)genetlink.o $(OBJ)af_netlink.o $(OBJ)attr.o 
SECURITY_OBJS =  $(OBJ)commoncap.o 
NETCORE_OBJS =  $(OBJ)sock.o $(OBJ)datagram.o $(OBJ)iovec.o 

PACKET_OBJS =  $(OBJ)af_packet.o 

ALL_OBJS =  $(S_OBJS) $(C_OBJS) 

ALL_OBJS +=  $(NETLINK_OBJS) $(SECURITY_OBJS) $(PACKET_OBJS) 
NETCORE_OBJS +=  $(OBJ)rtnetlink.o 

ALL_OBJS +=  $(NETCORE_OBJS) 

all: $(TARGET) makefile


$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(NETLINK_OBJS):	$(OBJ)%.o: $(INC)net/netlink/%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(PACKET_OBJS):	$(OBJ)%.o: $(INC)net/packet/%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<


$(SECURITY_OBJS):	$(OBJ)%.o: $(INC)security/%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(NETCORE_OBJS):	$(OBJ)%.o: $(INC)net/core/%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst $(TARGET)
		
