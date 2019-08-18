ROOTPATH=../../../../
include $(ROOTPATH)config.mak

LIB = $(ROOTPATH)lib/

TARGET = $(LIB)libath9kv32.a

G_INC = $(ROOTPATH)include/

CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -G 8 -I$(INC) -I$(G_INC) -I$(ROOTPATH)libSrc/lwip/include -I$(ROOTPATH)libSrc/usbotg/include/ -DDBG -DBOOT_ROM_TYPE 

CC_OPTION += -include $(ROOTPATH)libSrc/mac80211_3/include/linux/compat-2.6.h
CC_OPTION +=  -I$(ROOTPATH)libSrc/mac80211_3/include 
CC_OPTION +=  -I$(ROOTPATH)libSrc/ath9kv32/include 
CC_OPTION +=  -I$(ROOTPATH)libSrc/os_linux27/include 
CC_OPTION +=  -D__KERNEL__
#CC_OPTION +=  -DMP662W_CHECK
CC_OPTION +=  -DCONFIG_ATH9K_RATE_CONTROL
CC_OPTION +=  -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =
C_OBJS =	$(OBJ)main.o \
					$(OBJ)hw.o \
					$(OBJ)init.o \
					$(OBJ)mac.o \
					$(OBJ)recv.o \
					$(OBJ)xmit.o \
					$(OBJ)eeprom.o \
					$(OBJ)eeprom_def.o \
					$(OBJ)eeprom_4k.o \
					$(OBJ)ar9003_eeprom.o \
					$(OBJ)calib.o \
					$(OBJ)beacon.o \
					$(OBJ)ar9002_calib.o \
					$(OBJ)ar9002_hw.o \
					$(OBJ)ar9003_hw.o \
					$(OBJ)ar9002_mac.o \
					$(OBJ)ar9003_mac.o \
					$(OBJ)ar9002_phy.o \
					$(OBJ)ar5008_phy.o \
					$(OBJ)common.o \
					$(OBJ)debug.o \
					$(OBJ)gpio.o \
					$(OBJ)hif_usb.o \
					$(OBJ)htc_drv_beacon.o \
					$(OBJ)htc_drv_init.o \
					$(OBJ)htc_drv_main.o \
					$(OBJ)htc_drv_txrx.o \
					$(OBJ)htc_drv_gpio.o \
					$(OBJ)htc_hst.o \
					$(OBJ)rc.o \
					$(OBJ)wmi.o \
					$(OBJ)mpx_ath9k.o \
					$(OBJ)ani.o \
					$(OBJ)htc_9271_fw.o \
					$(OBJ)ar9271_fw.o
					
					#
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
		
