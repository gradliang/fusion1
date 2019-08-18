include ../../../config.mak

ROOTPATH=../../../
MK_RELEASE = $(word $(words $(shell grep 'e_RE' $(ROOTPATH)include\corelib.h)),$(shell grep 'e_RE' $(ROOTPATH)include\corelib.h))

TARGET = $(LIB)libr8188cu.a

CC = $(BIN)gcc
CC_OPTION += $(MAIN_FLAG) -march=r3000 -c -mno-abicalls
CC_OPTION += -O2 -G 8 
#CC_OPTION += -O0 -ggdb -G 8
CC_OPTION += -I$(INC) -I$(G_INC)
CC_OPTION += -I../../../libSrc/lwip/include
CC_OPTION += -I../../../libSrc/usbotg/include
CC_OPTION += -DBOOT_ROM_TYPE
CC_OPTION += -DDBG 
CC_OPTION += -include $(ROOTPATH)libSrc/os_linux27/include/linux/compat-2.6.h
CC_OPTION += -I../../../libSrc/os_linux27/include
#CC_OPTION += -I../../../libSrc/mac80211/include 
CC_OPTION += -DRTL8192CU
CC_OPTION += -D__KERNEL__
CC_OPTION +=  -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =
C_OBJS =	$(OBJ)Hal8192CUHWImg.o \
			$(OBJ)rtl8192c_sreset.o \
			$(OBJ)osdep_service.o \
			$(OBJ)os_intfs.o \
			$(OBJ)usb_intf.o \
			$(OBJ)ioctl_linux.o \
			$(OBJ)xmit_linux.o \
			$(OBJ)mlme_linux.o \
			$(OBJ)recv_linux.o \
			$(OBJ)hal_init.o \
			$(OBJ)rtl8192c_hal_init.o \
			$(OBJ)rtl8192c_phycfg.o	\
			$(OBJ)rtl8192c_rf6052.o \
			$(OBJ)rtl8192c_dm.o \
			$(OBJ)rtl8192c_rxdesc.o \
			$(OBJ)rtl8192c_cmd.o \
			$(OBJ)usb_ops_linux.o \
			$(OBJ)usb_halinit.o \
			$(OBJ)rtl8192cu_led.o \
			$(OBJ)rtl8192cu_xmit.o \
			$(OBJ)rtl8192cu_recv.o \
			$(OBJ)rtw_cmd.o \
			$(OBJ)rtw_security.o \
			$(OBJ)rtw_io.o \
			$(OBJ)rtw_ioctl_set.o \
			$(OBJ)rtw_ieee80211.o \
			$(OBJ)rtw_mlme.o \
			$(OBJ)rtw_mlme_ext.o \
			$(OBJ)rtw_wlan_util.o \
			$(OBJ)rtw_pwrctrl.o \
			$(OBJ)rtw_rf.o \
			$(OBJ)rtw_recv.o \
			$(OBJ)rtw_sta_mgt.o \
			$(OBJ)rtw_xmit.o \
			$(OBJ)rtw_efuse.o \
			$(OBJ)mpx_rtl8188cu.o

ALL_OBJS =  $(C_OBJS)	$(S_OBJS) 

all: $(TARGET) makefile


$(TARGET): 	$(ALL_OBJS)
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
