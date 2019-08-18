include ../../../config.mak
 
ROOTPATH=../../../
 
TARGET = $(LIB)liblwip.a

MK_USB = $(word $(words $(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h))
MK_DM9621 = $(word $(words $(shell grep 'Make_DM9621_ETHERNET' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_DM9621_ETHERNET' $(ROOTPATH)include\corelib.h))

CC = $(BIN)gcc
CC_OPTION += -O2 -G 8
#CC_OPTION += -O0 -ggdb
CC_OPTION += -I$(INC) -I$(G_INC) 
ifeq ($(MK_USB),5)
CC_OPTION += -include $(ROOTPATH)libSrc/os_linux27/include/linux/compat-2.6.h
CC_OPTION += -I../../../libSrc/ath9k/include 
CC_OPTION += -I../../../libSrc/os_linux27/include 
else
ifeq ($(MK_USB),7)
CC_OPTION += -include $(ROOTPATH)libSrc/os_linux27/include/linux/compat-2.6.h
CC_OPTION += -I../../../libSrc/os_linux27/include
else 
ifeq ($(MK_USB),8)
CC_OPTION += -include $(ROOTPATH)libSrc/mac80211_36/include/linux/compat-2.6.h
CC_OPTION += -I../../../libSrc/mac80211_36/include 
CC_OPTION += -I../../../libSrc/os_linux27/include
else 
CC_OPTION += -I../../../libSrc/os_linux/include 
endif 
endif
endif
ifeq ($(MK_DM9621),1)
CC_OPTION += -I../../../libSrc/dm9621/include 
endif
CC_OPTION += -I../../../libSrc/openssl/include
CC_OPTION += -I../../../libSrc/zip/include
CC_OPTION += -DHAVE_LWIP_SOCKET
CC_OPTION +=  -D__KERNEL__
#CC_OPTION += -DCONFIG_NO_MALLOC
#CC_OPTION += -DNETWORK_DEBUG_ENABLE 
CC_OPTION += -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

MK_SMTP = $(word $(words $(shell grep 'Make_SMTP' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_SMTP' $(ROOTPATH)include\corelib.h))
MK_UMODEM = $(word $(words $(shell grep 'Make_MODEM' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_MODEM' $(ROOTPATH)include\corelib.h))
MK_ADHOC = $(word $(words $(shell grep 'Make_ADHOC' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_ADHOC' $(ROOTPATH)include\corelib.h))
MK_WPA = $(word $(words $(shell grep 'Make_WPA' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_WPA' $(ROOTPATH)include\corelib.h))

S_OBJS =

C_OBJS =  $(OBJ)net_nic.o $(OBJ)net_utility.o $(OBJ)net_arp.o \
          $(OBJ)net_ip.o $(OBJ)net_icmp.o $(OBJ)net_udp.o $(OBJ)net_netctrl.o $(OBJ)os_mp52x.o \
          $(OBJ)net_ns.o $(OBJ)net_dhcp.o $(OBJ)net_ftp.o $(OBJ)net_ftpserver.o \
          $(OBJ)os_timer.o $(OBJ)util_queue.o $(OBJ)net_autosearch.o $(OBJ)webclient.o $(OBJ)net_sys.o \
          $(OBJ)uip_timer.o $(OBJ)webclient-strings.o $(OBJ)net_file.o \
          $(OBJ)wlan_utils.o $(OBJ)net_resolv.o $(OBJ)memfunc.o $(OBJ)netpool_mem.o \
          $(OBJ)net_helper.o $(OBJ)skbuff.o $(OBJ)net_sntp.o $(OBJ)ppp.o $(OBJ)lcp.o $(OBJ)fsm.o \
          $(OBJ)chap.o $(OBJ)randm.o $(OBJ)magic.o $(OBJ)auth.o $(OBJ)ipcp.o $(OBJ)pap.o $(OBJ)sio.o $(OBJ)vj.o \
          $(OBJ)aalMs_Net.o

C_OBJS +=  $(OBJ)net_socket.o $(OBJ)net_tcp.o $(OBJ)net_ioctl.o 
C_OBJS +=  $(OBJ)net_todo.o $(OBJ)hsm.o 
C_OBJS +=  $(OBJ)net_init.o
C_OBJS +=  $(OBJ)net_stat.o

C_OBJS +=  $(OBJ)log.o

ifeq ($(MK_UMODEM), 1)
C_OBJS +=  $(OBJ)mms.o $(OBJ)async_req.o $(OBJ)events.o $(OBJ)gsm.o $(OBJ)gsm_sm.o $(OBJ)gsm_timer.o \
          $(OBJ)tevent.o $(OBJ)tevent_req.o $(OBJ)tevent_timed.o $(OBJ)ttime.o $(OBJ)wwan_conn.o 
C_OBJS +=  $(OBJ)sms.o 
endif

ifeq ($(MK_SMTP), 1)
C_OBJS +=  $(OBJ)smtp_client.o 
endif
ifeq ($(MK_ADHOC), 1)
C_OBJS +=  $(OBJ)wlan_adhoc.o 
endif

ifeq ($(MK_WPA), 2)
C_OBJS +=  $(OBJ)wlan_hostapd.o 
endif

C_OBJS +=  $(OBJ)net_socket3.o 

ALL_OBJS =  $(C_OBJS) $(S_OBJS)

all: $(TARGET) makefile

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)*.lst $(TARGET)
