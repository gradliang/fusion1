ROOTPATH=../
LIBSRCPATH=../libsrc/

include ../config.mak
TARGET = libIPLAY


##################################################################################################
# SWITCH
##################################################################################################

CC_OPTION += -mno-abicalls $(OPT2) $(ALL_INCLUDE) -Wa,-al=$(OBJ)

#         all : mpx612

##################################################################################################
# RULES
##################################################################################################


ALL_LIBS =	$(LIB)libbios.a $(LIB)libos.a $(LIB)libMcard.a $(LIB)libfile.a $(LIB)libdisplay.a $(LIB)libsensor.a $(LIB)libimage.a \
		$(LIB)libusbotg.a $(LIB)libmpapi.a $(LIB)libdemux.a $(LIB)libmad.a \
		$(LIB)libswmp3.a $(LIB)libsoftaviwave.a\
		$(LIB)libstream.a \
		$(LIB)libutilities.a $(LIB)libavipacker.a\
		$(LIB)libffmpeg.a $(LIB)libperipheral.a $(LIB)libsystem.a \
		$(LIB)libutil.a $(LIB)libRecord.a\
		$(LIB)libtouchctrl.a \

#		$(LIB)libmpeg4.a $(LIB)libdwl.a $(LIB)libh264.a $(LIB)libmpeg12.a $(LIB)libpp.a\
#		$(LIB)libmjpg.a \
#		$(LIB)libswaac.a $(LIB)libswwma.a $(LIB)libswac3.a $(LIB)libswogg.a $(LIB)libmp4ff.a  $(LIB)libswmad.a  $(LIB)libhwmad.a $(LIB)libswamr.a $(LIB)libswram.a 
#		$(LIB)libxml.a $(LIB)libsubtitle.a $(LIB)libmpxtool.a $(LIB)libtext.a 
#		$(LIB)libptpdps.a \
#$(LIB)libxpg.a 

#############	 BELOW IS OBSOLETE	###################
#		$(LIB)libjpeg.a \
#		$(LIB)libdiv3.a $(LIB)libmpeg2.a $(LIB)libxvid.a $(LIB)libmjpeg.a \


libIPLAY: bios Mcard file os image demux softaviwave \
		 ffmpeg xpg usbotg mpapi swmp3 utilities stream libmad \
		util \
		peripheral system display sensor avipacker record\
		touchctrl\
#		mpeg4 dwl h264 mpeg12 pp\
#		mjpg \

# swaac hwmad swwma swac3 swogg libmp4ff swmad swamr swram
# xml subtitle mpxtool  libfaad hwfaad ptpdps text

#############	 BELOW IS OBSOLETE	###################
#		div3 mpeg2 xvid mjpeg \
#		jpeg \

#av:	audio demux mpapi swmp3 swwma mjpeg swaac softaviwave xvid mpeg4 dwl h264 mpeg12 pp div3 mpeg2 stream ffmpeg \
#	swac3 utilities swogg swram swmad swamr
#	hwfaad

fileIO: file MCard ptpdps
usb: ptpdps

MK_TESTCONSOLE = $(word $(words $(shell grep 'Make_TESTCONSOLE' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_TESTCONSOLE' $(ROOTPATH)include\corelib.h))
MK_WIFI = $(word $(words $(shell grep 'NETWARE_ENABLE' $(ROOTPATH)include\corelib.h)),$(shell grep 'NETWARE_ENABLE' $(ROOTPATH)include\corelib.h))
MK_ADHOC = $(word $(words $(shell grep 'Make_ADHOC' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_ADHOC' $(ROOTPATH)include\corelib.h))
MK_WPA = $(word $(words $(shell grep 'Make_WPA' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_WPA' $(ROOTPATH)include\corelib.h))
MK_LWIP = $(word $(words $(shell grep 'Make_LWIP' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_LWIP' $(ROOTPATH)include\corelib.h))
MK_CURL = $(word $(words $(shell grep 'Make_CURL' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_CURL' $(ROOTPATH)include\corelib.h))
MK_OPENSSL = $(word $(words $(shell grep 'Make_OPENSSL' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_OPENSSL' $(ROOTPATH)include\corelib.h))
MK_UPNP_SDK = $(word $(words $(shell grep 'Make_UPNP_SDK' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_UPNP_SDK' $(ROOTPATH)include\corelib.h))
MK_DM9K_DRIVER = $(word $(words $(shell grep 'Make_DM9KS_ETHERNET' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_DM9KS_ETHERNET' $(ROOTPATH)include\corelib.h))
MK_DM9621_DRIVER = $(word $(words $(shell grep 'Make_DM9621_ETHERNET' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_DM9621_ETHERNET' $(ROOTPATH)include\corelib.h))
MK_SDIO = $(word $(words $(shell grep 'Make_SDIO' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_SDIO' $(ROOTPATH)include\corelib.h))
MK_USB = $(word $(words $(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h))
MK_RTF = $(word $(words $(shell grep 'e_UNRTF' $(ROOTPATH)include\corelib.h)),$(shell grep 'e_UNRTF' $(ROOTPATH)include\corelib.h))
MK_ANTIWORD = $(word $(words $(shell grep 'e_ANTIWORD' $(ROOTPATH)include\corelib.h)),$(shell grep 'e_ANTIWORD' $(ROOTPATH)include\corelib.h))
MK_LRF = $(word $(words $(shell grep 'e_LRF' $(ROOTPATH)include\corelib.h)),$(shell grep 'e_LRF' $(ROOTPATH)include\corelib.h))
MK_TTF = $(word $(words $(shell grep 'Make_TTF' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_TTF' $(ROOTPATH)include\corelib.h))
MK_PDB = $(word $(words $(shell grep 'e_PDB' $(ROOTPATH)include\corelib.h)),$(shell grep 'e_PDB' $(ROOTPATH)include\corelib.h))
MK_RELEASE = $(word $(words $(shell grep 'Make_RELEASE' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_RELEASE' $(ROOTPATH)include\corelib.h))
EREADER_ENABLE = $(word $(words $(shell grep 'EREADER_ENABLE' $(ROOTPATH)include\corelib.h)),$(shell grep 'EREADER_ENABLE' $(ROOTPATH)include\corelib.h))
MK_TXT = $(word $(words $(shell grep 'Make_TXT' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_TXT' $(ROOTPATH)include\corelib.h))
MK_UMODEM = $(word $(words $(shell grep 'Make_MODEM' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_MODEM' $(ROOTPATH)include\corelib.h))
MK_OSIP = $(word $(words $(shell grep 'Make_OSIP' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_OSIP' $(ROOTPATH)include\corelib.h))
PLATFORM = $(word $(words $(shell grep 'e ___' $(ROOTPATH)include\iplaysysconfig.h)),$(shell grep 'e ___' $(ROOTPATH)include\iplaysysconfig.h))
MK_RTSP = $(word $(words $(shell grep 'Make_RTSP' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_RTSP' $(ROOTPATH)include\corelib.h))

ifeq ($(MK_WIFI), 1)

ALL_LIBS +=	$(LIB)libnetstream.a
libIPLAY : netstream

ifeq ($(MK_RELEASE),0)

ifeq ($(MK_USB),1)
ALL_LIBS +=	$(LIB)librt73.a
libIPLAY : rt73
endif

ifeq ($(MK_USB),3)
ALL_LIBS +=	$(LIB)libmac80211.a $(LIB)libar2524.a $(LIB)libos_linux.a $(LIB)libnethelper.a 
libIPLAY : mac80211 ar2524 os_linux nethelper
endif

ifeq ($(MK_USB),4)
ALL_LIBS +=	$(LIB)librt73.a $(LIB)libmac80211.a $(LIB)libar2524.a $(LIB)libos_linux.a
libIPLAY : rt73 mac80211 ar2524 os_linux
endif

# add to support Atheros AR9271
ifeq ($(MK_USB),5)

ifeq ($(MK_WPA),1)
ALL_LIBS += $(LIB)libos_linux27.a
libIPLAY : os_linux27
endif

ifeq ($(MK_WPA),2)
ALL_LIBS +=	$(LIB)libos_linux27.a $(LIB)libwireless_3.a $(LIB)compat.a 
libIPLAY : os_linux27 wireless_3 compat 
endif

endif

# RTSP
ifeq ($(MK_RTSP),1)
ALL_LIBS +=	$(LIB)libavformat.a $(LIB)libavutil.a $(LIB)libavcodec.a 
libIPLAY : av
endif

ifeq ($(MK_USB),7)
ALL_LIBS += $(LIB)libr8188cu.a $(LIB)libos_linux27.a $(LIB)libnethelper.a
libIPLAY : r8188c os_linux27 nethelper
endif

ifeq ($(MK_USB),8)
ALL_LIBS += $(LIB)libr8188e.a $(LIB)libos_linux27.a $(LIB)libwireless_36.a $(LIB)libnethelper.a
libIPLAY : r8188e os_linux27 wireless_36 nethelper
endif

ifeq ($(MK_LWIP),1)
     ALL_LIBS +=	$(LIB)liblwip.a 
     libIPLAY : lwip 
endif

endif

ifeq ($(MK_WPA),1)
     ALL_LIBS +=	$(LIB)libwpa_supplicant.a
     libIPLAY : wpa_supplicant
endif

ifeq ($(MK_WPA),2)
     ALL_LIBS +=	$(LIB)libhostapd.a $(LIB)libnl.a 
     libIPLAY : hostapd netlink
endif

ifeq ($(MK_CURL),1)
     ALL_LIBS +=	$(LIB)libcurl.a
     libIPLAY : curl
endif

ifeq ($(MK_OPENSSL),1)
     ALL_LIBS +=	$(LIB)libopenssl.a
     libIPLAY : openssl
endif

ifeq ($(MK_UPNP_SDK),1)
     ALL_LIBS +=	$(LIB)libixml.a
     libIPLAY : ixml
     ALL_LIBS +=	$(LIB)libupnpctrl.a
     libIPLAY : upnpctrl
endif

ifeq ($(MK_DM9K_DRIVER),1)
     ALL_LIBS +=	$(LIB)libdm9k.a
     libIPLAY : dm9k
endif

ifeq ($(MK_DM9621_DRIVER),1)
     ALL_LIBS +=	$(LIB)libdm9621.a $(LIB)libos_linux.a $(LIB)libnethelper.a 
     libIPLAY : dm9621 os_linux nethelper
endif
ifeq ($(MK_UMODEM),1)
     ALL_LIBS +=	$(LIB)libnetusb.a
     libIPLAY : netusb
endif

ifeq ($(MK_OSIP),1)
     ALL_LIBS +=	$(LIB)libosip.a $(LIB)libeXosip.a 
     libIPLAY : osip eXosip
endif


endif

ifeq ($(MK_TESTCONSOLE),1)
     ALL_LIBS +=	$(LIB)libtcons.a
     libIPLAY : tcons
endif

#ifeq ($(MK_ANTIWORD),1)
#ALL_LIBS += $(LIB)libantiword.a
#libIPLAY : antiword
#endif

#ifeq ($(MK_PDB),1)
#ALL_LIBS += $(LIB)libpdbtool.a 
#ALL_LIBS += $(LIB)liblua.a 
#libIPLAY : pdb
#endif

#ifeq ($(MK_RTF),1)
#ALL_LIBS += $(LIB)libunrtf.a
#libIPLAY : unrtf
#endif

#ifeq ($(MK_LRF),1)
#ALL_LIBS += $(LIB)liblrfparser.a
#libIPLAY : lrfparser
#endif

#ALL_LIBS += $(LIB)libepub.a
#libIPLAY : epub
#ALL_LIBS += $(LIB)libxml2.a
#libIPLAY : xml2
#ALL_LIBS += $(LIB)libzip.a
#libIPLAY : zip

#ALL_LIBS += $(LIB)libcharset.a $(LIB)libinit.a $(LIB)liblex.a $(LIB)libparse.a $(LIB)libselect.a $(LIB)libutils.a
#libIPLAY : libcss
ifeq ($(MK_RELEASE),0)

ifeq ($(MK_TTF), 1)
ALL_LIBS += $(LIB)libfontengine.a
libIPLAY : fontengine
endif

endif

ifeq ($(MK_TXT), 1)
ALL_LIBS += $(LIB)libtxt.a
libIPLAY : text
endif

ifneq ($(PLATFORM), 0x661)
ALL_LIBS += $(LIB)libaudio.a
libIPLAY : audio
endif

############################################################################################

audio:
	$(MAKE) -C $(LIBSRCPATH)audio/make
bios:
	$(MAKE) -C $(LIBSRCPATH)bios/make
peripheral:
	$(MAKE) -C $(LIBSRCPATH)peripheral/make
system:
	$(MAKE) -C $(LIBSRCPATH)system/make
display:
	$(MAKE) -C $(LIBSRCPATH)display/make
sensor:
	$(MAKE) -C $(LIBSRCPATH)sensor/make	
jpeg:
	$(MAKE) -C $(LIBSRCPATH)jpeg/make
os:
	$(MAKE) -C $(LIBSRCPATH)os/make
file:
	$(MAKE) -C $(LIBSRCPATH)file/make
stream:
	$(MAKE) -C $(LIBSRCPATH)stream/make
demux:
	$(MAKE) -C $(LIBSRCPATH)demux/make
image:
	$(MAKE) -C $(LIBSRCPATH)image/make
#epd:
#	$(MAKE) -C $(LIBSRCPATH)epd/make
swmp3:
	$(MAKE) -C $(LIBSRCPATH)codec/swmp3/make
swwma:
	$(MAKE) -C $(LIBSRCPATH)codec/swwma/make
swaac:
	$(MAKE) -C $(LIBSRCPATH)codec/swaac/make
swogg:
	$(MAKE) -C $(LIBSRCPATH)codec/swogg/make
swram:
	$(MAKE) -C $(LIBSRCPATH)codec/swram/make
swamr:
	$(MAKE) -C $(LIBSRCPATH)codec/swamr/make
mjpeg:
	$(MAKE) -C $(LIBSRCPATH)codec/mjpeg/make
mjpg:
	$(MAKE) -C $(LIBSRCPATH)codec/mjpg/make
hwmad:
	$(MAKE) -C $(LIBSRCPATH)codec/hwmad/make
libmad:
	$(MAKE) -C $(LIBSRCPATH)codec/libmad/make
swmad:
	$(MAKE) -C $(LIBSRCPATH)codec/swmad/make
hwfaad:
	$(MAKE) -C $(LIBSRCPATH)codec/hwfaad/make
libfaad:
	$(MAKE) -C $(LIBSRCPATH)codec/faad/make
ffmpeg:
	$(MAKE) -C $(LIBSRCPATH)codec/ffmpeg/make
libmp4ff:
	$(MAKE) -C $(LIBSRCPATH)codec/mp4ff/make
softaviwave:
	$(MAKE) -C $(LIBSRCPATH)codec/softaviwave/make
swac3:
	$(MAKE) -C $(LIBSRCPATH)codec/swac3/make
utilities:
	$(MAKE) -C $(LIBSRCPATH)codec/utilities/make
dwl:
	$(MAKE) -C $(LIBSRCPATH)codec/dwl/make
mpeg4:
	$(MAKE) -C $(LIBSRCPATH)codec/mpeg4/make
h264:
	$(MAKE) -C $(LIBSRCPATH)codec/h264/make
mpeg12:
	$(MAKE) -C $(LIBSRCPATH)codec/mpeg12/make
pp:
	$(MAKE) -C $(LIBSRCPATH)codec/pp/make
mpeg2:
	$(MAKE) -C $(LIBSRCPATH)codec/mpeg2/make
xvid:
	$(MAKE) -C $(LIBSRCPATH)codec/xvid/make
div3:
	$(MAKE) -C $(LIBSRCPATH)codec/div3/make
avipacker:
	$(MAKE) -C $(LIBSRCPATH)codec/aviPacker/make
ptpdps:
	$(MAKE) -C $(LIBSRCPATH)ptpdps/make
Mcard:
	$(MAKE) -C $(LIBSRCPATH)Mcard/make
xpg:
	$(MAKE) -C $(LIBSRCPATH)xpg/make
usbotg:
	$(MAKE) -C $(LIBSRCPATH)usbotg/make
ui:
	$(MAKE) -C $(LIBSRCPATH)ui/make
mpapi:
	$(MAKE) -C $(LIBSRCPATH)mpapi/make
xml:
	$(MAKE) -C $(LIBSRCPATH)xml/make
netstream:
	$(MAKE) -C $(LIBSRCPATH)netstream/make
wpa_supplicant:
	$(MAKE) -C $(LIBSRCPATH)wpa_supplicant/make
hostapd:
	$(MAKE) -C $(LIBSRCPATH)hostapd/make
netlink:
	$(MAKE) -C $(LIBSRCPATH)netlink/make
lwip:
	$(MAKE) -C $(LIBSRCPATH)lwip/make
curl:
	$(MAKE) -C $(LIBSRCPATH)curl/make
openssl:
	$(MAKE) -C $(LIBSRCPATH)openssl/make
upnpctrl:
	$(MAKE) -C $(LIBSRCPATH)upnpctrl/make
rt73:
	$(MAKE) -C $(LIBSRCPATH)rt73/make
ar2524:
	$(MAKE) -C $(LIBSRCPATH)ar2524/make
os_linux:
	$(MAKE) -C $(LIBSRCPATH)os_linux/make
mac80211:
	$(MAKE) -C $(LIBSRCPATH)mac80211/make
ixml:
	$(MAKE) -C $(LIBSRCPATH)ixml/make
tcons:
	$(MAKE) -C $(LIBSRCPATH)Tconsole/make
text:
	$(MAKE) -C $(LIBSRCPATH)text/make
util:
	$(MAKE) -C $(LIBSRCPATH)util/make
record:
	$(MAKE) -C $(LIBSRCPATH)record/make
subtitle:
	$(MAKE) -C $(LIBSRCPATH)subtitle/make
unrtf:
	$(MAKE) -C $(LIBSRCPATH)unrtf/make
antiword:
	$(MAKE) -C $(LIBSRCPATH)antiword/make
epub:
	$(MAKE) -C $(LIBSRCPATH)epub/make
xml2:
	$(MAKE) -C $(LIBSRCPATH)xml2/make
zip:
	$(MAKE) -C $(LIBSRCPATH)zip/make
lrfparser:
	$(MAKE) -C $(LIBSRCPATH)lrfparser/make
zlib:
	$(MAKE) -C $(LIBSRCPATH)zlib/make
fontengine:
	$(MAKE) -C $(LIBSRCPATH)fontengine/make
pdb:
	$(MAKE) -C $(LIBSRCPATH)pdb/make
touchctrl:
	$(MAKE) -C $(LIBSRCPATH)touchctrl/make	
libcss:
	$(MAKE) -C $(LIBSRCPATH)libcss/make
dm9k:
	$(MAKE) -C $(LIBSRCPATH)dm9k/make
dm9621:
	$(MAKE) -C $(LIBSRCPATH)dm9621/make
netusb:
	$(MAKE) -C $(LIBSRCPATH)netusb/make
mpxtool:
	$(MAKE) -C $(LIBSRCPATH)mpxtool/make
osip:
	$(MAKE) -C $(LIBSRCPATH)osip/make
eXosip:
	$(MAKE) -C $(LIBSRCPATH)eXosip/make
nethelper:
	$(MAKE) -C $(LIBSRCPATH)nethelper/make
r8188c:
	$(MAKE) -C $(LIBSRCPATH)r8188c/make
r8188e:
	$(MAKE) -C $(LIBSRCPATH)r8188e/make
os_linux27:
	$(MAKE) -C $(LIBSRCPATH)os_linux27/make	
wireless:
	$(MAKE) -C $(LIBSRCPATH)os_linux27/wireless
wireless_3:
	$(MAKE) -C $(LIBSRCPATH)os_linux27/wireless_3
wireless_36:
	$(MAKE) -C $(LIBSRCPATH)os_linux27/wireless_36
compat:
	$(MAKE) -C $(LIBSRCPATH)os_linux27/compat
mac80211_2:
	$(MAKE) -C $(LIBSRCPATH)mac80211_2/make
mac80211_3:
	$(MAKE) -C $(LIBSRCPATH)mac80211_3/make
ath:
	$(MAKE) -C $(LIBSRCPATH)ath9k/make	
ath9k:
	$(MAKE) -C $(LIBSRCPATH)ath9k/include/make	
athv32:
	$(MAKE) -C $(LIBSRCPATH)ath9kv32/make	
ath9kv32:
	$(MAKE) -C $(LIBSRCPATH)ath9kv32/include/make	
	$(MAKE) -C $(LIBSRCPATH)ath9kv32/make	
rtsp:
	$(MAKE) -C $(LIBSRCPATH)rtsp/make	
av:
	$(MAKE) -C $(LIBSRCPATH)libav/make	

##################################################################################################
# CLEAN UP
##################################################################################################
ifeq ($(MK_RELEASE),0)
clean:	cleanbios cleanos cleanMcard cleanfile cleanusb \
	cleanaudio cleandemux cleancodec cleanstream \
	cleanimage cleanxpg cleanusbotg cleanmpapi cleanlib cleanperipheral cleansystem cleandisplay cleansensor cleantcons\
	cleanxml cleanwpa_supplicant cleanlwip cleancurl cleanrt73 cleannetusb \
	cleanar2524 cleanos_linux cleanmac80211 \
	cleantext cleanutil cleanrecord cleannetstream cleansubtitle \
	cleantcons cleanmpxtool \
	cleanixml cleanupnpctrl cleanopenssl \
	cleantouchctrl cleanfontengine cleannethelper \
	cleanr8188c cleanos_linux27 cleanhostapd cleannetlink \
	cleanwireless_3 cleancompat cleanav
else
clean:	cleanbios cleanos cleanMcard cleanfile cleanusb \
	cleanaudio cleandemux cleancodec cleanstream \
	cleanimage cleanxpg cleanusbotg cleanmpapi cleanperipheral cleansystem cleandisplay cleansensor cleantcons\
	cleanxml cleanwpa_supplicant cleancurl \
	cleantext cleanutil cleanrecord cleannetstream cleansubtitle \
	cleantcons cleanmpxtool \
	cleanixml cleanupnpctrl cleanopenssl \
	cleantouchctrl cleanfontengine cleanhostapd cleannetlink
endif
#	cleanopenssl cleanupnpctrl\
#	cleanjpeg \
# cleanantiword cleanunrtf cleanlrfparser  cleanxml2 cleanzip cleanzlib cleanpdb cleanfontengine cleanepub

cleantcons:
ifeq ($(MK_TESTCONSOLE),1)
	$(MAKE) -C $(LIBSRCPATH)Tconsole/make clean
endif

cleanbios:
	$(MAKE) -C $(LIBSRCPATH)bios/make clean
cleanperipheral:
	$(MAKE) -C $(LIBSRCPATH)peripheral/make clean
cleansystem:
	$(MAKE) -C $(LIBSRCPATH)system/make clean
cleandisplay:
	$(MAKE) -C $(LIBSRCPATH)display/make clean
cleansensor:
	$(MAKE) -C $(LIBSRCPATH)sensor/make clean	
cleanjpeg:
	$(MAKE) -C $(LIBSRCPATH)jpeg/make clean
cleanos:
	$(MAKE) -C $(LIBSRCPATH)os/make clean
cleanMcard:
	$(MAKE) -C $(LIBSRCPATH)Mcard/make clean
cleanfile:
	$(MAKE) -C $(LIBSRCPATH)file/make clean
cleanusb:
	$(MAKE) -C $(LIBSRCPATH)ptpdps/make clean
cleanaudio:
	$(MAKE) -C $(LIBSRCPATH)audio/make clean
cleancodec:
	$(MAKE) -C $(LIBSRCPATH)codec/swmp3/make clean
	$(MAKE) -C $(LIBSRCPATH)codec/swwma/make clean
	$(MAKE) -C $(LIBSRCPATH)codec/swamr/make clean
#	$(MAKE) -C $(LIBSRCPATH)codec/mjpeg/make clean			#OBSOLETE
#	$(MAKE) -C $(LIBSRCPATH)codec/mjpg/make clean
	$(MAKE) -C $(LIBSRCPATH)codec/swaac/make clean
	$(MAKE) -C $(LIBSRCPATH)codec/swac3/make clean
	$(MAKE) -C $(LIBSRCPATH)codec/swogg/make clean
	$(MAKE) -C $(LIBSRCPATH)codec/swram/make clean
	$(MAKE) -C $(LIBSRCPATH)codec/softaviwave/make clean
	$(MAKE) -C $(LIBSRCPATH)codec/hwmad/make clean
	$(MAKE) -C $(LIBSRCPATH)codec/hwfaad/make clean
	$(MAKE) -C $(LIBSRCPATH)codec/libmad/make clean
	$(MAKE) -C $(LIBSRCPATH)codec/faad/make clean
	$(MAKE) -C $(LIBSRCPATH)codec/mp4ff/make clean
	$(MAKE) -C $(LIBSRCPATH)codec/ffmpeg/make clean
#	$(MAKE) -C $(LIBSRCPATH)codec/dwl/make clean
#	$(MAKE) -C $(LIBSRCPATH)codec/mpeg4/make clean
#	$(MAKE) -C $(LIBSRCPATH)codec/h264/make clean
#	$(MAKE) -C $(LIBSRCPATH)codec/mpeg12/make clean
#	$(MAKE) -C $(LIBSRCPATH)codec/pp/make clean
#	$(MAKE) -C $(LIBSRCPATH)codec/xvid/make clean
#	$(MAKE) -C $(LIBSRCPATH)codec/div3/make clean
#	$(MAKE) -C $(LIBSRCPATH)codec/mpeg2/make clean
	$(MAKE) -C $(LIBSRCPATH)codec/swmad/make clean
	$(MAKE) -C $(LIBSRCPATH)codec/utilities/make clean
	$(MAKE) -C $(LIBSRCPATH)codec/swmad/make clean
	$(MAKE) -C $(LIBSRCPATH)codec/aviPacker/make clean
cleandemux:
	$(MAKE) -C $(LIBSRCPATH)demux/make clean
cleanstream:
	$(MAKE) -C $(LIBSRCPATH)stream/make clean
cleanimage:
	$(MAKE) -C $(LIBSRCPATH)image/make clean
#cleanepd:
#	$(MAKE) -C $(LIBSRCPATH)epd/make clean	
cleanxpg:
	$(MAKE) -C $(LIBSRCPATH)xpg/make clean
cleanusbotg:
	$(MAKE) -C $(LIBSRCPATH)usbotg/make clean
cleanui:
	$(MAKE) -C $(LIBSRCPATH)ui/make clean
cleanmpapi:
	$(MAKE) -C $(LIBSRCPATH)mpapi/make clean
cleanxml:
	$(MAKE) -C $(LIBSRCPATH)xml/make clean
cleannetstream:
	$(MAKE) -C $(LIBSRCPATH)netstream/make clean
cleanwpa_supplicant:
	$(MAKE) -C $(LIBSRCPATH)wpa_supplicant/make clean
cleanhostapd:
	$(MAKE) -C $(LIBSRCPATH)hostapd/make clean
cleannetlink:
	$(MAKE) -C $(LIBSRCPATH)netlink/make clean
cleanlwip:
	$(MAKE) -C $(LIBSRCPATH)lwip/make clean
cleancurl:
	$(MAKE) -C $(LIBSRCPATH)curl/make clean
cleanopenssl:
	$(MAKE) -C $(LIBSRCPATH)openssl/make clean
cleanixml:
	$(MAKE) -C $(LIBSRCPATH)ixml/make clean
cleanupnpctrl:
	$(MAKE) -C $(LIBSRCPATH)upnpctrl/make clean
cleanrt73:
	$(MAKE) -C $(LIBSRCPATH)rt73/make clean
cleanar2524:
	$(MAKE) -C $(LIBSRCPATH)ar2524/make clean
cleanos_linux:
	$(MAKE) -C $(LIBSRCPATH)os_linux/make clean
cleanmac80211:
	$(MAKE) -C $(LIBSRCPATH)mac80211/make clean
cleanlib:
	rm -f $(ALL_LIBS)
cleantext:
	$(MAKE) -C $(LIBSRCPATH)text/make clean
cleanutil:
	$(MAKE) -C $(LIBSRCPATH)util/make clean
cleanrecord:
	$(MAKE) -C $(LIBSRCPATH)record/make clean
cleansubtitle:
	$(MAKE) -C $(LIBSRCPATH)subtitle/make clean
cleanunrtf:
	$(MAKE) -C $(LIBSRCPATH)unrtf/make clean
cleanantiword:
	$(MAKE) -C $(LIBSRCPATH)antiword/make clean
cleanepub:
	$(MAKE) -C $(LIBSRCPATH)epub/make clean
cleanxml2:
	$(MAKE) -C $(LIBSRCPATH)xml2/make clean
cleanzip:
	$(MAKE) -C $(LIBSRCPATH)zip/make clean
cleanlrfparser:
	$(MAKE) -C $(LIBSRCPATH)lrfparser/make clean
cleanzlib:
	$(MAKE) -C $(LIBSRCPATH)zlib/make clean
cleanfontengine:
	$(MAKE) -C $(LIBSRCPATH)fontengine/make clean
cleanpdb:			
	$(MAKE) -C $(LIBSRCPATH)pdb/make clean	
cleantouchctrl:			
	$(MAKE) -C $(LIBSRCPATH)touchctrl/make clean
cleanlibcss:			
	$(MAKE) -C $(LIBSRCPATH)libcss/make clean	
cleandm9k:
	$(MAKE) -C $(LIBSRCPATH)dm9k/make clean
cleandm9621:
	$(MAKE) -C $(LIBSRCPATH)dm9621/make clean
cleannetusb:
	$(MAKE) -C $(LIBSRCPATH)netusb/make clean
cleanmpxtool:
	$(MAKE) -C $(LIBSRCPATH)mpxtool/make clean
cleanosip:
	$(MAKE) -C $(LIBSRCPATH)osip/make clean
	$(MAKE) -C $(LIBSRCPATH)eXosip/make clean
cleannethelper:
	$(MAKE) -C $(LIBSRCPATH)nethelper/make clean
cleanr8188c:
	$(MAKE) -C $(LIBSRCPATH)r8188c/make clean
cleanr8188e:
	$(MAKE) -C $(LIBSRCPATH)r8188e/make clean
cleanos_linux27:
	$(MAKE) -C $(LIBSRCPATH)os_linux27/make	clean
	$(MAKE) -C $(LIBSRCPATH)os_linux27/wireless_3	clean
	$(MAKE) -C $(LIBSRCPATH)os_linux27/compat	clean
cleanwireless:
	$(MAKE) -C $(LIBSRCPATH)os_linux27/wireless	clean
cleanwireless_3:
	$(MAKE) -C $(LIBSRCPATH)os_linux27/wireless_3	clean
cleanwireless_36:
	$(MAKE) -C $(LIBSRCPATH)os_linux27/wireless_36	clean
cleancompat:
	$(MAKE) -C $(LIBSRCPATH)os_linux27/compat	clean
cleanath9k:
	$(MAKE) -C $(LIBSRCPATH)ath9k/make clean
	$(MAKE) -C $(LIBSRCPATH)ath9k/include/make clean
cleanath9kv32:
	$(MAKE) -C $(LIBSRCPATH)ath9kv32/make clean
	$(MAKE) -C $(LIBSRCPATH)ath9kv32/include/make clean
cleanmac80211_2:
	$(MAKE) -C $(LIBSRCPATH)mac80211_2/make clean
cleanmac80211_3:
	$(MAKE) -C $(LIBSRCPATH)mac80211_3/make clean
cleanrtsp:
	$(MAKE) -C $(LIBSRCPATH)rtsp/make clean
cleanav:
	$(MAKE) -C $(LIBSRCPATH)libav/make clean
	
