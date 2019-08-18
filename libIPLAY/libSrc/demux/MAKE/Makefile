
include ../../../config.mak

SRC += ../source/avi/ ../source/aac/ ../source/asf/ ../source/mov/ ../source/mp3/ ../source/mpeg/  ../source/flv/ ../source/h264/ ../source/h263/

VPATH   = $(SRC)
TARGET = $(LIB)libdemux.a
ROOTPATH=../../../
#CC_OPTION  += -O0 -g $(MAIN_FLAG) -I../../api/include -I../../codec/div3/include  -I../../stream/include -I../../include -I../../mpapi/include  $(INLINE_OPTION)
#CC_OPTION += -O0 -ggdb
CC_OPTION += -I../../audio/include -I../../mpapi/include -I../../subtitle/INCLUDE
CC_OPTION += -I$(INC)avi -I$(INC)aac -I$(INC)asf -I$(INC)mov -I$(INC)mpeg -I$(INC)flv -I$(INC)h264 -I$(INC)h263
CC_OPTION += -Wa,-al=

AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

S_OBJS =  $(OBJ)board_S.o
PLATFORM = $(word $(words $(shell grep 'e ___' $(ROOTPATH)include\iplaysysconfig.h)),$(shell grep 'e ___' $(ROOTPATH)include\iplaysysconfig.h))
C_OBJS =  $(OBJ)audio_decoder.o $(OBJ)audio_out.o $(OBJ)avsync.o \
		  $(OBJ)demux.o $(OBJ)demuxmgr.o  $(OBJ)demux_stream.o $(OBJ)filter_graph.o \
		  $(OBJ)video_out.o \
		  $(OBJ)video_decoder.o	$(OBJ)board.o \
		  $(OBJ)mpfifo.o $(OBJ)video.o\
		  $(OBJ)memwatch.o \
		  
ifneq	($(PLATFORM),0x661)
C_OBJS += $(OBJ)mpeg_hdr.o
C_OBJS += $(OBJ)parse_es.o
C_OBJS += $(OBJ)asfheader.o
C_OBJS += $(OBJ)demux_asf.o
C_OBJS += $(OBJ)aviheader.o
C_OBJS += $(OBJ)demux_avi.o
C_OBJS += $(OBJ)demux_mov.o
C_OBJS += $(OBJ)parse_mp4.o
C_OBJS += $(OBJ)demux_mpg.o
C_OBJS += $(OBJ)demux_ts.o
C_OBJS += $(OBJ)demux_flv.o
C_OBJS += $(OBJ)aviprint.o
C_OBJS += $(OBJ)lrc.o
C_OBJS += $(OBJ)demux_h264.o
C_OBJS += $(OBJ)demux_h263.o

endif

ALL_OBJS =  $(C_OBJS) $(S_OBJS)

all: $(TARGET)

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: %.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: %.c  makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<
	


clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst  $(TARGET)


