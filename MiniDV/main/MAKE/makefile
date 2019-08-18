ROOTPATH=../../


include $(ROOTPATH)config.mak
TARGET = mp612
TARGET0 = MP650

LIBPATH=../../../libIPLAY/
LIB=$(LIBPATH)lib/
#LIB_IMAGE=../../../iplay_image/
FT_INC = ../../../libIPLAY/libSrc/fontengine/INCLUDE
FT_INC2 = ../../../libIPLAY/libSrc/fontengine/INCLUDE/freetype

##################################################################################################
# SWITCH
##################################################################################################

#CC_OPTION += -O1 -fno-strict-aliasing -G 8 -mno-abicalls $(ALL_INCLUDE) -I$(FT_INC) -I$(FT_INC2) -Wa,-al=$(OBJ)
#CC_OPTION += -O2 -ggdb -fno-strict-aliasing -G 8 -mno-abicalls $(ALL_INCLUDE) -I$(FT_INC) -I$(FT_INC2) -Wa,-al=$(OBJ)
CC_OPTION += -O0 -ggdb -G 8 -mno-abicalls $(ALL_INCLUDE) -I$(FT_INC) -I$(FT_INC2) -Wa,-al=$(OBJ)
#         all : mpx612

##################################################################################################
# RULES
##################################################################################################

ALL_OBJS = 	$(OBJ)main.o $(OBJ)ui_timer.o $(OBJ)xpgFunc.o $(OBJ)xpgString.o \
		$(OBJ)imageplayer.o $(OBJ)Setup.o $(OBJ)isp.o \
		$(OBJ)xpgDriveFunc.o $(OBJ)xpgDrawSprite.o\
		$(OBJ)xpgBrowser.o $(OBJ)platform.o \
		$(OBJ)camcorder_func.o \
		$(OBJ)WebCam.o \
		$(OBJ)UI_FileSystem.o $(OBJ)xpgProcSensorData.o $(OBJ)xpgVideoFunc.o $(OBJ)xpgPhotoFunc.o

# $(OBJ)slideEffect.o $(OBJ)xpgMyFavor.o $(OBJ)slideEffectMultiDirection.o $(OBJ)SetupString.o $(OBJ)SPI_Ex.o
# $(OBJ)xpgSlideFunc.o $(OBJ)ImageProcessing.o $(OBJ)SlideEffect3D.o $(OBJ)SlideEffectMaterial.o $(OBJ)SlideEffectSubWin.o
# $(OBJ)PhotoFrame.o $(OBJ)xpgWordFunc.o  $(OBJ)xpgVideoFunc.o $(OBJ)xpgPhotoFunc.o $(OBJ)xpgAudioFunc.o $(OBJ)xpgKeyPad.o $(OBJ)xpgTimeStamp.o
# $(OBJ)MiniDV_StateMachine.o $(OBJ)xpgIconFunc.o $(OBJ)fileplayer.o
#$(OBJ)xpgCamFunc.o $(OBJ)camera_func.o
		
#ADDITION_OBJS = $(LIB)indarch.o


ALL_LIBS =	$(LIB)libbios.a $(LIB)libos.a $(LIB)libMcard.a $(LIB)libfile.a $(LIB)libdisplay.a $(LIB)libsensor.a $(LIB)libimage.a\
		$(LIB)libusbotg.a $(LIBMAIN)libui.a \
		$(LIB)libdemux.a $(LIB)libmpapi.a \
		$(LIB)libjpeg.a \
		$(LIB)libstream.a \
		$(LIB)libperipheral.a $(LIB)libsystem.a \
		$(LIB)libutil.a\
		$(LIB)libavipacker.a $(LIB)libRecord.a \
		$(LIB)libtouchctrl.a

#		$(LIB)libhwfaad.a $(LIB)libtext.a 
#		 $(LIB)libepd.a $(LIB)libxml.a $(LIB)libmpxtool.a $(LIB)libptpdps.a  $(LIB)libvision.a

ALL_LIBS += $(LIB)libutilities.a

LIBGCC= $(shell $(CC) $(CC_OPTION) --print-libgcc-file-name)
LIBM=   $(shell $(CC) $(CC_OPTION) --print-file-name=libm.a)

LIBC=   $(shell $(CC) $(CC_OPTION) --print-file-name=libc.a)
LIBCPP= $(shell $(CC) $(CC_OPTION) --print-file-name=libstdc++.a)
CRUNTIME=$(LIBCPP) $(LIBC) $(LIBM) $(LIBGCC)


ADD_LIBS = -L$(LIB) -L$(LIBMAIN) --start-group --no-whole-archive -lbios -los -lMcard -lfile -lperipheral \
		-lsystem -lui -ldisplay -lsensor -limage -lusbotg -lmpapi -ldemux \
		-ljpeg \
		-lstream \
		-lavipacker \
		-lutil -lRecord \
		-ltouchctrl

#    -lhwfaad -ltext 
# -lepd -lxml -lmpxtool -lptpdps -lvision

ADD_LIBS += -lutilities

##################################################################################################
# OPTIONAL LIBRARIES
##################################################################################################

PLATFORM = $(word $(words $(shell grep 'e ___' $(ROOTPATH)include\iplaysysconfig.h)),$(shell grep 'e ___' $(ROOTPATH)include\iplaysysconfig.h))
MK_XPG = $(word $(words $(shell grep 'MAKE_XPG_PLAYER' $(ROOTPATH)include\corelib.h)),$(shell grep 'MAKE_XPG_PLAYER' $(ROOTPATH)include\corelib.h))
MK_AUDIO = $(word $(words $(shell grep 'AUDIO_ON' $(ROOTPATH)include\corelib.h)),$(shell grep 'AUDIO_ON' $(ROOTPATH)include\corelib.h))
MK_SWMP3 = $(word $(words $(shell grep 'MP3_SW_AUDIO' $(ROOTPATH)include\corelib.h)),$(shell grep 'MP3_SW_AUDIO' $(ROOTPATH)include\corelib.h))
MK_HWMP3 = $(word $(words $(shell grep 'MP3_HW_CODEC' $(ROOTPATH)include\corelib.h)),$(shell grep 'MP3_HW_CODEC' $(ROOTPATH)include\corelib.h))
MK_MADMP3 = $(word $(words $(shell grep 'MP3_MAD_AUDIO' $(ROOTPATH)include\corelib.h)),$(shell grep 'MP3_MAD_AUDIO' $(ROOTPATH)include\corelib.h))
MK_BT = $(word $(words $(shell grep 'MAKE_BLUETOOTH' $(ROOTPATH)include\corelib.h)),$(shell grep 'MAKE_BLUETOOTH' $(ROOTPATH)include\corelib.h))
MK_SWAAC = $(word $(words $(shell grep 'AAC_SW_AUDIO' $(ROOTPATH)include\corelib.h)),$(shell grep 'AAC_SW_AUDIO' $(ROOTPATH)include\corelib.h))
MK_FAAD  = $(word $(words $(shell grep 'AAC_FAAD_AUDIO' $(ROOTPATH)include\corelib.h)),$(shell grep 'AAC_FAAD_AUDIO' $(ROOTPATH)include\corelib.h))
MK_HWAAC  = $(word $(words $(shell grep 'AAC_ENABLE' $(ROOTPATH)include\corelib.h)),$(shell grep 'AAC_ENABLE' $(ROOTPATH)include\corelib.h))
MK_WMA = $(word $(words $(shell grep 'WMA_ENABLE' $(ROOTPATH)include\corelib.h)),$(shell grep 'WMA_ENABLE' $(ROOTPATH)include\corelib.h))
MK_AMR = $(word $(words $(shell grep 'AMR_ENABLE' $(ROOTPATH)include\corelib.h)),$(shell grep 'AMR_ENABLE' $(ROOTPATH)include\corelib.h))
MK_AMRFULL = $(word $(words $(shell grep 'AMR_ENCODE_ENABLE' $(ROOTPATH)include\corelib.h)),$(shell grep 'AMR_ENCODE_ENABLE' $(ROOTPATH)include\corelib.h))
MK_SWOGG = $(word $(words $(shell grep 'OGG_ENABLE' $(ROOTPATH)include\corelib.h)),$(shell grep 'OGG_ENABLE' $(ROOTPATH)include\corelib.h))
MK_SWRAM = $(word $(words $(shell grep 'RAM_AUDIO_ENABLE' $(ROOTPATH)include\corelib.h)),$(shell grep 'RAM_AUDIO_ENABLE' $(ROOTPATH)include\corelib.h))
MK_SWAC3 = $(word $(words $(shell grep 'AC3_AUDIO_ENABLE' $(ROOTPATH)include\corelib.h)),$(shell grep 'AC3_AUDIO_ENABLE' $(ROOTPATH)include\corelib.h))
MK_SWWAV = $(word $(words $(shell grep 'WAV_ENABLE' $(ROOTPATH)include\corelib.h)),$(shell grep 'WAV_ENABLE' $(ROOTPATH)include\corelib.h))
MK_WIFI = $(word $(words $(shell grep 'NETWARE_ENABLE' $(ROOTPATH)include\corelib.h)),$(shell grep 'NETWARE_ENABLE' $(ROOTPATH)include\corelib.h))
MK_WPA = $(word $(words $(shell grep 'Make_WPA' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_WPA' $(ROOTPATH)include\corelib.h))
MK_LWIP = $(word $(words $(shell grep 'Make_LWIP' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_LWIP' $(ROOTPATH)include\corelib.h))
MK_CURL = $(word $(words $(shell grep 'Make_CURL' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_CURL' $(ROOTPATH)include\corelib.h))
MK_OPENSSL = $(word $(words $(shell grep 'Make_OPENSSL' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_OPENSSL' $(ROOTPATH)include\corelib.h))
MK_UPNP_SDK = $(word $(words $(shell grep 'Make_UPNP_SDK' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_UPNP_SDK' $(ROOTPATH)include\corelib.h))
MK_DM9K_DRIVER = $(word $(words $(shell grep 'Make_DM9KS_ETHERNET' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_DM9KS_ETHERNET' $(ROOTPATH)include\corelib.h))
MK_SDIO = $(word $(words $(shell grep 'Make_SDIO' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_SDIO' $(ROOTPATH)include\corelib.h))
MK_USB = $(word $(words $(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h))
MK_TESTCONSOLE = $(word $(words $(shell grep 'e_TE' $(ROOTPATH)include\corelib.h)),$(shell grep 'e_TE' $(ROOTPATH)include\corelib.h))
MK_EREADER = $(word $(words $(shell grep 'EREADER_ENABLE' $(ROOTPATH)include\corelib.h)),$(shell grep 'EREADER_ENABLE' $(ROOTPATH)include\corelib.h))
MK_RTF = $(word $(words $(shell grep 'e_UNRTF' $(ROOTPATH)include\corelib.h)),$(shell grep 'e_UNRTF' $(ROOTPATH)include\corelib.h))
MK_ANTIWORD = $(word $(words $(shell grep 'e_ANTIWORD' $(ROOTPATH)include\corelib.h)),$(shell grep 'e_ANTIWORD' $(ROOTPATH)include\corelib.h))
MK_LRF = $(word $(words $(shell grep 'e_LRF' $(ROOTPATH)include\corelib.h)),$(shell grep 'e_LRF' $(ROOTPATH)include\corelib.h))
MK_TTF = $(word $(words $(shell grep 'Make_TTF' $(ROOTPATH)include\corelib.h)),$(shell grep 'e_TTF' $(ROOTPATH)include\corelib.h))
MK_PDB = $(word $(words $(shell grep 'e_PDB' $(ROOTPATH)include\corelib.h)),$(shell grep 'e_PDB' $(ROOTPATH)include\corelib.h))
MK_RELEASE = $(word $(words $(shell grep 'Make_RELEASE' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_RELEASE' $(ROOTPATH)include\corelib.h))
MK_DIAGNOSTICTC = $(word $(words $(shell grep 'e_DI' $(ROOTPATH)include\corelib.h)),$(shell grep 'e_DI' $(ROOTPATH)include\corelib.h))
MK_TXT = $(word $(words $(shell grep 'Make_TXT' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_TXT' $(ROOTPATH)include\corelib.h))
MK_EPUB = $(word $(words $(shell grep 'Make_EPUB' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_EPUB' $(ROOTPATH)include\corelib.h))
MK_PDF = $(word $(words $(shell grep 'Make_PDF' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_PDF' $(ROOTPATH)include\corelib.h))
MK_NETSTREAM = $(word $(words $(shell grep 'Make_NETSTREAM' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_NETSTREAM' $(ROOTPATH)include\corelib.h))
MK_UMODEM = $(word $(words $(shell grep 'Make_MODEM' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_MODEM' $(ROOTPATH)include\corelib.h))
############################################################################################
# ICCORE
############################################################################################
ifeq ($(PLATFORM),0x660)
ICCORE = 0x660
TARGET0 = MP660
PLATFORM_H = Platform_MP660.h
MK_VD = 0
endif
ifeq ($(PLATFORM),0x661)
ICCORE = 0x660
TARGET0 = MP660
PLATFORM_H = Platform_MP661.h
MK_MJ = 0
MK_VD = 0
endif
ifeq ($(PLATFORM),0x663)
ICCORE = 0x660
TARGET0 = MP660
PLATFORM_H = Platform_MP663.h
MK_VD = 0
endif
ifeq ($(PLATFORM),0x6635642)
ICCORE = 0x660
TARGET0 = MP660
PLATFORM_H = Platform_MP663_DV.h
MK_VD = 0
endif
ifeq ($(PLATFORM),0x662)
ICCORE = 0x660
TARGET0 = MP660
PLATFORM_H = Platform_MP662W.h
MK_MJ = 0
MK_VD = 0
endif

ifeq ($(PLATFORM),0x662001)
ICCORE = 0x660
TARGET0 = MP660
PLATFORM_H = Platform_MP662W_WELD.h
MK_MJ = 0
MK_VD = 0
endif

ifeq ($(PLATFORM),0x662002)
ICCORE = 0x660
TARGET0 = MP660
PLATFORM_H = Platform_MP662W_SFDT.h
MK_MJ = 0
MK_VD = 0
endif


ifeq ($(PLATFORM),0x663007)
ICCORE = 0x660
TARGET0 = MP660
PLATFORM_H = Platform_MP663_SpyCamera.h
MK_VD = 0
endif

MK_VD = $(word $(words $(shell grep -w 'VIDEO_ENABLE' $(ROOTPATH)include\$(PLATFORM_H))), $(shell grep -w 'VIDEO_ENABLE' $(ROOTPATH)include\$(PLATFORM_H)))
MK_MJ = $(word $(words $(shell grep -w 'MJPEG_ENABLE' $(ROOTPATH)include\$(PLATFORM_H))),$(shell grep -w 'MJPEG_ENABLE' $(ROOTPATH)include\$(PLATFORM_H)))
MK_SUBTITLE = $(word $(words $(shell grep -w 'DISPLAY_VIDEO_SUBTITLE' $(ROOTPATH)include\$(PLATFORM_H))), $(shell grep -w 'DISPLAY_VIDEO_SUBTITLE' $(ROOTPATH)include\$(PLATFORM_H)))
MK_LYRIC = $(word $(words $(shell grep -w 'LYRIC_ENABLE' $(ROOTPATH)include\$(PLATFORM_H))), $(shell grep -w 'LYRIC_ENABLE' $(ROOTPATH)include\$(PLATFORM_H)))
MK_MPEG12 = $(word $(words $(shell grep 'VCODEC_MPEG12_ENABLE' $(ROOTPATH)include\corelib.h)),$(shell grep 'VCODEC_MPEG12_ENABLE' $(ROOTPATH)include\corelib.h))
MK_MPEG4 = $(word $(words $(shell grep 'VCODEC_MPEG4_ENABLE' $(ROOTPATH)include\corelib.h)),$(shell grep 'VCODEC_MPEG4_ENABLE' $(ROOTPATH)include\corelib.h))
MK_H264 = $(word $(words $(shell grep 'VCODEC_H264_ENABLE' $(ROOTPATH)include\corelib.h)),$(shell grep 'VCODEC_H264_ENABLE' $(ROOTPATH)include\corelib.h))
############################################################################################
ifeq ($(MK_XPG),1)
#ALL_LIBS += $(LIB)libxpg.a
ADD_LIBS += -lxpg
endif

ifeq ($(MK_VD),1)
  ifeq ($(ICCORE),0x650)
    ALL_LIBS += $(LIB)libffmpeg.a
    ADD_LIBS += -lffmpeg
    ifeq ($(MK_MPEG12),1)
      ALL_LIBS += $(LIB)libmpeg12.a
      ADD_LIBS += -lmpeg12
    endif
    ifeq ($(MK_MPEG4),1)
      ALL_LIBS += $(LIB)libmpeg4.a
      ADD_LIBS += -lmpeg4
    endif
    ifeq ($(MK_H264),1)
      ALL_LIBS += $(LIB)libh264.a
      ADD_LIBS += -lh264
    endif
    ALL_LIBS += $(LIB)libdwl.a $(LIB)libpp.a
    ADD_LIBS += -ldwl -lpp
  else
    #ALL_LIBS += $(LIB)libdiv3.a $(LIB)libmpeg2.a $(LIB)libxvid.a $(LIB)libffmpeg.a
    #ADD_LIBS += -ldiv3 -lmpeg2 -lxvid -lffmpeg -lamr
    ALL_LIBS += $(LIB)libdiv3.a $(LIB)libmpeg2.a $(LIB)libxvid.a
    ADD_LIBS += -ldiv3 -lmpeg2 -lxvid
  endif
endif

ifeq ($(MK_MJ),1)
ALL_LIBS += $(LIB)libmjpg.a
ADD_LIBS += -lmjpg
endif

ifeq ($(MK_LYRIC),1)
ALL_LIBS += $(LIB)libsubtitle.a
ADD_LIBS += -lsubtitle
else
	ifeq ($(MK_SUBTITLE),1)
	ALL_LIBS += $(LIB)libsubtitle.a
	ADD_LIBS += -lsubtitle
	endif
endif

ifeq ($(MK_BT),1)
ALL_LIBS += $(LIB)libBlueTooth.a
ADD_LIBS += -lBlueTooth
ALL_LIBS += $(LIB)libsbc.a
ADD_LIBS += -lsbc
endif



ifeq ($(MK_TESTCONSOLE), 1)
ALL_LIBS += $(LIB)libtcons.a
ADD_LIBS += -ltcons
endif

ifeq ($(MK_AUDIO),1)
ALL_LIBS += $(LIB)libaudio.a
ADD_LIBS += -laudio

ifeq ($(MK_SWMP3),1)
ifneq ($(ICCORE),0x615)
ALL_LIBS += $(LIB)libmp3.a
ADD_LIBS += -lmp3
else
ALL_LIBS += $(LIB)libmp3_MP615.a
ADD_LIBS += -lmp3_MP615
endif
ALL_LIBS += $(LIB)libswmp3.a
ADD_LIBS += -lswmp3
endif

ifeq ($(MK_HWMP3),1)
ALL_LIBS += $(LIB)libhwmad.a $(LIB)libswmp3.a $(LIB)libmp3.a
ADD_LIBS += -lswmp3 -lhwmad -lmp3
endif

ifeq ($(MK_MADMP3),1)
ALL_LIBS += $(LIB)libmad.a $(LIB)libutilities.a  $(LIB)libswmad.a
ADD_LIBS += -lmad -lutilities -lswmad
endif

ifeq ($(MK_SWAAC),1)
ifneq ($(ICCORE),0x615)
ALL_LIBS += $(LIB)libaac.a
ADD_LIBS += -laac
else
ALL_LIBS += $(LIB)libaac_MP615.a
ADD_LIBS += -laac_MP615
endif
ALL_LIBS += $(LIB)libswaac.a
ADD_LIBS += -lswaac
endif

ifeq ($(MK_FAAD),1)
ifneq ($(PLATFORM),0x6158819)
ifeq ($(MK_SWMP3),1)
ifneq ($(MK_WAV),1)
ALL_LIBS += $(LIB)libutilities.a
ADD_LIBS += -lutilities
endif
endif
ALL_LIBS += $(LIB)libfaad.a $(LIB)libmp4ff.a $(LIB)libswfaad.a
ADD_LIBS += -lfaad -lmp4ff -lswfaad
endif
endif

ifeq ($(MK_WMA),1)
ifneq ($(ICCORE),0x615)
ALL_LIBS += $(LIB)libwma.a
ADD_LIBS += -lwma
else
ALL_LIBS += $(LIB)libwma_MP615.a
ADD_LIBS += -lwma_MP615
endif
ALL_LIBS += $(LIB)libswwma.a
ADD_LIBS += -lswwma
endif

ifeq ($(MK_AMR), 1)
  ifeq ($(MK_AMRFULL), 1)
    ALL_LIBS += $(LIB)libswamr.a $(LIB)libamrfull.a
    ADD_LIBS += -lswamr -lamrfull
  else
    # Because sram initial function is embedded in libffmpeg.a,
    #  including the library is necessary.
    ALL_LIBS += $(LIB)libswamr.a $(LIB)libamr.a $(LIB)libffmpeg.a $(LIB)libdiv3.a
    ADD_LIBS += -lswamr -lamr -lffmpeg -ldiv3
  endif
endif

ifeq ($(MK_SWRAM),1)
ALL_LIBS += $(LIB)libswram.a $(LIB)libram.a
ADD_LIBS += -lswram -lram
endif

ifeq ($(MK_SWWAV),1)
ALL_LIBS += $(LIB)libsoftaviwave.a $(LIB)libaviwave.a
ADD_LIBS += -lsoftaviwave	-laviwave
endif

ifeq ($(MK_SWAC3),1)
ifneq ($(ICCORE),0x615)
ALL_LIBS += $(LIB)libac3.a
ADD_LIBS += -lac3
else
ALL_LIBS += $(LIB)libac3_MP615.a
ADD_LIBS += -lac3_MP615
endif
ALL_LIBS += $(LIB)libswac3.a
ADD_LIBS += -lswac3

endif

ifeq ($(MK_SWOGG),1)
ifneq ($(ICCORE),0x615)
ALL_LIBS += $(LIB)libogg.a
ADD_LIBS += -logg
else
ALL_LIBS += $(LIB)libogg_MP615.a
ADD_LIBS += -logg_MP615
endif
ALL_LIBS += $(LIB)libswogg.a
ADD_LIBS += -lswogg	 
endif

endif

ifeq ($(MK_WIFI), 1)

ifeq ($(MK_NETSTREAM), 1)
ADD_LIBS += -lnetstream
ALL_LIBS += $(LIB)libnetstream.a
endif

ifeq ($(MK_LWIP),1)
ALL_LIBS +=	$(LIB)liblwip.a
ADD_LIBS += -llwip
ALL_LIBS +=	$(LIBMAIN)libnet.a 
ADD_LIBS += -lnet
endif

ifeq ($(MK_WPA),1)
ALL_LIBS += $(LIB)libwpa_supplicant.a
ADD_LIBS += -lwpa_supplicant
endif

ifeq ($(MK_USB),1)
ALL_LIBS +=	$(LIB)librt73.a
ADD_LIBS += -lrt73
endif

ifeq ($(MK_USB),2)
ALL_LIBS +=	$(LIB)libr8711u.a
ADD_LIBS += -lr8711u
endif

ifeq ($(MK_USB),3)
ALL_LIBS +=	$(LIB)libmac80211.a $(LIB)libar2524.a $(LIB)libos_linux.a
ADD_LIBS += -lmac80211 -lar2524 -los_linux
endif

ifeq ($(MK_USB),4)
ALL_LIBS +=	$(LIB)librt73.a $(LIB)libmac80211.a $(LIB)libar2524.a $(LIB)libos_linux.a
ADD_LIBS += -lrt73 -lmac80211 -lar2524 -los_linux
endif

ifeq ($(MK_DM9K_DRIVER),1)
ALL_LIBS +=	$(LIB)libdm9k.a
ADD_LIBS += -ldm9k
endif

ifeq ($(MK_CURL),1)
ALL_LIBS += $(LIB)libcurl.a
ADD_LIBS += -lcurl
endif

ifeq ($(MK_OPENSSL),1)
ALL_LIBS += $(LIB)libopenssl.a
ADD_LIBS += -lopenssl
endif

ifeq ($(MK_UMODEM),1)
ALL_LIBS += $(LIB)libnetusb.a
ADD_LIBS += -lnetusb
endif

ifeq ($(MK_SDIO),2)
ALL_LIBS +=	$(LIB)libr8711.a
ADD_LIBS += -lr8711
endif

ifeq ($(MK_UPNP_SDK),1)
ALL_LIBS +=	$(LIB)libixml.a $(LIB)libupnpctrl.a
ADD_LIBS += -lixml -lupnpctrl
endif
endif



ifeq ($(MK_EREADER),1)
ALL_LIBS += $(LIB)libzlib.a
ADD_LIBS += -lzlib
ALL_LIBS += $(LIBMAIN)libtypesetting.a
ADD_LIBS += -ltypesetting

#ifeq ($(MK_ANTIWORD),1)
#ALL_LIBS += $(LIB)libantiword.a
#ADD_LIBS += -lantiword
#endif

#ifeq ($(MK_PDB),1)
#ALL_LIBS += $(LIB)libpdbtool.a
#ALL_LIBS += $(LIB)liblua.a
#ADD_LIBS += -lpdbtool -llua
#endif

#ifeq ($(MK_RTF),1)
#ALL_LIBS += $(LIB)libunrtf.a
#ADD_LIBS += -lunrtf
#endif

#ifeq ($(MK_LRF),1)
#ALL_LIBS += $(LIB)liblrfparser.a
#ADD_LIBS += -llrfparser
#endif
endif

ifeq ($(MK_TXT),1)
ALL_LIBS += $(LIB)libtxt.a
ADD_LIBS += -ltxt
endif

ifeq ($(MK_EPUB),1)
ALL_LIBS += $(LIB)libepub.a $(LIB)libxml2.a $(LIB)libzip.a
ADD_LIBS += -lepub -lxml2 -lzip
#ALL_LIBS += $(LIB)libcharset.a $(LIB)libinit.a $(LIB)liblex.a $(LIB)libparse.a $(LIB)libselect.a $(LIB)libutils.a
#ADD_LIBS += -lcharset -linit -llex -lparse -lselect -lutils
endif

ifeq ($(MK_PDF),1)
ALL_LIBS += $(LIB)libpng.a $(LIB)libgoo.a $(LIB)libfofi.a $(LIB)libsplash.a $(LIB)libpdf.a
ADD_LIBS += -lpng -lgoo -lfofi -lsplash -lpdf
endif

ifeq ($(MK_TTF), 1)
ALL_LIBS += $(LIB)libfontengine.a
ADD_LIBS += -lfontengine
endif

ifeq ($(MK_DIAGNOSTICTC),1)
ALL_LIBS += $(LIBMAIN)libdiagnostictc.a
ADD_LIBS += -ldiagnostictc
endif

ADD_LIBS += $(CRUNTIME) --end-group

##################################################################################################
# Target
##################################################################################################

ifeq ($(MK_DIAGNOSTICTC),1)
ifeq ($(MK_RELEASE),2)
mpx612:  ui typesetting diagnostictc net $(TARGET).elf $(ALL_LIBS)
else
mpx612:  ui lib typesetting diagnostictc net $(TARGET).elf $(ALL_LIBS)
endif

else
ifeq ($(MK_RELEASE),2)
mpx612:  ui typesetting $(TARGET).elf $(ALL_LIBS)
else
mpx612:  ui lib typesetting net $(TARGET).elf $(ALL_LIBS)
endif

endif


$(TARGET).elf	: $(ALL_OBJS) $(ALL_LIBS) $(ADDITION_OBJS) $(TARGET).ld makefile
	$(LD) -T $(TARGET).ld -d -e Boot -relax --sort-common -Map $(TARGET).map --cref $(ALL_OBJS) $(ADDITION_OBJS) $(ADD_LIBS) -o $(TARGET).elf
	$(OC) -O binary $(TARGET).elf $(TARGET).bin
ifneq ($(ICCORE),0x615)
	cp $(TARGET).bin $(TARGET0).bin
	cp $(TARGET).elf $(TARGET0).elf
	cp $(TARGET).map $(TARGET0).map
	rm -f $(TARGET).bin
	rm -f $(TARGET).elf
	rm -f $(TARGET).map
#	$(OBJDUMP) -d $(TARGET0).elf > $(TARGET0).asm
else
	rm -f $(TARGET0).bin
	rm -f $(TARGET0).elf
	rm -f $(TARGET0).map
#	$(OBJDUMP) -d $(TARGET).elf > $(TARGET).asm
endif

#	@grep '	[ls]w[rl]	' mp612.asm

	@del cp.bat
	@echo copy $(TARGET0).bin %%1>>cp.bat

$(ALL_OBJS)		: $(OBJ)%.o: $(SRC)%.c makefile ../../config.mak
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

lib:
	$(MAKE) -C $(LIBPATH)make

ui:
#####   Below message is used for flag checking   #####
	@config
	@echo ------------------------------------------------------
	@echo The Platform you are making is $(PLATFORM)  !!!
	@echo MK_BT flag                    - $(MK_BT)
	@echo MK_TESTCONSOLE flag           - $(MK_TESTCONSOLE)
	@echo Softwarw MP3 codec flag       - $(MK_SWMP3)
	@echo AAC SWAAC codec flag          - $(MK_SWAAC)
	@echo Video enable flag             - $(MK_VD)
	@echo Video codec MPEG-12           - $(MK_MPEG12)
	@echo Video codec MPEG-4            - $(MK_MPEG4)
	@echo Video codec H.264             - $(MK_H264)
	@echo MotionJpeg flag               - $(MK_MJ)
	@echo WIFI flag                     - $(MK_WIFI)
	@echo WPA flag                      - $(MK_WPA)
	@echo LWIP flag                     - $(MK_LWIP)
	@echo CURL flag                     - $(MK_CURL)
	@echo OPENSSL flag                  - $(MK_OPENSSL)
	@echo UPNP flag                     - $(MK_UPNP_SDK)
	@echo SDIO flag                     - $(MK_SDIO)
	@echo USB flag                      - $(MK_USB)
typesetting:

	@echo TTF flag                      - $(MK_TTF)
	@echo DiagnosticTestConsole flag    - $(MK_DIAGNOSTICTC)
	@echo ------------------------------------------------------
ifeq ($(MK_EREADER),1)
#	$(MAKE) -C $(ROOTPATH)typesetting/make
endif

	$(MAKE) -C $(ROOTPATH)ui/make
#image:
#	$(MAKE) -C $(LIB_IMAGE)make

fontengine:
#	$(MAKE) -C $(LIBSRCPATH)fontengine/make

diagnostictc:
#	$(MAKE) -C $(ROOTPATH)diagnostic/make
net:
#	$(MAKE) -C $(ROOTPATH)net/make

##################################################################################################
# CLEAN UP
##################################################################################################
ifeq ($(MK_RELEASE),2)
clean:
	$(MAKE) -C $(ROOTPATH)ui/make clean
#	$(MAKE) -C $(ROOTPATH)typesetting/make clean
#	$(MAKE) -C $(ROOTPATH)diagnostic/make clean
	rm -f $(ALL_OBJS) $(TARGET).elf  $(OBJ)\*.lst $(SRC)\*.bak
else
clean:
	$(MAKE) -C $(LIBPATH)make clean
	$(MAKE) -C $(ROOTPATH)ui/make clean
#	$(MAKE) -C $(ROOTPATH)typesetting/make clean
#	$(MAKE) -C $(ROOTPATH)diagnostic/make clean
#	$(MAKE) -C $(ROOTPATH)net/make clean
	rm -f $(ALL_OBJS) $(TARGET).elf  $(OBJ)\*.lst $(SRC)\*.bak
endif

