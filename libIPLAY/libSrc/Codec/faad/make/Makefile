ROOTPATH=../../../../
include $(ROOTPATH)config.mak

G_INC = $(ROOTPATH)include/
INC = ../include/
OBJ = ../object/
LIB = $(ROOTPATH)lib/
SRC = ../source/
MP4FF_INC = ../../mp4ff/include


TARGET = $(LIB)libfaad.a

CC_OPTION  += $(MAIN_FLAG) -O2 -I$(INC) -I../../include/ -I$(G_INC) -I$(MP4FF_INC) -DHAVE_AV_CONFIG_H \
				-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE $(INLINE_OPTION) -Wa,-al=

#CC_OPTION  += $(MAIN_FLAG) -O0 -ggdb -I$(INC) -I../../include/ -I$(G_INC) -I$(MP4FF_INC) -DHAVE_AV_CONFIG_H \
#				-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE $(INLINE_OPTION) -Wa,-al=

#--------------------------------------------------------------
#the folowing line is depending on where compiler M600 is. 
#BIN = C:\Magic-Pixel\tools\mips\bin\mips-elf-mpx-
#--------------------------------------------------------------
CC = $(BIN)gcc
AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=
LD = $(BIN)ld
AR = $(BIN)ar

ALL_C_OBJS =  $(OBJ)bits.o $(OBJ)cfft.o $(OBJ)decoder.o $(OBJ)drc.o \
		  $(OBJ)drm_dec.o $(OBJ)error.o $(OBJ)filtbank.o \
		  $(OBJ)ic_predict.o $(OBJ)is.o $(OBJ)lt_predict.o $(OBJ)mdct.o $(OBJ)mp4.o $(OBJ)ms.o \
		  $(OBJ)output.o $(OBJ)pns.o $(OBJ)ps_dec.o $(OBJ)ps_syntax.o \
		  $(OBJ)pulse.o $(OBJ)specrec.o $(OBJ)syntax.o $(OBJ)tns.o $(OBJ)hcr.o $(OBJ)huffman.o \
		  $(OBJ)rvlc.o $(OBJ)ssr.o $(OBJ)ssr_fb.o $(OBJ)ssr_ipqf.o $(OBJ)common.o \
		  $(OBJ)sbr_dct.o $(OBJ)sbr_e_nf.o $(OBJ)sbr_fbt.o $(OBJ)sbr_hfadj.o $(OBJ)sbr_hfgen.o \
		  $(OBJ)sbr_huff.o $(OBJ)sbr_qmf.o $(OBJ)sbr_syntax.o $(OBJ)sbr_tf_grid.o $(OBJ)sbr_dec.o \
		  $(OBJ)ad_faad.o $(OBJ)aac_info.o $(OBJ)faad_aac.o $(OBJ)memory.o $(OBJ)timer.o

C_OBJS =  $(OBJ)bits.o $(OBJ)cfft.o $(OBJ)decoder.o $(OBJ)drc.o $(OBJ)error.o $(OBJ)filtbank.o \
		  $(OBJ)is.o $(OBJ)mdct.o $(OBJ)mp4.o $(OBJ)ms.o $(OBJ)output.o $(OBJ)pns.o \
		  $(OBJ)pulse.o $(OBJ)specrec.o $(OBJ)syntax.o $(OBJ)tns.o $(OBJ)huffman.o $(OBJ)common.o \
		  $(OBJ)aac_info.o $(OBJ)memory.o $(OBJ)timer.o \
		  $(OBJ)ad_faad.o $(OBJ)faad_aac.o
S_OBJS =

ALL_OBJS =  $(C_OBJS) $(S_OBJS)

all: $(TARGET)

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst $(SRC)/*.bak  $(TARGET)

