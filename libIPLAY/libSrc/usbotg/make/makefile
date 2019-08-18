include ../../../config.mak

ROOTPATH=../../../

TARGET = $(LIB)libusbotg.a

MK_USB = $(word $(words $(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h))

CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -G 8 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -ggdb -G 8 -I$(INC) -I$(G_INC)  -DBOOT_ROM_TYPE -Wa,-ahl=
ifeq ($(MK_USB),5)
CC_OPTION +=  -I../../../libSrc/ath9k/include 
CC_OPTION +=  -I../../../libSrc/os_linux27/include 
else
ifeq ($(MK_USB),7)
CC_OPTION +=  -I../../../libSrc/os_linux27/include
else
ifeq ($(MK_USB),8)
CC_OPTION +=  -I../../../libSrc/mac80211_36/include 
CC_OPTION +=  -I../../../libSrc/os_linux27/include
else
CC_OPTION +=  -I../../../libSrc/os_linux/include 
CC_OPTION +=  -I../../../libSrc/mac80211/include 
CC_OPTION +=  -I../../../libSrc/ar2524/include 
endif
endif
endif
CC_OPTION +=  -I../../../libSrc/lwip/include
CC_OPTION +=  -D__KERNEL__
CC_OPTION +=  -Wa,-ahl=
AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=
 
LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS =	$(OBJ)usbotg_host.o $(OBJ)usbotg_host_sm.o $(OBJ)usbotg_host_setup.o $(OBJ)usbotg_host_msdc.o $(OBJ)usbotg_host_sidc.o \
		$(OBJ)usbotg_host_hub.o $(OBJ)usbotg_device.o $(OBJ)usbotg_bt.o $(OBJ)usbotg_device_sidc.o $(OBJ)usbotg_ctrl.o \
		$(OBJ)Usbotg_host_web_cam.o $(OBJ)Usbotg_host_urb.o $(OBJ)Usbotg_host_cdc.o $(OBJ)usbotg_device_cdc.o $(OBJ)usbotg_utility.o \
		$(OBJ)usbotg_device_iso.o  $(OBJ)Usbotg_Device_WebCam.o $(OBJ)Usbotg_sensorcfg.o 	$(OBJ)usbotg_device_protection.o \
		$(OBJ)usbotg_device_extern.o $(OBJ)usbotg_device_extern_sam.o $(OBJ)Usbotg_host_hid.o $(OBJ)usbotg_device_msdc_teco.o

C_OBJS += $(OBJ)usbotg_device_msdc.o 
#C_OBJS += $(OBJ)usbotg_device_msdc_sdram.o   #USB OTG Device MSDC for 12MB SD-RAM R/W Test with PC


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
	
