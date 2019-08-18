ROOTPATH=../../../
include ../../../config.mak

TARGET = $(LIB)libwpa_supplicant.a

MK_USB = $(word $(words $(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h))
MK_WPS = $(word $(words $(shell grep 'Make_WPS' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_WPS' $(ROOTPATH)include\corelib.h))

CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -G 8 -I$(INC) -I$(G_INC) -I../../../libSrc/lwip/include -I../../../libSrc/sdio/include -DBOOT_ROM_TYPE 
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -ggdb -G 8 -I$(INC) -I$(G_INC) -I../../../libSrc/lwip/include -I../../../libSrc/sdio/include -DBOOT_ROM_TYPE 
ifeq ($(MK_USB),5)
CC_OPTION += -include $(ROOTPATH)libSrc/os_linux27/include/linux/version.h
CC_OPTION += -I../../../libSrc/ar9271/include 
CC_OPTION += -I../../../libSrc/os_linux27/include 
else
ifeq ($(MK_USB),7)
CC_OPTION += -include $(ROOTPATH)libSrc/os_linux27/include/linux/version.h
CC_OPTION += -I../../../libSrc/os_linux27/include
else
ifeq ($(MK_USB),8)
CC_OPTION += -include $(ROOTPATH)libSrc/os_linux27/include/linux/version.h
CC_OPTION += -I../../../libSrc/os_linux27/include
else
CC_OPTION += -I../../../libSrc/os_linux/include 
endif
endif
endif
#ifeq ($(MK_USB),3)
CC_OPTION +=  -I../../../libSrc/ar2524/include 
#endif
CC_OPTION += -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS =  $(OBJ)drivers.o $(OBJ)driver_wext.o $(OBJ)common.o $(OBJ)wpa_debug.o $(OBJ)config.o \
		  $(OBJ)os_unix.o $(OBJ)ms_funcs.o $(OBJ)tlsv1_client_write.o $(OBJ)tlsv1_client_read.o $(OBJ)tlsv1_record.o \
		  $(OBJ)tlsv1_cred.o $(OBJ)tlsv1_client.o $(OBJ)tlsv1_common.o $(OBJ)tls_internal.o $(OBJ)aes.o\
		  $(OBJ)crypto_internal.o $(OBJ)bignum.o $(OBJ)pmksa_cache.o $(OBJ)preauth.o $(OBJ)wpa.o \
		 	$(OBJ)wpabuf.o $(OBJ)wpa_ie.o $(OBJ)wpa_common.o $(OBJ)scan_helpers.o $(OBJ)rsa.o $(OBJ)aes_wrap.o \
		 	$(OBJ)x509v3.o $(OBJ)asn1.o $(OBJ)base64.o $(OBJ)eap_common.o $(OBJ)chap.o \
		  $(OBJ)des.o $(OBJ)sha1.o $(OBJ)rc4.o $(OBJ)md4.o $(OBJ)md5.o $(OBJ)mschapv2.o $(OBJ)tncc.o \
		  $(OBJ)eloop.o \
		  $(OBJ)blacklist.o\
		  $(OBJ)eap_sim_common.o $(OBJ)scan.o \
 		  $(OBJ)wpas_glue.o $(OBJ)wpa_supplicant.o $(OBJ)l2_packet_linux.o $(OBJ)events.o $(OBJ)wpa_main.o \
 		  $(OBJ)driver_event.o $(OBJ)Config_file.o $(OBJ)wpa_ctrl.o $(OBJ)ctrl_iface.o $(OBJ)ctrl_iface_uitron.o \
 		  $(OBJ)wpa_ctrl_mpx.o 

ALL_OBJS =  $(S_OBJS) $(C_OBJS)

all: $(TARGET) makefile

ifeq ($(MK_USB),1)
C_OBJS +=  $(OBJ)driver_ralink.o 
endif	

ifeq ($(MK_USB),2)
endif		

ifeq ($(MK_USB),3)
C_OBJS +=  $(OBJ)driver_zydas.o 
endif

ifeq ($(MK_USB),4)
C_OBJS +=  $(OBJ)driver_ralink.o 
C_OBJS +=  $(OBJ)driver_zydas.o 
endif

ifeq ($(MK_USB),7)
C_OBJS += $(OBJ)eapol_supp_sm.o
C_OBJS += $(OBJ)eap_methods.o
C_OBJS += $(OBJ)eap.o
C_OBJS += $(OBJ)eap_md5.o
C_OBJS += $(OBJ)eap_tls.o
C_OBJS += $(OBJ)eap_mschapv2.o
C_OBJS += $(OBJ)eap_peap.o
C_OBJS += $(OBJ)eap_ttls.o
C_OBJS += $(OBJ)eap_gtc.o
C_OBJS += $(OBJ)eap_otp.o
C_OBJS += $(OBJ)eap_leap.o
C_OBJS += $(OBJ)eap_tnc.o
C_OBJS += $(OBJ)eap_tls_common.o
C_OBJS += $(OBJ)eap_peap_common.o
endif

ifeq ($(MK_WPS),1) 	
C_OBJS += $(OBJ)eapol_supp_sm.o 	
C_OBJS += $(OBJ)eap.o
C_OBJS += $(OBJ)eap_methods.o
C_OBJS += $(OBJ)wps_winrvp.o
C_OBJS += $(OBJ)wps_supplicant.o 
C_OBJS += $(OBJ)wps.o
C_OBJS += $(OBJ)uuid.o
C_OBJS += $(OBJ)wps_common.o
C_OBJS += $(OBJ)wps_registrar.o 
C_OBJS += $(OBJ)wps_dev_attr.o 
C_OBJS += $(OBJ)wps_attr_build.o
C_OBJS += $(OBJ)wps_attr_parse.o
C_OBJS += $(OBJ)wps_attr_process.o
C_OBJS += $(OBJ)wps_enrollee.o 
C_OBJS += $(OBJ)sha256.o
C_OBJS += $(OBJ)dh_groups.o
C_OBJS += $(OBJ)eap_wsc.o
C_OBJS += $(OBJ)eap_wsc_common.o 
endif	  

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
