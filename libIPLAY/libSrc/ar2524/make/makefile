include ../../../config.mak
ROOTPATH=../../../
MK_ADHOC = $(word $(words $(shell grep 'Make_ADHOC' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_ADHOC' $(ROOTPATH)include\corelib.h))
TARGET = $(LIB)libar2524.a

CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -G 8 -I$(INC) -I$(G_INC) -I../../../libSrc/lwip/include -I../../usbotg/include/ -DDBG -DBOOT_ROM_TYPE 
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -ggdb -G 8 -I$(INC) -I$(G_INC) -I../../../libSrc/lwip/include -I../../usbotg/include/ -DBOOT_ROM_TYPE
CC_OPTION +=  -I../../../libSrc/os_linux/include 
CC_OPTION +=  -I../../../libSrc/mac80211/include 
CC_OPTION +=  -DZD1211B -DZDCONF_WE_STAT_SUPPORT=1 -DZDCONF_WEXT_WPA=1
CC_OPTION +=  -DZDCONF_DBGMSG_NORMAL=0 -DZDCONF_DTIM_BC_FIX=1
CC_OPTION +=  -D__KERNEL__
#CC_OPTION +=  -DHMAC_DEBUG=1
CC_OPTION +=  -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =
C_OBJS =	$(OBJ)zd_chip.o \
					$(OBJ)zd_ieee80211.o \
					$(OBJ)zd_mac.o \
					$(OBJ)zd_rf.o \
					$(OBJ)zd_rf_al2230.o \
					$(OBJ)zd_rf_al7230b.o \
					$(OBJ)zd_rf_rf2959.o \
					$(OBJ)zd_rf_uw2453.o \
					$(OBJ)zd_rf_ar2124.o \
					$(OBJ)zd_usb.o \
					$(OBJ)zd_mpx.o \
					$(OBJ)zd_fw.o \
					$(OBJ)zdhw.o \
					$(OBJ)zd1205.o \
					$(OBJ)zdglobal.o \
					$(OBJ)zddebug.o \
					$(OBJ)zdhci.o \
					$(OBJ)zdmic.o \
					$(OBJ)zd1211.o \
					$(OBJ)zdencrypt.o \
					$(OBJ)zdpsmon.o 
					
ALL_OBJS =  $(S_OBJS) $(C_OBJS)

all: $(TARGET) makefile

ifeq ($(MK_ADHOC),1)
C_OBJS += $(OBJ)zdpmfilter.o 
C_OBJS += $(OBJ)zdshared.o 
C_OBJS += $(OBJ)zdbuf.o 
endif

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst $(TARGET)
		
