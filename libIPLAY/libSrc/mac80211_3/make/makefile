include ../../../config.mak

TARGET = $(LIB)libmac80211_3.a

CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -G 8 -I$(INC) -I$(G_INC) -DDBG -DBOOT_ROM_TYPE 
CC_OPTION += -include ../../../libSrc/mac80211_3/include/linux/compat-2.6.h
CC_OPTION += -I../../../libSrc/ath9kv32/include 
CC_OPTION += -I../../../libSrc/lwip/include 
CC_OPTION += -I../../../libSrc/usbotg/include 
CC_OPTION += -I../../../libSrc/os_linux27/include 
CC_OPTION += -I../../../libSrc/os/include 
CC_OPTION += -D__KERNEL__
CC_OPTION += -DPLATFORM_MPIXEL
CC_OPTION += -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =
C_OBJS =	$(OBJ)main.o \
			$(OBJ)rx.o \
			$(OBJ)tx.o \
			$(OBJ)key.o \
			$(OBJ)sta_info.o \
			$(OBJ)mlme.o \
			$(OBJ)cfg.o \
			$(OBJ)rate.o \
			$(OBJ)wep.o \
			$(OBJ)iface.o \
			$(OBJ)tkip.o \
			$(OBJ)work.o \
			$(OBJ)scan.o \
			$(OBJ)wpa.o \
			$(OBJ)michael.o \
			$(OBJ)event.o \
			$(OBJ)aes_ccm.o \
			$(OBJ)rc80211_minstrel.o \
			$(OBJ)rc80211_pid_algo.o \
			$(OBJ)agg-tx.o \
			$(OBJ)agg-rx.o \
			$(OBJ)led.o \
			$(OBJ)ht.o \
			$(OBJ)status.o \
			$(OBJ)spectmgmt.o \
			$(OBJ)wme.o \
			$(OBJ)aes_cmac.o \
			$(OBJ)ibss.o \
			$(OBJ)offchannel.o \
			$(OBJ)chan.o \
			$(OBJ)zdencrypt.o \
			$(OBJ)util.o 

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
		
