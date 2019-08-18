ROOTPATH=../../../
include ../../../config.mak

TARGET = $(LIB)libhostapd.a

-include .config

MK_USB = $(word $(words $(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h))
MK_WPS = $(word $(words $(shell grep 'Make_WPS' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_WPS' $(ROOTPATH)include\corelib.h))

CC = $(BIN)gcc
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -G 8 -I$(INC) -I$(INC)utils/ -I$(G_INC) 
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -G 8 -I$(INC) -I$(INC)utils/ -I$(G_INC) 
CC_OPTION += -ggdb
CC_OPTION += -I../../../libSrc/compiler/include 
CC_OPTION += -I../../../libSrc/lwip/include 
CC_OPTION += -I../../../libSrc/netlink/include 
CC_OPTION += -I../../../libSrc/OPENSSL/include 
CC_OPTION += -DHOSTAPD
#CC_OPTION += -DWPA_TRACE
CC_OPTION += -DCONFIG_NO_RADIUS
CC_OPTION += -DCONFIG_NO_ACCOUNTING
CC_OPTION += -DCONFIG_NO_VLAN
CC_OPTION += -DCONFIG_CTRL_IFACE -DCONFIG_CTRL_IFACE_UNIX
CC_OPTION += -DCONFIG_IEEE80211N
CC_OPTION += -DCONFIG_NO_HOSTAPD_LOGGER
CC_OPTION += -DPLATFORM_MPIXEL
CC_OPTION += -DNEED_AP_MLME
CC_OPTION += -D_MIPS_SZPTR=32
CC_OPTION += -DUSE_KERNEL_HEADERS
CC_OPTION += -D__USE_EXTERN_INLINES
#CC_OPTION += -D_EXTERN_INLINE=static\ inline
#DRV_OPTION += -DCONFIG_DRIVER_NL80211

ifeq ($(MK_USB),7)
CC_OPTION += -include $(ROOTPATH)libSrc/os_linux27/include/linux/version.h
CC_OPTION += -I../../../libSrc/os_linux27/include
else
CC_OPTION += -I../../../libSrc/os_linux/include 
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

C_OBJS =  $(OBJ)main.o $(OBJ)ctrl_iface.o \
		  $(OBJ)config_file.o $(OBJ)eap_register.o $(OBJ)iplay.o 

AP_OBJS =  $(OBJ)ieee802_11.o $(OBJ)beacon.o $(OBJ)wpa_auth_ie.o $(OBJ)wpa_auth.o $(OBJ)wpa_auth_glue.o \
 		  $(OBJ)ieee802_1x.o $(OBJ)utils.o $(OBJ)wmm.o $(OBJ)ap_list.o $(OBJ)hw_features.o \
 		  $(OBJ)ap_mlme.o $(OBJ)preauth_auth.o $(OBJ)pmksa_cache_auth.o $(OBJ)ieee802_11_shared.o \
 		  $(OBJ)ap_config.o $(OBJ)ap_drv_ops.o $(OBJ)drv_callbacks.o $(OBJ)authsrv.o \
 		  $(OBJ)tkip_countermeasures.o \
 		  $(OBJ)hostapd.o $(OBJ)ieee802_11_auth.o $(OBJ)ctrl_iface_ap.o \
		  $(OBJ)ieee802_11_ht.o $(OBJ)sta_info.o 

#CRYPTO_OBJS =  $(OBJ)crypto_openssl.o \
# 		  $(OBJ)sha1-prf.o $(OBJ)tls_none.o $(OBJ)md5.o $(OBJ)aes-wrap.o \
#		  $(OBJ)random.o 

CRYPTO_OBJS =  $(OBJ)random.o \
 		  $(OBJ)sha1-prf.o $(OBJ)sha1-pbkdf2.o $(OBJ)md5.o $(OBJ)md5-internal.o $(OBJ)aes-wrap.o \
 		  $(OBJ)aes-internal-enc.o \
 		  $(OBJ)aes-internal.o \
		  $(OBJ)rc4.o $(OBJ)sha1.o $(OBJ)sha1-internal.o 

EAP_SERVER_OBJS =  $(OBJ)eap_server_identity.o $(OBJ)eap_server_methods.o \
 		  $(OBJ)eap_server.o 

EAPOL_AUTH_OBJS =  $(OBJ)eapol_auth_sm.o 

EAP_COMMON_OBJS =  $(OBJ)eap_common.o 

DRIVERS_OBJS =  $(OBJ)drivers.o \
 		  $(OBJ)driver_common.o 

COMMON_OBJS =  $(OBJ)wpa_ctrl.o \
 		  $(OBJ)ieee802_11_common.o \
 		  $(OBJ)wpa_common.o 

L2_PACKET_OBJS = $(OBJ)l2_packet_none.o 

UTILS_OBJS = $(OBJ)wpabuf.o $(OBJ)eloop.o $(OBJ)common.o $(OBJ)os_internal.o \
 		  $(OBJ)wpa_debug.o $(OBJ)ip_addr.o $(OBJ)radiotap.o

ACS_OBJS = $(OBJ)acs.o 

all: $(TARGET) makefile

NEED_RC4=y
NEED_AES=y
NEED_MD5=y
NEED_SHA1=y
CONFIG_NO_ACCOUNTING=y

include ../include/drivers/drivers.mak

DRIVERS_OBJS +=  $(DRV_OBJS)

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

ALL_OBJS =  $(S_OBJS) $(C_OBJS) $(AP_OBJS) $(EAP_SERVER_OBJS) $(DRIVERS_OBJS) $(COMMON_OBJS) $(L2_PACKET_OBJS) $(UTILS_OBJS) $(CRYPTO_OBJS) \
	$(EAPOL_AUTH_OBJS) $(EAP_COMMON_OBJS) $(ACS_OBJS)



all: $(TARGET) makefile


$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(AP_OBJS):	$(OBJ)%.o: $(INC)ap/%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(DRIVERS_OBJS):	$(OBJ)%.o: $(INC)drivers/%.c makefile 
	$(CC) $(DRV_CFLAGS) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(EAP_SERVER_OBJS):	$(OBJ)%.o: $(INC)eap_server/%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(EAP_COMMON_OBJS):	$(OBJ)%.o: $(INC)eap_common/%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(EAPOL_AUTH_OBJS):	$(OBJ)%.o: $(INC)eapol_auth/%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(CRYPTO_OBJS):	$(OBJ)%.o: $(INC)crypto/%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(COMMON_OBJS):	$(OBJ)%.o: $(INC)common/%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(L2_PACKET_OBJS):	$(OBJ)%.o: $(INC)/l2_packet/%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(UTILS_OBJS):	$(OBJ)%.o: $(INC)/utils/%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(ACS_OBJS):	$(OBJ)%.o: $(INC)/acs/%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<
clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst $(TARGET)
