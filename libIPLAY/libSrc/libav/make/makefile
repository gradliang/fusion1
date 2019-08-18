include ../../../config.mak
 
ROOTPATH=../../../
 
LIBAVFORMAT = $(LIB)libavformat.a
LIBAVUTIL = $(LIB)libavutil.a
LIBAVCODEC = $(LIB)libavcodec.a
TARGET = $(LIBAVFORMAT) $(LIBAVUTIL) $(LIBAVCODEC)

MK_USB = $(word $(words $(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h))
MK_DM9621 = $(word $(words $(shell grep 'Make_DM9621_ETHERNET' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_DM9621_ETHERNET' $(ROOTPATH)include\corelib.h))

CC = $(BIN)gcc
CC_OPTION += -O2 -G 8
#CC_OPTION += -O0 -G 8
#CC_OPTION += -ggdb
CC_OPTION += -include $(INC)/config.h
CC_OPTION += -I$(INC) -I$(G_INC) 
CC_OPTION += -DPLATFORM_MPIXEL
CC_OPTION += -I../../../libSrc/rtsp/include
CC_OPTION += -I../../../libSrc/libev/include
CC_OPTION += -I../../../libSrc/compiler/include
CC_OPTION += -I../../../libSrc/lwip/include
CC_OPTION += -I../../../libSrc/zlib/include
#CC_OPTION += -I../../../libSrc/os_linux27/include
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

C_OBJS +=  $(OBJ)stubs.o 

LIBAVFORMAT_OBJ =  $(OBJ)libavformat/
LIBAVFORMAT_OBJS =  $(LIBAVFORMAT_OBJ)utils.o $(LIBAVFORMAT_OBJ)options.o \
		  $(LIBAVFORMAT_OBJ)aviobuf.o $(LIBAVFORMAT_OBJ)avio.o \
		  $(LIBAVFORMAT_OBJ)file.o $(LIBAVFORMAT_OBJ)http.o $(LIBAVFORMAT_OBJ)allformats.o \
		  $(LIBAVFORMAT_OBJ)httpauth.o $(LIBAVFORMAT_OBJ)urldecode.o \
		  $(LIBAVFORMAT_OBJ)mpeg.o $(LIBAVFORMAT_OBJ)mux.o \
		  $(LIBAVFORMAT_OBJ)rtpdec.o $(LIBAVFORMAT_OBJ)id3v1.o $(LIBAVFORMAT_OBJ)id3v2.o \
		  $(LIBAVFORMAT_OBJ)metadata.o $(LIBAVFORMAT_OBJ)mov.o $(LIBAVFORMAT_OBJ)rawdec.o \
		  $(LIBAVFORMAT_OBJ)isom.o $(LIBAVFORMAT_OBJ)riff.o $(LIBAVFORMAT_OBJ)dv.o \
		  $(LIBAVFORMAT_OBJ)mov_chan.o $(LIBAVFORMAT_OBJ)mp3dec.o 

LIBAVCODEC_OBJ =  $(OBJ)libavcodec/
LIBAVCODEC_OBJS =  $(LIBAVCODEC_OBJ)avpacket.o $(LIBAVCODEC_OBJ)utils.o \
		  $(LIBAVCODEC_OBJ)allcodecs.o $(LIBAVCODEC_OBJ)mjpegenc.o \
		  $(LIBAVCODEC_OBJ)ac3tab.o $(LIBAVCODEC_OBJ)mpeg4audio.o \
		  $(LIBAVCODEC_OBJ)mpegaudiodata.o $(LIBAVCODEC_OBJ)bitstream.o \
		  $(LIBAVCODEC_OBJ)mathtables.o $(LIBAVCODEC_OBJ)options.o \
		  $(LIBAVCODEC_OBJ)parser.o $(LIBAVCODEC_OBJ)codec_desc.o \
		  $(LIBAVCODEC_OBJ)mpegaudiodecheader.o 

LIBAVUTIL_OBJ =  $(OBJ)libavutil/
LIBAVUTIL_OBJS =  $(LIBAVUTIL_OBJ)avstring.o $(LIBAVUTIL_OBJ)mem.o $(LIBAVUTIL_OBJ)dict.o $(LIBAVUTIL_OBJ)opt.o \
		  $(LIBAVUTIL_OBJ)time.o $(LIBAVUTIL_OBJ)log2_tab.o $(LIBAVUTIL_OBJ)base64.o \
		  $(LIBAVUTIL_OBJ)md5.o $(LIBAVUTIL_OBJ)random_seed.o $(LIBAVUTIL_OBJ)parseutils.o \
		  $(LIBAVUTIL_OBJ)channel_layout.o $(LIBAVUTIL_OBJ)rational.o \
		  $(LIBAVUTIL_OBJ)mathematics.o 


ALL_OBJS =  $(C_OBJS) $(S_OBJS) $(LIBAVUTIL_OBJS) $(LIBAVCODEC_OBJS) $(LIBAVFORMAT_OBJS) 

all: $(TARGET) makefile

$(LIBAVFORMAT)	: $(LIBAVFORMAT_OBJS) $(C_OBJS)
	$(AR) -rsv $(LIBAVFORMAT) $(LIBAVFORMAT_OBJS) $(C_OBJS)

$(LIBAVUTIL)	: $(LIBAVUTIL_OBJS)
	$(AR) -rsv $(LIBAVUTIL) $(LIBAVUTIL_OBJS)

$(LIBAVCODEC)	: $(LIBAVCODEC_OBJS)
	$(AR) -rsv $(LIBAVCODEC) $(LIBAVCODEC_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(LIBAVFORMAT_OBJS):	$(LIBAVFORMAT_OBJ)%.o: $(INC)libavformat/%.c makefile
	$(CC) -I$(INC)libavformat $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(LIBAVUTIL_OBJS):	$(LIBAVUTIL_OBJ)%.o: $(INC)libavutil/%.c makefile
	$(CC) -include $(INC)libavutil/internal.h $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(LIBAVCODEC_OBJS):	$(LIBAVCODEC_OBJ)%.o: $(INC)libavcodec/%.c makefile
	$(CC) -I$(INC)libavcodec $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(LIBAVFORMAT_OBJ)*.lst $(LIBAVCODEC_OBJS)*.lst $(LIBAVUTIL_OBJ)*.lst $(TARGET)
